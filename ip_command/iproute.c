#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <linux/in_route.h>
#include <net/if.h>
#include <errno.h>

#include "basedef.h"
#include "ip_common.h"

#ifndef RTAX_RTTVAR
#define RTAX_RTTVAR RTAX_HOPS
#endif

struct rtnl_handle rth = { .fd = -1 };

enum list_action {
	IPROUTE_LIST,
	IPROUTE_FLUSH,
	IPROUTE_SAVE,
};

unsigned int ifname_to_index(const char *ifname)
{
    unsigned int if_index = 0;

    if (NULL == ifname) {
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

int calc_host_len(struct rtmsg *r)
{
	if (r->rtm_family == AF_INET6)
		return 128;
	else if (r->rtm_family == AF_INET)
		return 32;
	else if (r->rtm_family == AF_DECnet)
		return 16;
	else if (r->rtm_family == AF_IPX)
		return 80;
	else
		return -1;
}

int iproute_modify(int cmd, unsigned flags, IPROUTE_T rt) 
{
	struct {
		struct nlmsghdr 	n;
		struct rtmsg 		r;
		char   			buf[1024];
	} req;
	char  mxbuf[256];
	struct rtattr * mxrta = (void*)mxbuf;
	unsigned mxlock = 0;
	int idx;
	int gw_ok = 0;
	int dst_ok = 0;
	int nhs_ok = 0;
	int scope_ok = 0;
	int table_ok = 0;
	int raw = 0;

	memset(&req, 0, sizeof(req));
	req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
	req.n.nlmsg_flags = NLM_F_REQUEST|flags;
	req.n.nlmsg_type = cmd;
	req.r.rtm_family = AF_INET;
	req.r.rtm_table = RT_TABLE_MAIN;
	req.r.rtm_scope = RT_SCOPE_NOWHERE;

	if (cmd != RTM_DELROUTE) {
		req.r.rtm_protocol = RTPROT_BOOT;
		req.r.rtm_scope = RT_SCOPE_UNIVERSE;
		req.r.rtm_type = RTN_UNICAST;
	}

	mxrta->rta_type = RTA_METRICS;
	mxrta->rta_len = RTA_LENGTH(0);

	req.r.rtm_dst_len = rt.dst_len;
    req.r.rtm_src_len = 0;
    req.r.rtm_tos = 0;
    req.r.rtm_flags = 0;

	if ((idx = ifname_to_index(rt.ifname)) == 0) {
		fprintf(stderr, "Cannot find device \"%s\"\n", rt.ifname);
		return -1;
	}
	addattr32(&req.n, sizeof(req), RTA_OIF, idx);

	 /* fill the netlink rtmsg attr */
	if (rt.dst) {
		addattr_l(&req.n, sizeof(req), RTA_DST, &rt.dst, sizeof(rt.dst));
		dst_ok = 1;
	}

	if (rt.gw) {
		addattr_l(&req.n, sizeof(req), RTA_GATEWAY, &rt.gw, sizeof(rt.gw));
		gw_ok = 1;
	}

    if (rt.tid > 0) {
		__u32 tid;
		tid = rt.tid;
        if (tid < 256) {
            req.r.rtm_table = tid;
        } else {
            req.r.rtm_table = RT_TABLE_UNSPEC;
            addattr32(&req.n, sizeof(req), RTA_TABLE, tid);
        }   
		table_ok = 1;
    }  

	__u32 metric;
	metric = rt.metric;
	addattr_l(&req.n, sizeof(req), RTA_PRIORITY, &metric, sizeof(metric));
	addattr_l(&req.n, sizeof(req), RTA_METRICS, RTA_DATA(mxrta), RTA_PAYLOAD(mxrta));

	if (!table_ok) {
		if (req.r.rtm_type == RTN_LOCAL ||
		    req.r.rtm_type == RTN_BROADCAST ||
		    req.r.rtm_type == RTN_NAT ||
		    req.r.rtm_type == RTN_ANYCAST)
			req.r.rtm_table = RT_TABLE_LOCAL;
	}
	if (!scope_ok) {
		if (req.r.rtm_type == RTN_LOCAL ||
		    req.r.rtm_type == RTN_NAT)
			req.r.rtm_scope = RT_SCOPE_HOST;
		else if (req.r.rtm_type == RTN_BROADCAST ||
			 req.r.rtm_type == RTN_MULTICAST ||
			 req.r.rtm_type == RTN_ANYCAST)
			req.r.rtm_scope = RT_SCOPE_LINK;
		else if (req.r.rtm_type == RTN_UNICAST ||
			 req.r.rtm_type == RTN_UNSPEC) {
			if (cmd == RTM_DELROUTE)
				req.r.rtm_scope = RT_SCOPE_NOWHERE;
			else if (!gw_ok && !nhs_ok)
				req.r.rtm_scope = RT_SCOPE_LINK;
		}
	}

	if (req.r.rtm_family == AF_UNSPEC)
		req.r.rtm_family = AF_INET;

	if (rtnl_talk(&rth, &req.n, 0, 0, NULL, NULL, NULL) < 0)
		return -1;

	return 0;
}

int do_iproute(IPROUTE_T rt_info, int flags)
{
	int ret = -1;

	if (rtnl_open(&rth, 0) < 0) {
		fprintf(stderr, "Cannot open rtnetlink\n");
		return -1;
	}

	if (flags == ADD)
		ret = iproute_modify(RTM_NEWROUTE, NLM_F_CREATE|NLM_F_EXCL, rt_info);
	else if (flags == DEL)
		ret = iproute_modify(RTM_DELROUTE, 0, rt_info);

	rtnl_close(&rth);
	return ret;
}

int iproute_add(unsigned int dstaddr, int mask, unsigned int gw, int metric, int table, const char *ifname)
{
	IPROUTE_T rt_info;

	memset(&rt_info, 0, sizeof(rt_info));
	rt_info.dst = dstaddr;
	rt_info.dst_len = mask;
	rt_info.gw = gw;
	rt_info.metric = metric;
	rt_info.tid = table;
	strcpy(rt_info.ifname, ifname);

	return do_iproute(rt_info, ADD);
}

int iproute_del(unsigned int dstaddr, int mask, unsigned int gw, int metric, int table, const char *ifname) 
{
	IPROUTE_T rt_info;

	memset(&rt_info, 0, sizeof(rt_info));
	rt_info.dst = dstaddr;
	rt_info.dst_len = mask;
	rt_info.gw = gw;
	rt_info.metric = metric;
	rt_info.tid = table;
	strcpy(rt_info.ifname, ifname);

	return do_iproute(rt_info, DEL);
}

