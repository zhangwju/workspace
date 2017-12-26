#ifndef __BASEDEF_H__
#define __BASEDEF_H__
#include "libnetlink.h"
#include "ip_common.h"

#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"
#define STR2MAC(x) &(x)[0],&(x)[1],&(x)[2],&(x)[3],&(x)[4],&(x)[5]
#define STR_TO_MAC(str, mac) \
    sscanf(str, MACSTR, STR2MAC((mac)))

#define IPSTR "%u.%u.%u.%u"
#define IP2STR(a)  ((a)>>24)&0xff, ((a)>>16)&0xff, ((a)>>8)&0xff, (a)&0xff

#define log_dbg(format, ...) fprintf(stderr, "[%s():%d] "format"\n", __FUNCTION__, __LINE__, ## __VA_ARGS__)


#ifndef IFNAMESZ
#define IFNAMESZ            16
#endif

#ifndef USERNAMESZ
#define USERNAMESZ          32
#endif

#ifndef uchar 
#define uchar unsigned char
#endif

#ifndef ushort
#define ushort unsigned short
#endif

#ifndef uint
#define uint unsigned int
#endif

#ifndef ulong
#define ulong unsigned long
#endif

#ifndef ullong
#define ullong unsigned long long
#endif

#ifndef u8
#define u8 unsigned char
#endif

#ifndef u16
#define u16 unsigned short int
#endif

#ifndef u32
#define u32 unsigned int
#endif

#ifndef u64
#define u64 unsigned long
#endif
typedef struct {
	uint src;
	uint dst;
	uint gw;
	int src_len;
	int dst_len;
	int tid;
	int metric;
	char ifname[IFNAMESZ];

}IPROUTE_T;

typedef struct {
	uint src;		/* [src] of equal [from] */
	uint dst;		/* [dst] of equal [to] */
	uint fwmark;
	int src_len;	/* netmask 8/16/24 ... */
	int dst_len;
	int tos;
	int tid;		/* table id */
	int pref;
	int invert;
	char oif[IFNAMESZ];
	char iif[IFNAMESZ];
	
}IPRULE_T;

typedef struct ipaddr_info {
	uint addr;
	
}IPADDR_T;

enum {
	ADD,
	DEL,
	GET,
	FLUSH
};

int do_iproute(IPROUTE_T rt_info, int flags);
int do_iprule(IPRULE_T rl_info, int flags);

#endif //__BASEDEF_H__
