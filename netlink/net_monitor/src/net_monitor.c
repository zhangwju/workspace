/*******************************************
 * Filename: net_monitor.c	
 * Author: zhangwj
 * Description: network interface monitor
 * Date: 2017-10-25
 *******************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/route.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include "nlk_socket.h"
#include "baselib.h"

#define EPOLL_LISTEN_MAX_CNT	256

#define MSG_HDRLEN				8
#define NOTIFY_PORT				9573

#define NET_MONITOR_CONFIG_PATH		"/etc/net_monitor"

int log_enable = 1;

struct msginfo {
	int msg_type;
	int msglen;
	char data[0];
};

struct notify_info {
	char ifname[IFNAMESZ];
	int status;	
};

struct netops_ctx {
	int epoll;
	int notify;
	uint servaddr;
	struct nlk_socket *nlk;
};
struct netops_ctx *ctx;

enum link_status {
	DEV_LINK_DOWN = 0,
	DEV_LINK_UP
};

int send_notify(int fd, const void *buf, int buflen)
{
	int ret = 0;
	struct sockaddr_in servaddr;

	memset(&servaddr, 0, sizeof(struct sockaddr_in));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = ctx->servaddr;
	servaddr.sin_port = htons(NOTIFY_PORT);
	
	if (sendto(fd, buf, buflen, 0, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
		log_dbg("sendto failure, %s\n", strerror(errno));
		return -1;
	}
	return ret;
}

int do_notify(char *ifname, int status)
{
	struct msginfo *msg = NULL;
	struct notify_info notify;

	msg = (struct msginfo *)malloc(sizeof(struct msginfo) + sizeof(struct notify_info));
	if(msg == NULL) {
		return -1;
	}

	memset(&notify, 0, sizeof(notify));
	strncpy(notify.ifname, ifname, IFNAMESZ);
	notify.status = status;
	
	msg->msg_type = 0;
	msg->msglen = MSG_HDRLEN + sizeof(struct notify_info);
	memcpy((void *)msg->data, (void *)&notify, sizeof(struct notify_info));

	return send_notify(ctx->notify, (void *)msg, msg->msglen);
}

int parse_rtattr(struct rtattr *tb[], int max, struct rtattr *rta, int len)
{
	memset(tb, 0, sizeof(struct rtattr *) * (max + 1));
	while (RTA_OK(rta, len)) {
		if ((rta->rta_type <= max) && (!tb[rta->rta_type]))
			tb[rta->rta_type] = rta;
		rta = RTA_NEXT(rta,len);
	}
	if (len)
		fprintf(stderr, "!!!Deficit %d, rta_len=%d\n", len, rta->rta_len);
	return 0;
}

int nl_netifinfo_handle(void *nlk, struct nlmsghdr *nlh)
{
    int len;
    struct rtattr *tb[IFLA_MAX + 1];
    struct ifinfomsg *ifinfo;
	int status;

    bzero(tb, sizeof(tb));
    ifinfo = NLMSG_DATA(nlh);
    len = nlh->nlmsg_len - NLMSG_SPACE(sizeof(*ifinfo));
    parse_rtattr(tb, IFLA_MAX, IFLA_RTA (ifinfo), len);
	
	status = (ifinfo->ifi_flags & IFF_RUNNING) ? DEV_LINK_UP: DEV_LINK_DOWN;
    if(tb[IFLA_IFNAME]) {
        log_dbg("Interface: %-5s  link_type: %-8s  status: %-5s", RTA_DATA(tb[IFLA_IFNAME]), 
			(nlh->nlmsg_type==RTM_NEWLINK) ? "NEWLINK" : "DELLINK", (ifinfo->ifi_flags & IFF_RUNNING) ? "up" : "down");
		return do_notify(RTA_DATA(tb[IFLA_IFNAME]), status);
    }

	return -1;
}

int nlmsg_handle(void *nlk)
{
	int status;
	int r_size;
	char buf[2048];
	struct nlmsghdr * nlh;

	while(1) {
		status = nlk_socket_recvfrom(nlk, buf, sizeof(buf));
		if(status < 0) {
			if (errno == EINTR || errno == EAGAIN)
				continue;
			log_dbg("netlink receive error %s (%d)\n", strerror(errno), errno);
			return -1;
		}

		r_size = status;
		nlh = (struct nlmsghdr *)buf;
		for(; NLMSG_OK(nlh, r_size); nlh = NLMSG_NEXT(nlh, r_size)) {   
			switch(nlh->nlmsg_type) {
			case NLMSG_DONE:
			case NLMSG_ERROR:
				break;
			case RTM_NEWLINK:
			case RTM_DELLINK:
				status = nl_netifinfo_handle(nlk, nlh); 
				break;
			case RTM_NEWADDR:
			case RTM_DELADDR:
				break;
			case RTM_NEWROUTE:
			case RTM_DELROUTE:
				break;
			default:
			break;
			}   
		}   
	}	
	return status;
}

int epoll_add_fd(int epoll, int fd)
{
    struct epoll_event ev;

    ev.data.fd = fd; 
    ev.events = EPOLLIN | EPOLLET;

    if (epoll_ctl(epoll, EPOLL_CTL_ADD, fd, &ev) < 0) {
        perror("epoll add fd error");
        return -1; 
    }   

    return 0;
}

int init_epoll()
{
	int epollfd = -1; 

	epollfd = epoll_create(EPOLL_LISTEN_MAX_CNT);
	if (epollfd < 0) {
		perror("epoll create failure!...");
		return -1; 
	}   
	return	epollfd;
}

int init_nl_socket()
{	
	pid_t pid;
	struct nlk_socket *nlk = NULL;
	
	nlk = nlk_socket_open(NETLINK_ROUTE);
	if(nlk == NULL) {
		return -1;
	}
	
	pid = getpid();
	if (nlk_socket_bind(nlk, RTMGRP_LINK, pid) < 0) {
		close(nlk->fd);
		free(nlk);
		return -1;
	}
	ctx->nlk = nlk;
	return 0;
}

int init_notify_socket(uint ipaddr, int dport)
{
	int sock;
	int timeout = 500;
	struct sockaddr_in servaddr;

	sock = socket(PF_INET, SOCK_DGRAM, 0);
	if(sock < 0) {
		log_dbg("Create notify socket error");
		return -1;
	}
	
	memset(&servaddr, 0, sizeof(struct sockaddr_in));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = ipaddr;
	servaddr.sin_port = htons(dport);
	
	setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(int));	

	return sock;
}

static void sighandler(int sig)
{
	log_dbg("Recvice signal ID[%d]\n", sig);

	if(ctx) {
		if(ctx->nlk) {
			nlk_socket_close(ctx->nlk);
		}
		if(ctx->epoll) {
			close(ctx->epoll);
		}
		if(ctx->notify) {
			close(ctx->notify);
		}
		free(ctx);
	}
	exit(1);
}

/* check ipaddr isvalid */
bool isvalidip(const char *ip)
{
    int dots = 0;
    int setions = 0;  
                    
    if (NULL == ip || *ip == '.') {
        return false;
    }   
        
    while (*ip) {
        if (*ip == '.') {
            dots ++; 
            if (setions >= 0 && setions <= 255) {
                setions = 0;
                ip++;
                continue;
            }   
            return false;
        }   
        else if (*ip >= '0' && *ip <= '9') {
            setions = setions * 10 + (*ip - '0');
        } else {
            return false;
        }   
        ip++;   
    }   
        
    if (setions >= 0 && setions <= 255) {
        if (dots == 3) {
            return true;
        }   
    }          
    return false;
}

int get_net_monitor_config(uint *servaddr)
{
	FILE * fp = NULL;
	char line[512];
	char ipaddr[16];
	
	fp = fopen(NET_MONITOR_CONFIG_PATH, "r");
	if(fp) {
		while (fgets(line, 511, fp)) {
			if (line[0] == '#'	|| 
				line[0] == '\0' || 
				line[0] == '\r') {
				continue;
			}
			if(strstr(line, "notify_server")) {
				sscanf(line, "notify_server=%s", ipaddr); 
			}
		}
		fclose(fp);	
	}
	if(isvalidip(ipaddr)) {
		*servaddr = inet_addr(ipaddr);
		log_dbg("notify_server: %s", ipaddr);
		return 0;
	}
	return -1;
}

void init_net_monitor_config()
{
	log_dbg("load net_monitor config...");
	do {
		if(get_net_monitor_config(&ctx->servaddr) == 0) {
			break;
		}
		sleep(2); 
	} while(1);
}

void open_and_config_notify()
{
	int sock;
	init_net_monitor_config();
	do {
		sock = init_notify_socket(ctx->servaddr, NOTIFY_PORT);
		if(sock > 0) {
			break;
		}
		sleep(2);
	} while(1);
	ctx->notify = sock;
	log_dbg("connect "IPSTR":%d success",IP2STR(ctx->servaddr), NOTIFY_PORT);
}

int global_init()
{	
	signal(SIGPIPE, SIG_IGN);
	signal(SIGTERM, sighandler);
	signal(SIGINT, sighandler);

	/* init log file */
    if (!log_init("net_monitor", "/var/log/net_monitor.log", 10*1024*1000, LOG_LEVEL_DEBUG, 0)) {
        return -1; 
    }

	ctx = (struct netops_ctx *)malloc(sizeof(struct netops_ctx));
	if(NULL ==  ctx) {
		fprintf(stderr, "allocate ctx failure\n");
		return -1;
	}
	
	memset(ctx, 0, sizeof(struct netops_ctx));
	open_and_config_notify();
	ctx->epoll = init_epoll();
	if(ctx->epoll < 0) {
		return -1;
	} 

	if(init_nl_socket() < 0) {
		return -1;
	}	

	if(epoll_add_fd(ctx->epoll, ctx->nlk->fd) < 0) {
		return -1;
	}	

	return 0;
}


void main_loop(void)
{
	int send_err = 0;
	int i = 0;
	int fd_cnt = 0;
	int sfd;
	int timeout = 500;
	struct epoll_event events[EPOLL_LISTEN_MAX_CNT];    

	memset(events, 0, sizeof(events));
	while(1) {   
		/* wait epoll event */
		fd_cnt = epoll_wait(ctx->epoll, events, EPOLL_LISTEN_MAX_CNT, timeout); 
		for(i = 0; i < fd_cnt; i++) {   
			sfd = events[i].data.fd;
			if(events[i].events & EPOLLIN) {   
				if (sfd == ctx->nlk->fd) {   
					if (nlmsg_handle(ctx->nlk) == 2) {
						send_err +=1;
					}    
				}   
			}   
		}   
		if(send_err == 3) {
			if(ctx->notify) {
				close(ctx->notify);
			}
			ctx->notify = init_notify_socket(ctx->servaddr, NOTIFY_PORT);	
			send_err = 0;
		}
	}   
}

int main(int argc, char **argv)
{
	if(global_init() < 0) {
		sighandler(SIGTERM);
	}
	
	/* enter main loop */
	main_loop();
	return 0;
}
