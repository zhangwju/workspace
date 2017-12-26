#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <string.h>
#include <linux/fib_rules.h>

#include "basedef.h"
#include "ip_common.h"

extern struct rtnl_handle rth;

static int iprule_modify(int cmd, IPRULE_T rl)
{
	int table_ok = 0;
	struct {
		struct nlmsghdr 	n;
		struct rtmsg 		r;
		char   			buf[1024];
	} req;

	memset(&req, 0, sizeof(req));

	req.n.nlmsg_type = cmd;
	req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
	req.n.nlmsg_flags = NLM_F_REQUEST;
	req.r.rtm_family = AF_INET;
	req.r.rtm_protocol = RTPROT_BOOT;
	req.r.rtm_scope = RT_SCOPE_UNIVERSE;
	req.r.rtm_table = 0;
	req.r.rtm_type = RTN_UNSPEC;
	req.r.rtm_flags = 0;

	if (cmd == RTM_NEWRULE) {
		req.n.nlmsg_flags |= NLM_F_CREATE|NLM_F_EXCL;
		req.r.rtm_type = RTN_UNICAST;
	}

	if (rl.invert) {
		req.r.rtm_flags |= FIB_RULE_INVERT;
	} 

	if (rl.src) {
		req.r.rtm_src_len = rl.src_len;
		addattr_l(&req.n, sizeof(req), FRA_SRC, &rl.src, sizeof(rl.src));
	} 
	
	if (rl.dst) {
		req.r.rtm_dst_len = rl.dst_len;
		addattr_l(&req.n, sizeof(req), FRA_DST, &rl.dst, sizeof(rl.dst));
	}
	
	if (rl.pref) {
		__u32 pref = rl.pref;
		addattr32(&req.n, sizeof(req), FRA_PRIORITY, pref);
	}

	if (rl.tos) {
		__u32 tos = rl.tos;
		req.r.rtm_tos = tos;
	} 
	if (rl.fwmark) {
		__u32 fwmark = rl.fwmark;
		addattr32(&req.n, sizeof(req), FRA_FWMARK, fwmark);
	} 

	if (rl.tid) {
		__u32 tid = rl.tid;
		if (tid < 256)
			req.r.rtm_table = tid;
		else {
			req.r.rtm_table = RT_TABLE_UNSPEC;
			addattr32(&req.n, sizeof(req), FRA_TABLE, tid);
		}
		table_ok = 1;
	} 

	if (rl.iif[0]) {
		addattr_l(&req.n, sizeof(req), FRA_IFNAME, rl.iif, strlen(rl.iif)+1);
	}

	if (rl.oif[0]) {
		addattr_l(&req.n, sizeof(req), FRA_OIFNAME, rl.oif, strlen(rl.oif)+1);
	}

	if (req.r.rtm_family == AF_UNSPEC)
		req.r.rtm_family = AF_INET;

	if (!table_ok && cmd == RTM_NEWRULE)
		req.r.rtm_table = RT_TABLE_MAIN;

	if (rtnl_talk(&rth, &req.n, 0, 0, NULL, NULL, NULL) < 0)
		return 2;

	return 0;
}


static int flush_rule(const struct sockaddr_nl *who, struct nlmsghdr *n, void *arg)
{
	struct rtnl_handle rth2;
	struct rtmsg *r = NLMSG_DATA(n);
	int len = n->nlmsg_len;
	struct rtattr * tb[FRA_MAX+1];

	len -= NLMSG_LENGTH(sizeof(*r));
	if (len < 0)
		return -1;

	parse_rtattr(tb, FRA_MAX, RTM_RTA(r), len);

	if (tb[FRA_PRIORITY]) {
		n->nlmsg_type = RTM_DELRULE;
		n->nlmsg_flags = NLM_F_REQUEST;

		if (rtnl_open(&rth2, 0) < 0)
			return -1;

		if (rtnl_talk(&rth2, n, 0, 0, NULL, NULL, NULL) < 0)
			return -2;

		rtnl_close(&rth2);
	}

	return 0;
}

static int iprule_flush(int argc, char **argv)
{
	int af = AF_INET;

	if (af == AF_UNSPEC)
		af = AF_INET;

	if (argc > 0) {
		fprintf(stderr, "\"ip rule flush\" does not allow arguments\n");
		return -1;
	}

	if (rtnl_wilddump_request(&rth, af, RTM_GETRULE) < 0) {
		perror("Cannot send dump request");
		return 1;
	}

	if (rtnl_dump_filter(&rth, flush_rule, NULL, NULL, NULL) < 0) {
		fprintf(stderr, "Flush terminated\n");
		return 1;
	}

	return 0;
}

int do_iprule(IPRULE_T rl_info, int flags)
{
	int ret = -1;

	if (rtnl_open(&rth, 0) < 0) {
		fprintf(stderr, "Cannot open rtnetlink\n");
		return -1; 
	}

	if (flags == ADD) {
		ret = iprule_modify(RTM_NEWRULE, rl_info);
	} else if (flags == DEL) {
		ret =  iprule_modify(RTM_DELRULE, rl_info);
	} else if (flags == FLUSH) {
	//	ret = iprule_flush();
	}

	rtnl_close(&rth);

	return ret; 
}

int iprule_add(unsigned int src, int src_len, unsigned int dst, unsigned int dst_len, 
	unsigned int fwmark, int pref, int table, int invert, const char *iif, const char *oif)
{
	IPRULE_T rl_info;

	memset(&rl_info, 0, sizeof(rl_info));
	rl_info.src = src;
	rl_info.src_len = src_len;
	rl_info.dst = dst;
	rl_info.dst_len = dst_len;
	rl_info.fwmark = fwmark;
	rl_info.pref = pref;
	rl_info.tid = table;
	rl_info.invert = invert;

	if (iif) {
		strcpy(rl_info.iif, iif);
	}

	if (oif) {
		strcpy(rl_info.oif, oif);
	}

	return do_iprule(rl_info, ADD);
}

int iprule_del(unsigned int src, int src_len, unsigned int dst, unsigned int dst_len, 
	unsigned int fwmark, int pref, int table, int invert, const char *iif, const char *oif)
{
	IPRULE_T rl_info;

	memset(&rl_info, 0, sizeof(rl_info));
	rl_info.src = src;
	rl_info.src_len = src_len;
	rl_info.dst = dst;
	rl_info.dst_len = dst_len;
	rl_info.fwmark = fwmark;
	rl_info.pref = pref;
	rl_info.tid = table;
	rl_info.invert = invert;

	if (iif) {
		strcpy(rl_info.iif, iif);
	}

	if (oif) {
		strcpy(rl_info.oif, oif);
	}

	return do_iprule(rl_info, DEL);
}

