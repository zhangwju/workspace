#ifndef __COMMON_H__
#define __COMMON_H__
#include "logger.h"

#define RWLOCK_WLOCK(rwlock) 			pthread_rwlock_wrlock(rwlock)
#define RWLOCK_WUNLOCK(rwlock) 			pthread_rwlock_unlock(rwlock)
#define RWLOCK_RLOCK(rwlock) 			pthread_rwlock_rdlock(rwlock)
#define RWLOCK_RUNLOCK(rwlock) 			pthread_rwlock_unlock(rwlock)

#ifndef IFNAMESZ
#define IFNAMESZ            16
#endif

#ifndef USERNAMESZ
#define USERNAMESZ          32
#endif

#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"
#define STR2MAC(x) &(x)[0],&(x)[1],&(x)[2],&(x)[3],&(x)[4],&(x)[5]
#define STR_TO_MAC(str, mac) \
    sscanf(str, MACSTR, STR2MAC((mac)))

#define IPSTR "%u.%u.%u.%u"
#define IP2STR(a)  ((a)>>24)&0xff, ((a)>>16)&0xff, ((a)>>8)&0xff, (a)&0xff

#define LINK_DETECT_LOG

#ifdef LINK_DETECT_LOG
#define log_dbg(...) log_print(__FILE__, __LINE__, LOG_LEVEL_DEBUG, __VA_ARGS__)
#else
#define log_dbg(format, ...) fprintf(stderr, "[%s():%d] "format"\n", __FUNCTION__, __LINE__, ## __VA_ARGS__)
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

enum link_type {
	UPLINK_FIBER,
	UPLINK_LTE,
	UPLINK_MICROWAVE,
	UPLINK_SATELLITE,
	DOWNLINK_LAN = 10,
	LINK_UNKNOWN
};

enum link_status {
	UPLINK_STATUS_DOWN,
	UPLINK_STATUS_GOOD,
	UPLINK_STATUS_POOR,
	UPLINK_STATUS_EXCELLENT
};

/* events broadcasted to all users of a device */
enum device_event {
	DEV_EVENT_ADD,
	DEV_EVENT_REMOVE,

	DEV_EVENT_UPDATE_IFNAME,
	DEV_EVENT_UPDATE_IFINDEX,

	DEV_EVENT_SETUP,
	DEV_EVENT_TEARDOWN,
	DEV_EVENT_UP,
	DEV_EVENT_DOWN,

	DEV_EVENT_LINK_UP,
	DEV_EVENT_LINK_DOWN,

	/* Topology changed (i.e. bridge member added) */
	DEV_EVENT_TOPO_CHANGE,

	__DEV_EVENT_MAX
};

/* uplink info of globle config */
/*********************************************************/
//new
struct globalargs {
	int period;
	unsigned int detect_addr;
};

struct netinfo {
	unsigned int ip;			
	unsigned int mask;
	unsigned int gw;
	unsigned int dns;
};

struct uplink_info {
	char ifname[IFNAMESZ];		/* detect interface */
	int	type;					/* 0: Fibre 1: 4G 2: Microwaves 3: Statellite */
	int subid;					/* for 4G 0,1,2... */
	int status;					/* 0: disconnect 1: excellent 2: good 3:poor */
	int rate;					/* rate of transmission of currnet */
	struct netinfo net;
	
	/* detect info */
	int sockfd;				
	int crtflag;				/* control flag of golable */
	int errcnt;
	int send_ok;				/* 0:ok 1:nok */
	int recv_ok;				
	int l2_state;				/* layer 2 status */
};

struct uplinks {
	int uplinks;
	struct uplink_info uplink[0];
};

struct linkmgt_info {
	struct globalargs args;
	struct uplinks dev;
};

/*********************************************************/

/*********************************************************/
#pragma pack(1)
struct modlue_notify_info {
	int msg_id;
	int msg_len;
	char data[0];
};

struct uplink_info_report {
	char ifname[IFNAMESZ];
	int uplink_type; //0: Fibre 1: 4G 2: Microwaves 3: Statellite
	int uplink_subid; // for 4G
 	int status; //0: disconnect 1: excellent 2: good 3:poor
	int rate; // M/s
	uint ip;
	uint mask;
	uint gw;
	uint dns;
};
#pragma pack()
/*********************************************************/

extern const char *uplink_type[];
#endif //__COMMON_H__
