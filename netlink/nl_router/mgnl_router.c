/*************************************
 * Filename: nl_route.c
 * Author: zhangwj
 * Description: 
 * Date: 2017-07-14
 ************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <errno.h>
#include <arpa/inet.h>
#include <net/if.h>
#include "mgnl_router.h"

#define	ATTR_LEN	1024
#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"
#define STR2MAC(x) &(x)[0],&(x)[1],&(x)[2],&(x)[3],&(x)[4],&(x)[5]
#define STR_TO_MAC(str, mac) \
    sscanf(str, MACSTR, STR2MAC((mac)))

#define IPSTR "%u.%u.%u.%u"
#define IP2STR(a) (a)&0xff, ((a)>>8)&0xff, ((a)>>16)&0xff, ((a)>>24)&0xff

typedef struct _nl_rtreq{
	struct nlmsghdr		nh;
	struct rtmsg		r;
	char	attrbuf[ATTR_LEN];
}nl_rtreq_t;

unsigned int ifname_to_index(const char *ifname)
{
	unsigned int if_index = 0;

	if(NULL == ifname) {
		return -1;
	}

	if_index = if_nametoindex(ifname);
	if (if_index == 0) {
		fprintf(stderr, "Interface %s : No such device\n", ifname);
		return -1;
	}  
	return if_index;
}

unsigned int if_index_to_name(unsigned int if_index, char *if_name)
{
    char *name = NULL;
    name = if_indextoname(if_index, if_name);
    if(NULL == name && errno == ENXIO) {
        fprintf(stderr, "Index %d : No such device\n", if_index);
        return -1; 
    }   

    return if_index;
}

static inline int rtm_get_table(struct rtmsg *r, struct rtattr **tb)
{
	unsigned int table = r->rtm_table;
	if (tb[RTA_TABLE])
		table = *(unsigned int*) RTA_DATA(tb[RTA_TABLE]);
	return table;
}

int parse_rtattr(struct rtattr *tb[], int max, struct rtattr *rta, int len)
{
	memset(tb, 0, sizeof(struct rtattr *) * (max + 1));
	while (RTA_OK(rta, len)) {
		if ((rta->rta_type <= max) && (!tb[rta->rta_type]))
			tb[rta->rta_type] = rta;
		rta = RTA_NEXT(rta,len);
	}

	return 0;
}

int addattr_rt(struct nlmsghdr *n, int maxlen, int type, void *data, unsigned int rlen)
{
    int len = RTA_LENGTH(rlen);
    struct rtattr *rta;

    if (NLMSG_ALIGN(n->nlmsg_len) + len > maxlen)
        return -1; 

    rta = (struct rtattr*)(((char*)n) + NLMSG_ALIGN(n->nlmsg_len));
    rta->rta_type = type;
    rta->rta_len = len;
    memcpy(RTA_DATA(rta), data, rlen);
    n->nlmsg_len = NLMSG_ALIGN(n->nlmsg_len) + len;

    return 0;
}

int addattr32(struct nlmsghdr *n, int maxlen, int type, __u32 data)
{
	int len = RTA_LENGTH(4);
	struct rtattr *rta;
	if (NLMSG_ALIGN(n->nlmsg_len) + len > maxlen) {
		fprintf(stderr,"addattr32: Error! max allowed bound %d exceeded\n",maxlen);
		return -1;
	}
	rta = (struct rtattr*)(((char*)n) + NLMSG_ALIGN(n->nlmsg_len));
	rta->rta_type = type;
	rta->rta_len = len;
	memcpy(RTA_DATA(rta), &data, 4);
	n->nlmsg_len = NLMSG_ALIGN(n->nlmsg_len) + len;
	return 0;
}

static void  show_rtinfo(nl_rtinfo_t *rt_info)
{
	static int cnt = 0;
	if (cnt == 0) {
		printf("Destination		Gateway		Genmask		Metric	Iface	Table\n");
	}

	printf(""IPSTR"		", IP2STR(rt_info->dst));
	printf(""IPSTR"		", IP2STR(rt_info->gw));
	printf("%u		", rt_info->mask);
	printf("%d	", rt_info->metric);
	printf("%s	", rt_info->ifname);
	printf("%d\n", rt_info->tid);
	cnt++;
}

int rtnl_send_check(struct mgnl_socket *nl, const char *buf, int len)
{
	struct nlmsghdr *h;
	int status;
	char resp[1024];

	status = send(nl->fd, buf, len, 0);
	if (status < 0)
		return status;

	/* Check for immediate errors */
	status = recv(nl->fd, resp, sizeof(resp), MSG_DONTWAIT|MSG_PEEK);
	if (status < 0) {
		if (errno == EAGAIN)
			return 0;
		return -1;
	}

	for (h = (struct nlmsghdr *)resp; NLMSG_OK(h, status);
	     h = NLMSG_NEXT(h, status)) {
		if (h->nlmsg_type == NLMSG_ERROR) {
			struct nlmsgerr *err = (struct nlmsgerr*)NLMSG_DATA(h);
			if (h->nlmsg_len < NLMSG_LENGTH(sizeof(struct nlmsgerr)))
				fprintf(stderr, "ERROR truncated\n");
			else 
				errno = -err->error;
			return -1;
		}
	}

	return 0;
}

int mgnl_table_flush(struct mgnl_socket *nl, const char *nlmsg, int msg_len, int tid)
{
	unsigned int table;
	struct nlmsghdr *n;
	struct rtmsg *r;
	int len;
	struct rtattr * tb[RTA_MAX+1];
	struct nlmsghdr *fn;
	char nl_buf[1024];
	
	fn = (struct nlmsghdr *)nl_buf;
	n = (struct nlmsghdr *)nlmsg;
	for(; NLMSG_OK(n, msg_len); n = NLMSG_NEXT(n, msg_len)) {
		r = NLMSG_DATA(n);
		len = n->nlmsg_len;
		parse_rtattr(tb, RTA_MAX, RTM_RTA(r), len);
		table = rtm_get_table(r, tb);

		if (table == tid) {
			memcpy(fn, n, n->nlmsg_len);
			fn->nlmsg_type 	= RTM_DELROUTE;
			fn->nlmsg_flags = NLM_F_REQUEST;
			fn->nlmsg_seq 	= nl->seq++;
			rtnl_send_check(nl, nl_buf, n->nlmsg_len);
		}
	}

	return 0;
}

int mgnl_dump_filter(struct mgnl_socket *nl, int table)
{
	int status;
	int ret, size;
	fd_set rfds;
	struct timeval timeout;
	struct nlmsghdr *nh;
	struct nlmsgerr *nlmsg_error;
	char nl_buf[2048];

	FD_ZERO(&rfds);			//clear fd set 
	FD_SET(nl->fd, &rfds);	//add nlfd to fd_set
	
	/* set select timeout */
	timeout.tv_sec = 2;   
	timeout.tv_usec = 0;
	
	ret = select(nl->fd+1, &rfds, NULL, NULL, &timeout);
	switch(ret) {
	case 0:
		perror("select timeout");
		return -1;
	case -1:
		perror("select error\n");
		return -1;
	default:
		if(FD_ISSET(nl->fd, &rfds)) { 
			while(1) {
				status = recv(nl->fd, nl_buf, sizeof(nl_buf), 0);
				 if(status < 0) {
					if (errno == EINTR || errno == EAGAIN)
						continue;
					fprintf(stderr, "netlink receive error %s (%d)\n",
						strerror(errno), errno);
					return -1;
				}

				if (status == 0) {
					fprintf(stderr, "EOF on netlink\n");
					return -1;
				}
	
				nh = (struct nlmsghdr *)nl_buf;
				size = status;
				
				if(NLMSG_OK(nh, size)) { 
					if(nh->nlmsg_type == NLMSG_DONE) {
						break;
					}

					if (nh->nlmsg_pid != nl->addr.nl_pid ||
						nh->nlmsg_seq != nl->dump) {
						continue;
					}

					if ((nh->nlmsg_type == NLMSG_ERROR)) {
						nlmsg_error = (struct nlmsgerr *)NLMSG_DATA(nh);
						if (nh->nlmsg_len < NLMSG_LENGTH(sizeof(struct nlmsgerr))) {
							fprintf(stderr, "ERROR truncated\n");
						} else {
							errno = -nlmsg_error->error;
							perror("RTNETLINK answers");
						}
						return -1;
					}
					mgnl_table_flush(nl, nl_buf, size, table);	
				}
			}
    		}
	}	
	return 0;
}

int mgnl_dump_request(struct mgnl_socket *nl, int type)
{
	struct {
		struct nlmsghdr nlh;
		struct rtgenmsg g;
	} req;

	memset(&req, 0, sizeof(req));

	req.nlh.nlmsg_len 	= sizeof(req);
	req.nlh.nlmsg_type 	= RTM_GETROUTE;
	req.nlh.nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST;
	req.nlh.nlmsg_seq 	= nl->dump = nl->seq++;
	req.nlh.nlmsg_pid 	= 0;
	req.g.rtgen_family 	= AF_INET;

	if (send(nl->fd, (void*)&req, sizeof(req), 0) < 0) {
		perror("send netlink failure\n");
		return -1;
	}

	return 0;
}

int nl_rtinfo_parse(struct nlmsghdr *nlh, int type)
{
	struct rtattr *rt_attr;  
	struct rtmsg *rt_msg;  
	int rt_len;
	nl_rtinfo_t rt_info;
	
	rt_msg = NLMSG_DATA(nlh);  
	rt_attr = (struct rtattr *)RTM_RTA(rt_msg);
	rt_len = RTM_PAYLOAD(nlh);

	memset(&rt_info, 0, sizeof(rt_info));
	rt_info.tid = rt_msg->rtm_table;
	rt_info.mask = rt_msg->rtm_dst_len;
	
	for(; RTA_OK(rt_attr,rt_len); rt_attr=RTA_NEXT(rt_attr,rt_len)) {
		switch(rt_attr->rta_type) {
		case RTA_OIF:
			if_indextoname(*(int *)RTA_DATA(rt_attr), rt_info.ifname);
			break;
		case RTA_GATEWAY:
			rt_info.gw = *(unsigned int *)RTA_DATA(rt_attr);
			break;
		case RTA_PREFSRC:
			rt_info.src = *(unsigned int *)RTA_DATA(rt_attr);
			break;
		case RTA_DST:
			rt_info.dst = *(unsigned int  *)RTA_DATA(rt_attr);
			break;
		case RTA_METRICS:
			rt_info.metric = *(int *)RTA_DATA(rt_attr);
			break;
		}
	} 	
	show_rtinfo(&rt_info);

	return 0;
}

int nl_respose_handle(const char *nlmsg, int msg_len, int type)
{
	int ret = -1;
	int r_size = msg_len;
	struct nlmsghdr *nlh;
	nlh = ( struct nlmsghdr *)nlmsg;

	for(; NLMSG_OK(nlh, r_size); nlh = NLMSG_NEXT(nlh, r_size)) {
		if(nlh->nlmsg_type == type) {
			nl_rtinfo_parse(nlh, type);
			ret = 0;
		}
	}
	return ret;
}

int nl_route_recv(const struct mgnl_socket *nl, char *nl_buf, int buflen)
{
	int status;
	int ret, size;
	int  msglen = 0;
	fd_set rfds;
	struct timeval timeout;
	struct nlmsghdr *nh;
	struct nlmsgerr *nlmsg_error;

	FD_ZERO(&rfds);			//clear fd set 
	FD_SET(nl->fd, &rfds);	//add nlfd to fd_set
	
	/* set select timeout */
	timeout.tv_sec = 2;
	timeout.tv_usec = 0;
	
	ret = select(nl->fd+1, &rfds, NULL, NULL, &timeout);
	switch(ret) {
	case 0:
		perror("select timeout");
		return -1;
	case -1:
		perror("select error\n");
		return -1;
	default:
		if(FD_ISSET(nl->fd, &rfds)) { 
			do {
				status = recv(nl->fd, nl_buf, buflen - msglen, 0);
				 if(status < 0) {
					if (errno == EINTR || errno == EAGAIN)
						continue;
					fprintf(stderr, "netlink receive error %s (%d)\n",
						strerror(errno), errno);
					return -1;
				}
					
				nh = (struct nlmsghdr *)nl_buf;
				size = status;
				if(NLMSG_OK(nh, size)) { 

					if(nh->nlmsg_type == NLMSG_DONE) {
						break;
					}

					if ((nh->nlmsg_type == NLMSG_ERROR)) {
						nlmsg_error = (struct nlmsgerr *)NLMSG_DATA(nh);
						if (nh->nlmsg_len < NLMSG_LENGTH(sizeof(struct nlmsgerr))) {
							fprintf(stderr, "ERROR truncated\n");
						} else {
							errno = -nlmsg_error->error;
							perror("RTNETLINK answers");
						}
						return -1;
					}

					nl_buf += size;
					msglen += size;
					if((nh->nlmsg_flags & NLM_F_MULTI)) {
						break;
					}
				}
			} while((nh->nlmsg_seq != nl->seq) || (nh->nlmsg_pid != nl->addr.nl_pid));
    	}
	}

	return msglen;
}

int nl_route_handle(struct mgnl_socket *nl, nl_rtinfo_t *rt, int type)
{
	char nl_buf[1024];
	unsigned int index = -1;
	nl_rtreq_t req;
	int size;
	char  mxbuf[256];
	struct rtattr * mxrta = (void*)mxbuf;

	index = ifname_to_index(rt->ifname);
	if(index < 0) {
		return -1;
	}

	/* METRICS */
	mxrta->rta_type = RTA_METRICS;
	mxrta->rta_len = RTA_LENGTH(0);

	/* Fill the netlink message header */
	memset(&req, 0, sizeof(req));
	req.nh.nlmsg_len	= NLMSG_LENGTH(sizeof(struct rtmsg));
	req.r.rtm_family	= AF_INET;
	req.r.rtm_table		= RT_TABLE_MAIN; /* route table (....) */
	req.nh.nlmsg_pid	= getpid();

	switch(type){
	case RTM_NEWROUTE:
		req.nh.nlmsg_flags	= NLM_F_REQUEST | NLM_F_CREATE | NLM_F_EXCL;//| NLM_F_ACK;
		req.nh.nlmsg_type	= RTM_NEWROUTE;
		req.r.rtm_protocol	= RTPROT_STATIC; /* by the administrator */
		req.r.rtm_scope		= RT_SCOPE_UNIVERSE;
		req.r.rtm_type		= RTN_UNICAST;
		break;
	case RTM_DELROUTE:
		req.nh.nlmsg_flags	= NLM_F_REQUEST;//| NLM_F_ACK;
		req.nh.nlmsg_type	= RTM_DELROUTE;
		break;
	default:
		fprintf(stderr, "type: %d unkonwn\n", type);
		return -1;
	}

	/* Fill the netlink message rtmsg */
	req.r.rtm_dst_len = rt->mask;
	req.r.rtm_src_len = 0;
	req.r.rtm_tos = 0;
	req.r.rtm_flags = 0;
	
	/* fill the netlink rtmsg attr */
	addattr_rt(&req.nh, sizeof(req), RTA_DST, &rt->dst, sizeof(rt->dst));
	addattr_rt(&req.nh, sizeof(req), RTA_GATEWAY, &rt->gw, sizeof(rt->gw));
	addattr_rt(&req.nh, sizeof(req), RTA_OIF, &index, sizeof(index));

	if (rt->tid > 0) {
		if (rt->tid < 256) {
			req.r.rtm_table = rt->tid;
		} else {
			req.r.rtm_table = RT_TABLE_UNSPEC;
			addattr32(&req.nh, sizeof(req), RTA_TABLE, rt->tid);
		}
	}

	addattr32(&req.nh, sizeof(req), RTA_PRIORITY, rt->metric);
	if (mxrta->rta_len > RTA_LENGTH(0)) { 
		addattr_rt(&req.nh, sizeof(req), RTA_METRICS, RTA_DATA(mxrta), RTA_PAYLOAD(mxrta));
	}

	if (send(nl->fd, &req, req.nh.nlmsg_len, 0) < 0) {
		perror("send netlink failure\n");
		return -1;
	}
	
	size = nl_route_recv(nl, nl_buf, sizeof(nl_buf));
	if(size < 0) {
		return -1;
	}

	if (nl_respose_handle(nl_buf, size, type) < 0) {
		return -1;
	}

	return 0;
}

int nl_route_get(struct mgnl_socket *nl)
{
	int size;
	char nl_buf[1024];
	
	if (mgnl_dump_request(nl, RTM_GETROUTE) < 0) {
		return -1;
	}

	size = nl_route_recv(nl, nl_buf, sizeof(nl_buf));
	if(size < 0) {
		return -1;
	}

	nl_respose_handle(nl_buf, size, RTM_NEWROUTE);
	return 0;
}

int nl_route_del(struct mgnl_socket *nl, nl_rtinfo_t *rt)
{
	return nl_route_handle(nl, rt, RTM_DELROUTE);
}

int nl_route_add(struct mgnl_socket *nl, nl_rtinfo_t *rt)
{
	
	return nl_route_handle(nl, rt, RTM_NEWROUTE);
}

int nl_route_flush(struct mgnl_socket *nl, int table)
{
	if (mgnl_dump_request(nl, RTM_GETROUTE) < 0) {
		return -1;
	}

	if (mgnl_dump_filter(nl, table) < 0) {
		return -1;
	}

	return 0;
}

struct mgnl_socket *mgnl_init()
{
	pid_t pid;
	struct mgnl_socket *nl = NULL;

	nl = mgnl_socket_open(NETLINK_ROUTE);
	if(NULL == nl) {
		return NULL;
	}
	
	pid = getpid();
	if (mgnl_socket_bind(nl, RTMGRP_IPV4_ROUTE, pid) < 0) {
		close(nl->fd);
		free(nl);
		return NULL;
	}
	
	return nl;
}

void mgnl_release(struct mgnl_socket *nl)
{
	mgnl_socket_close(nl);
}

