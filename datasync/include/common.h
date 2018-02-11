#ifndef __COMMON_H__
#define __COMMON_H__

#define dsync_lock(lock)			pthread_mutex_lock(lock)
#define dsync_unlock(lock)			pthread_mutex_unlock(lock)

#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"
#define STR2MAC(x) &(x)[0],&(x)[1],&(x)[2],&(x)[3],&(x)[4],&(x)[5]
#define STR_TO_MAC(str, mac) \
    sscanf(str, MACSTR, STR2MAC((mac)))

#define IPSTR "%u.%u.%u.%u"
#define IP2STR(a)  ((a)>>24)&0xff, ((a)>>16)&0xff, ((a)>>8)&0xff, (a)&0xff

#define log_dbg(format, ...) fprintf(stderr, "[%s():%d] "format"\n", __FUNCTION__, __LINE__, ## __VA_ARGS__)

#ifndef IFNAMESZ
#define IFNAMESZ 16
#endif 

#ifndef USERNAMESZ 
#define USERNAMESZ 32
#endif

#define port_isvalid(port)		((port >=1024) && (port <= 50000))

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
#define u8 uchar
#endif

#ifndef u16
#define u16 ushort
#endif

#ifndef u32
#define u32 uint
#endif

#ifndef u64
#define u64 ulong
#endif

#endif //__COMMON_H__
