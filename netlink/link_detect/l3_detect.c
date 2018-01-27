/***********************************
 * Filename: l3_detect.c
 * Author: zhangwj
 * Date: 2017-07-21
 * Description: 
 * Warnning:
 ***********************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h> //isspace()...
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <pthread.h>
#include <sys/time.h> //gettimeofday()
#include <sys/ioctl.h>
#include <errno.h>
 
#include "nano_ipc.h"
#include "iface_info.h"

#define MAX_ICMP_SEND_PERIOD 		3
#define MAX_ICMP_RECEIVE_PERIOD 	5

extern struct linkmgt_info *plkmgt;
extern unsigned int g_l3_thread_ctr;
extern unsigned int g_detect_addr;
extern pthread_mutex_t g_mutex;
extern pthread_rwlock_t g_RWLock;
extern int nn_cli_fd;

char *xstrdup(const char *s)
{   
    char *d = strdup(s);
    if (!d) {
		fprintf(stderr, "out of virtual memory\n");
    	exit(2);
	}
    return d;
}

char *proc_gen_fmt(char *name, int more, FILE * fh,...)
{
    char buf[512], format[512] = ""; 
    char *title, *head, *hdr;
    va_list ap; 

    if (!fgets(buf, (sizeof buf) - 1, fh))
    return NULL;
    strcat(buf, " ");

    va_start(ap, fh);
    title = va_arg(ap, char *); 
    for (hdr = buf; hdr;) {
    while (isspace(*hdr) || *hdr == '|')
        hdr++;
    head = hdr;
    hdr = strpbrk(hdr, "| \t\n");
    if (hdr)
        *hdr++ = 0;

    if (!strcmp(title, head)) {
        strcat(format, va_arg(ap, char *));
        title = va_arg(ap, char *); 
        if (!title || !head)
        break;
    } else {
        strcat(format, "%*s");  /* XXX */
    }   
    strcat(format, " ");
    }   
    va_end(ap);

    if (!more && title) {
    	fprintf(stderr, "warning: %s does not contain required field %s\n",
        	name, title);
    	return NULL;
    }   
    return xstrdup(format);
}

int inet_parse_gateway(char *bufp, uint *gateway)
{   
    char *sp = bufp, *bp;
    uint i;
    uint val;
    struct sockaddr_in sin;

    val = 0;
    bp = (char *) &val;
    for (i = 0; i < sizeof(sin.sin_addr.s_addr); i++) {
    *sp = toupper(*sp);
    
    if ((*sp >= 'A') && (*sp <= 'F'))
        bp[i] |= (int) (*sp - 'A') + 10;
    else if ((*sp >= '0') && (*sp <= '9'))
        bp[i] |= (int) (*sp - '0');
    else
        return -1;

    bp[i] <<= 4;
    sp++;
    *sp = toupper(*sp);
    
    if ((*sp >= 'A') && (*sp <= 'F'))
        bp[i] |= (int) (*sp - 'A') + 10;
    else if ((*sp >= '0') && (*sp <= '9'))
        bp[i] |= (int) (*sp - '0');
    else
        return -1;

    sp++;
    }
	
	if (val != 0) {
		*gateway = val;
		return 0;
	}	
	return -1;
}

int get_gateway_by_ifname(const char *ifname, uint *gateway)
{
	int ret = -1;
	char buff[1024];
	FILE *fp = NULL;
	char gate_addr[128];
	char iface[17];
	char *fmt;

	fp = fopen("/proc/net/route", "r");
	if (NULL == fp) {
		return -1;
	}

	fmt = proc_gen_fmt("/proc/net/route", 0, fp,
					"Iface", "%15s",
					"Gateway", "%127s",
					NULL);
	
	while (fgets(buff, 1023, fp)) {
		sscanf(buff, fmt, iface, gate_addr);
		if(!strcmp(iface, ifname)) {
			if (!inet_parse_gateway(gate_addr, gateway)) {
				ret = 0;
				break;
			}
		}
	}
	free(fmt);
	fclose(fp);
	return ret;
}

int get_netinfo_by_ifname(const char *ifname, struct netinfo *net)
{
	int sock = 0;
	struct sockaddr_in sin;
	struct ifreq ifr;
	int ret = 0;

	if (NULL == ifname || NULL == net) {
		return -1;
	}

	/* open a SOCK_DGRAM socket */
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		log_dbg("Create socket error, Error:[%s]", strerror(errno));
		return -1;
	}
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ - 1] = 0;

	/* Get interface 	ip 	address */
	if (ioctl(sock, SIOCGIFADDR, &ifr) == 0) {
		memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
		net->ip = htonl(sin.sin_addr.s_addr);
		ret++;
    } 
	/* Get interface netmask address */
	if (ioctl(sock, SIOCGIFNETMASK, &ifr) == 0) {
		memcpy(&sin, &ifr.ifr_netmask, sizeof(sin));
		net->mask = htonl(sin.sin_addr.s_addr);
		ret++;
    } 
	/* Get interface gateway address*/
	if (get_gateway_by_ifname(ifname, &net->gw) == 0) {
		ret ++;
	}
	close(sock);

	if (ret == 3) 
		return 0;
	return -1;
}

int recreate_detect_scokfd(struct uplink_info *uplink)
{
    int i;
    int sockfd;
    struct sockaddr_in dstaddr;
    struct ifreq data;
    struct timeval tv; 

	if (uplink->sockfd) {
		close(uplink->sockfd);
		uplink->sockfd = -1;
	}
	
    if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1) {   
        log_dbg("Create Socket Fail!\n");
        return -1; 
    }   

    /* Set socket option:  bind device */
    strncpy(data.ifr_name, uplink->ifname, IFNAMESZ);
    if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, (char *)&data, sizeof(data))  < 0) {
		//log_dbg("Set Socket option bind to device: %s failed", uplink->ifname);
		close(sockfd);
		return -1; 
    }   

    /* Set socket option: send timeout */
    tv.tv_sec  = 1;
    tv.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0) {   
        log_dbg("Set Socket option send timeout failed");
        close(sockfd);
        return -1; 
    } 

    /* Set socket option: receive timeout */
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {   
        log_dbg("Set Socket option receive timeout failed");
        close(sockfd);
        return -1; 
    }
    uplink->sockfd = sockfd;
	
    return sockfd;
}

void tv_sub(struct timeval *out,struct timeval *in)
{       
	if ((out->tv_usec -= in->tv_usec) < 0) 
	{       
		--out->tv_sec;
		out->tv_usec += 1000000;
	}
	out->tv_sec -= in->tv_sec;
}

unsigned short in_cksum(unsigned short *addr, int len, unsigned short csum)
{
	int nleft = len;
	unsigned short *w = addr;
	unsigned short answer = 0;
	int sum = csum;

	/*
	 *  Our algorithm is simple, using a 32 bit accumulator (sum),
	 *  we add sequential 16 bit words to it, and at the end, fold
	 *  back all the carry bits from the top 16 bits into the lower
	 *  16 bits.
	 */
	while (nleft > 1)  {
		sum += *w++;
		nleft -= 2;
	}

	/* mop up an odd byte, if necessary */
	if (nleft == 1) {
		*(unsigned char *)(&answer)=*(unsigned char *)w;
		sum += answer; /* le16toh() may be unavailable on old systems */
	} 

	/*
	 * add back carry outs from top 16 bits to low 16 bits
	 */
	sum = (sum >> 16) + (sum & 0xffff);	/* add hi 16 to low 16 */
	sum += (sum >> 16);			/* add carry */
	answer = ~sum;				/* truncate to 16 bits */
	return (answer);
}

void icmp_echo_set(struct icmp *icmphdr, int seq, int length)
{
	icmphdr->icmp_type = ICMP_ECHO;
	icmphdr->icmp_code = 0;
	icmphdr->icmp_cksum = 0;
	icmphdr->icmp_seq = htons(seq);
	icmphdr->icmp_id = htons(getpid() & 0xFFFF);
	icmphdr->icmp_cksum = in_cksum((unsigned short*)icmphdr, length, 0);
}

int icmp_recv(int sockfd)
{
    int i, c = 0;
    char buf[128];
	int count = 0;
	int offset;
    struct icmp *icmp;
	struct ip * ip_hdr;
   
	memset(buf, 0, sizeof(buf));
	while (count < MAX_ICMP_RECEIVE_PERIOD) {
		if ((c = recv(sockfd, buf, sizeof(buf), 0)) < 0) {
			if (errno == EINTR || errno == EAGAIN) {
				count++;
				continue;
			}
			//log_dbg("icmp recv, Error[%d:%s]", errno, strerror(errno));
			return -1;
		}
		
		ip_hdr = (struct ip *)buf;
		offset = ip_hdr->ip_hl << 2;
		icmp = (struct icmp*)(buf + offset);
		if (((c - offset) >= 8)) { /* icmp_hdr >= 8 */
			if ((icmp->icmp_type == ICMP_ECHOREPLY) &&
				(icmp->icmp_id == htons((getpid() & 0xFFFF)))) {
				return 0;
			}
		}  
		count++;
	}

	//log_dbg("icmp_type: %u, icmp_code: %u, icmp_id: %u", icmp->icmp_type, icmp->icmp_code, icmp->icmp_id);
    return -1;
}

int icmp_send(int sockfd, struct sockaddr *dstaddr, int count)
{
	int ret = -1;
	int i;
	char send_buf[128];

	for (i = 0; i < count; i++) {
		icmp_echo_set((struct icmp *)send_buf, i, 64);
		ret = sendto(sockfd, send_buf, 64, 0, dstaddr, sizeof(struct sockaddr));
		if(ret < 0) {
			log_dbg("icmp send, Error[%d:%s]", errno, strerror(errno));
		}

	}    
	return ret;
}

int get_max_sockfd(struct uplinks *dev)
{
	int maxfd = -1;
	int i = 0;

	for (i = 0; i < dev->uplinks; i++) {
		if ((dev->uplink[i].valid == 0) && 
			(dev->uplink[i].send_ok == 0) && 
			(dev->uplink[i].recv_ok == 1)) {
			if (maxfd < dev->uplink[i].sockfd) {
				maxfd = dev->uplink[i].sockfd;	
			}
		}
	}
	
	return maxfd;
}

static int l3_uplink_notify(int uplink_type, int status, int subid, int rate, struct netinfo net) 
{
	struct modlue_notify_info *msg = NULL;
	struct uplink_info_report uplink;

	msg = (struct modlue_notify_info *)malloc(sizeof(struct modlue_notify_info) + sizeof(struct uplink_info_report));
	if (NULL == msg) {
		return -1;
	}
	
	msg->msg_id = 0;
	memset(&uplink, 0, sizeof(uplink));
	uplink.uplink_type = uplink_type;
	uplink.status = status;
	uplink.uplink_subid = subid;
	uplink.rate = rate;
	uplink.ip = htonl(net.ip);
	uplink.mask = htonl(net.mask);
	uplink.gw = htonl(net.gw);
	uplink.dns = htonl(net.dns);
	memcpy((void *)msg->data, (void *)&uplink, sizeof(struct uplink_info_report));

	pthread_mutex_lock(&g_mutex);
	nn_socket_send(nn_cli_fd, (void *)msg, 
		(sizeof(struct modlue_notify_info) + sizeof(struct uplink_info_report)));
	pthread_mutex_unlock(&g_mutex);
	free(msg);

	return 0;
}

void icmp_statistics_reset(struct uplinks *dev)
{
	int i = 0;
	
	for (i = 0; i <dev->uplinks; i++) {
		dev->uplink[i].recv_ok = 1;
		dev->uplink[i].send_ok = 1;
	}
}

void *l3_thread_handler(void *arg)
{
	int status;
	int i, m; 
	int ret;
	int count;
	int maxfd;
	int pos;
	int do_shut[24];
	unsigned int detect_addr;
	struct uplinks *dev;
	struct timeval tv;
	struct sockaddr_in dstaddr;
	int sockfd;
	fd_set rfd;

	/* Fill dstination address */
	memset(&dstaddr, 0, sizeof(dstaddr));
	dstaddr.sin_family = AF_INET;
	dstaddr.sin_port = htons(0);
	
	dev = &plkmgt->dev;
	tv.tv_sec = 5;
	tv.tv_usec = 0;

	while (g_l3_thread_ctr) {

		icmp_statistics_reset(dev);
		for (m = 0; m < MAX_ICMP_SEND_PERIOD; m++) {
			FD_ZERO(&rfd);
			for (i = 0; i < dev->uplinks; i++) {
				if (dev->uplink[i].l2_state != 1 && 
					dev->uplink[i].valid != 1 &&
					dev->uplink[i].net_ok != 1 &&
					dev->uplink[i].recv_ok == 1) {

					detect_addr = dev->uplink[i].net.gw;
					if (dev->uplink[i].type == UPLINK_LTE && g_detect_addr) {
						detect_addr = g_detect_addr;
					}
					
					dstaddr.sin_addr.s_addr = htonl(detect_addr);
					if (icmp_send(dev->uplink[i].sockfd, (struct sockaddr *)&dstaddr, 1) > 0) {
						do_shut[i] = 0;
						dev->uplink[i].send_ok = 0;
						FD_SET(dev->uplink[i].sockfd, &rfd);
						//log_dbg("ifname: %s icmp send ok", dev->uplink[i].ifname);
					} else {
						if (!((errno == ETIMEDOUT)
							|| (errno == EINTR)
							|| (errno == EAGAIN))) {
							do_shut[i] = 1;
						}
						//log_dbg("ifname: %s icmp send, Error[%d:%s]", dev->uplink[i].ifname, errno, strerror(errno));
					}
				}
			}//end for(i)
			
			maxfd = get_max_sockfd(dev);
			if (maxfd > 0) {
				ret = select(maxfd + 1, &rfd, NULL, NULL, &tv);
				if (ret > 0) {
					for (i = 0; i < dev->uplinks; i++) {
						if ((dev->uplink[i].l2_state != 1) && 
							(dev->uplink[i].send_ok == 0)) {

							if (FD_ISSET(dev->uplink[i].sockfd, &rfd)) {
								if (icmp_recv(dev->uplink[i].sockfd) == 0) {
									do_shut[i] = 0;
									dev->uplink[i].recv_ok = 0;
									//log_dbg("ifname: %s icmp recv ok", dev->uplink[i].ifname);
								} else {
									if (!((errno == ETIMEDOUT)
										|| (errno == EINTR)
										|| (errno == EAGAIN))) {
										do_shut[i] = 1;
									}
									//log_dbg("ifname: %s icmp recv do_shut: %d, Error[%d:%s]", dev->uplink[i].ifname, do_shut[i], errno, strerror(errno));
								}
							}
						}
					} //end for(i)
				}
			}
		} // end for(m)

		log_dbg("");
		log_dbg("---------------------- Line Status ------------------------------");
		for (i = 0; i < dev->uplinks; i++) {

			status = UPLINK_STATUS_DOWN;
			if (dev->uplink[i].recv_ok == 0) {
				status = UPLINK_STATUS_EXCELLENT;
			} else {
			//	dev->uplink[i].errcnt ++;
			}

			l3_uplink_notify(dev->uplink[i].type, status, dev->uplink[i].subid, 0, dev->uplink[i].net);
			log_dbg("Interface: %-5s Type: %-10s Subid: %-2d Status: %-5s Rate: %d", dev->uplink[i].ifname, 
				uplink_type[dev->uplink[i].type], dev->uplink[i].subid, (status == UPLINK_STATUS_EXCELLENT)? "up":"down", 0);
			
			if (do_shut[i] || dev->uplink[i].valid) {
				dev->uplink[i].valid = do_shut[i];
				if (recreate_detect_scokfd(&dev->uplink[i]) > 0) {
					dev->uplink[i].valid = 0;
				} else {
					dev->uplink[i].valid = 1;
				}
			}

			if (get_netinfo_by_ifname(dev->uplink[i].ifname, &(dev->uplink[i].net)) == 0) {
				dev->uplink[i].net_ok = 0;
			} else {
				dev->uplink[i].net_ok = 1;
			}
		}//end for()

		sleep(plkmgt->args.period);
	} //end while()
}

#if 0
void *l3_thread_handler(void *arg)
{
	int i;
	int status;
	int ret;
	struct uplink_dev *dev = NULL;
	struct iface_info_t *node = NULL;
	struct netdev_info net;
	unsigned int detect_addr;
	
	sleep(3);//wait network initialization
	dev = &g_conf->dev;
	while (g_l3_thread_ctr) {	
		for (i = 0; i < dev->uplinks; i++) {
			RWLOCK_RLOCK(&g_RWLock);
			node = get_iface_node(dev->uplink[i].ifname);
			RWLOCK_RUNLOCK(&g_RWLock);
			if (node) {

				memset(&net, 0, sizeof(net));
				ret = get_netinfo_by_ifname(dev->uplink[i].ifname, &net);
				if (ret == 0) {
					/* default detect address is gateway */
					detect_addr = net.gw;
					if (node->type == UPLINK_LTE && g_detect_addr) {
						/* 4G detect address */
						detect_addr = g_detect_addr;
					}

					if (do_icmp_check(dev->uplink[i].ifname, detect_addr) == 0) {
						if (node->state != DEV_EVENT_LINK_UP) {
							RWLOCK_WLOCK(&g_RWLock);
							iface_node_state_change(node, DEV_EVENT_LINK_UP);
							RWLOCK_WUNLOCK(&g_RWLock);
						}
					} else if(node->state != DEV_EVENT_LINK_DOWN){
						RWLOCK_WLOCK(&g_RWLock);
						iface_node_state_change(node, DEV_EVENT_LINK_DOWN);
						RWLOCK_WUNLOCK(&g_RWLock);					
					}
				} else if(node->state != DEV_EVENT_LINK_DOWN) {
					RWLOCK_WLOCK(&g_RWLock);
					iface_node_state_change(node, DEV_EVENT_LINK_DOWN);
					RWLOCK_WUNLOCK(&g_RWLock);
				}
				/* notify manager module with nanomsg */
				status = (node->state == DEV_EVENT_LINK_UP)? UPLINK_STATUS_EXCELLENT : UPLINK_STATUS_DOWN;
				l3_uplink_notify(node->type, status, node->subid, 0, net);
				log_dbg("Interface: %-5s Type: %-10s Subid: %-2d Status: %-5s Rate: %d", 
					node->ifname, uplink_type[node->type], node->subid, (status == UPLINK_STATUS_EXCELLENT)? "up":"down", 0);
			}
		} /* end for() */
		sleep(g_conf->period); //link_detect period 
	} /* end while() */
}
#endif
