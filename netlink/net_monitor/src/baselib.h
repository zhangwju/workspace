#ifndef __BASELIB_H__
#define __BASELIB_H__
#include "logger.h"

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
#define IP2STR(a) (a)&0xff, ((a)>>8)&0xff, ((a)>>16)&0xff, ((a)>>24)&0xff

#define NET_MONITOR_LOG

#ifdef NET_MONITOR_LOG
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

#endif //__BASELIB_H__
