#ifndef __COMMON_H__
#define __COMMON_H__

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

#define log_dbg(format, ...) fprintf(stderr, "[%s():%d] "format"\n", __FUNCTION__, __LINE__, ## __VA_ARGS__)

#ifndef uchar 
#define unsigned char
#endif

#ifndef ushort
#define ushort unsigned short int
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
	DOWNLINK_LAN,
	LINK_UNKNOWN
};

enum link_status {
	DEV_LINK_DOWN,
	DEV_LINK_UP
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

#pragma pack(1)
struct modlue_notify_info {
	int msg_id;
	char data[0];
};

struct uplink_info_report {
	int uplink_type; //0£ºfibre 1£º4G£»2:Microwaves 3: moons
	int uplink_subid; // for 4G
 	int status; //0: disconnect 1: excellent 2: good 3:poor
	int rate; // M/s
};
#pragma pack()
#endif //__COMMON_H__
