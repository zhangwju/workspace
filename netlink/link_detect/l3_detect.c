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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <net/if.h>

#define DEFAULT_DETECT_TIME			30

#define ICMP_ECHOREQ 				5
#define MAX_ICMP_SEND_PERIOD 		3
#define MAX_ICMP_RECEIVE_PERIOD 	10
static int icmp_req_seq = 0; /* The sequence of ICMP requst */

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

unsigned int inet_parse_gateway(char *bufp, uint *gateway)
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
	*gateway = val;
	
	return 0;
}

int get_gateway_by_ifname(const char *ifname, uint gateway)
{
	int i = 0;
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

static int icmp_echo_request(int sockfd, struct sockaddr_in *dstaddr)
{
	int ret;
	char buf[128];
	uint len = sizeof(struct icmp);
	socklen_t dstlen = sizeof(struct sockaddr_in);
	struct icmp *echo_req;

	memset(buf, 0, sizeof(buf));
	echo_req = (struct icmp*)buf;
	echo_req->icmp_type = ICMP_ECHOREQ;
	echo_req->icmp_code = 0;
	echo_req->icmp_cksum = 0;
	echo_req->icmp_id = getpid();
	echo_req->icmp_seq = icmp_req_seq++;
	struct timeval *tval = (struct timeval *)echo_req->icmp_data;
	gettimeofday(tval, NULL);
	echo_req->icmp_cksum = cal_cksum((uint16_t *)echo_req,  sizeof(struct icmp) + sizeof(struct timeval));

	ret = sendto(sockfd, buf, len, 0, (struct sockaddr*)dstaddr, dstlen);
	if (ret > 0)
		return -1;
	else
		return 0;

}

static int icmp_echo_reply(int sockfd, struct sockaddr_in *dstaddr)
{
	int i, c;
	char buf[128];
	struct ip *ip_info;
	struct icmp *icmp;
	socklen_t dstlen = sizeof(struct sockaddr_in);

	for (i = 0; i < MAX_ICMP_RECEIVE_PERIOD; i++)
	{
		memset(buf, 0, sizeof(buf));
		if ((c = recvfrom(sockfd, buf, sizeof(buf), 0, 
							(struct sockaddr *)dstaddr, &dstlen)) < 0)
		{
			continue;
		}

		if (c >= 48) /* ip + icmp */
		{
			icmp = (struct icmp*)(buf + sizeof(struct ip)); /* skip ip hdr */
			if(icmp->icmp_type == ICMP_ECHOREPLY)
			{
				log_dbg("ICMP echo-rep success -->");
				return 0;
			}
		}	
	}
	
	return -1;
}

static STATUS do_icmp_check(const char *ifname, uint ip)
{
	int i;
	int sockfd;
	struct sockaddr_in dstaddr;
	struct ifreq data;
	struct timeval tv;

	if((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1)
	{
		log_dbg("Create Socket Fail!\n");
		return -1;
	}

	/* Set socket option:  bind device */
	strncpy(data.ifr_name, ifname, IFNAMESZ);
	if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, (char *)&data, sizeof(data))  < 0) {
	       log_dbg("Set Socket option bind to device: %s failed", ifname);
		   close(sockfd);
		   return -1;
	}

	/*Set socket option: send timeout */
	tv.tv_sec  = 1;
	tv.tv_usec = 0;
	if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0)
	{
		log_dbg("Set Socket option send timeout failed");
		close(sockfd);
		return -1;
	}

	/*Set socket option: receive timeout */
	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
	{
		log_dbg("Set Socket option receive timeout failed");
		close(sockfd);
		return -1;
	}

	
	/* Fill dstination address */
	dstaddr.sin_family = AF_INET;
	dstaddr.sin_port = htons(0);
	dstaddr.sin_addr.s_addr = htonl(ip);
	
#if 0
	if(inet_pton(AF_INET, ipstr, &dstaddr.sin_addr) == -1)
	{
		log_dbg("Dest ip address: %s error!\n", ipstr);
		close(sockfd);
		return -1;
	}
#endif

	icmp_req_seq = 0;
	for (i = 0; i < MAX_ICMP_SEND_PERIOD; i++)
	{
		log_dbg("<-- Send ICMP echo-req from: %s", ifname);
		if (icmp_echo_request(sockfd, &dstaddr) == STATUS_OK &&
			icmp_echo_reply(sockfd, &dstaddr) == STATUS_OK)
		{
			close(sockfd);
			return 0;
		}
	}

	close(sockfd);
	return -1;
}

static int l3_uplink_notify(const struct iface_info_t *node) 
{
	struct modlue_notify_info *msg = NULL;
	struct uplink_info_report uplink;
	
	if(NULL == node) {
		return -1
	}

	msg = (struct modlue_notify_info *)malloc(sizeof(struct modlue_notify_info) + sizeof(struct uplink_info_report));
	if(NULL == msg) {
		return -1;
	}
	
	msg->msg_id = 0;
	memset(&uplink, 0, sizeof(uplink));
	uplink.uplink_type = node->type;
	uplink.status = node->state;
	uplink.uplink_subid = node->subid;
	memcpy((void *)msg->data, (void *)&uplink, sizeof(struct uplink_info_report));

	pthread_mutex_lock(&g_mutex);
	nn_socket_send((void *)msg, 
		sizeof(struct modlue_notify_info) + sizeof(struct uplink_info_report)), 
		nn_cli_fd);
	pthread_mutex_unlock(&g_mutex);
	free(msg);
}

void l3_thread_handler(void *arg)
{
	int ret;
	int i, cnt;
	uint gateway;
	struct uplink_dev *dev = NULL;
	struct uplink_info *uplink = NULL;
	struct iface_info_t *node = NULL;

	dev = g_conf->dev;
	while (g_l3_thread_ctr) {
		
		for (i = 0; i < dev->uplinks; i++) {
			RWLOCK_RLOCK(&g_RWLock);
			node = get_iface_node(dev->uplink[i].ifname);
			RWLOCK_RUNLOCK(&g_RWLock);

			ret = get_gateway_by_ifname(dev->uplink[i].ifname, &gateway);
			if (ret == 0) {
				if (do_icmp_check(dev->uplink[i].ifname, gateway) == 0) {
					if (node->state != DEV_EVENT_LINK_UP) {
						RWLOCK_WLOCK(&g_RWLock);
						iface_node_state_change(node, DEV_EVENT_LINK_UP);
						RWLOCK_WUNLOCK(&g_RWLock);
						/* notify manager module with nanomsg */
						l3_uplink_notify(node);
						continue;
					}
				}
			}
			
			if(node->state != DEV_EVENT_LINK_DOWN) {
				RWLOCK_WLOCK(&g_RWLock);
				iface_node_state_change(node, DEV_EVENT_LINK_DOWN);
				RWLOCK_WUNLOCK(&g_RWLock);
			}
			l3_uplink_notify(node);
		}
		sleep(g_conf->period);
	}
}


