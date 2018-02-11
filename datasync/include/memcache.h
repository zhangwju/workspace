#ifndef __MEMCACHE_H__
#define __MEMCACHE_H__
#include <unistd.h>
#include <pthread.h>
#include "common.h"
#include "array-hlist.h"

#define	MEMBLK_CNT		(1 << 12)

struct dsync_blk {
	arr_hlist_node_t node;
	int priority;			
	int datalen;
	uchar data[4096];
};

typedef struct _memcache_cbk_s {
	pthread_mutex_t mutex;
	struct dsync_blk entry[MEMBLK_CNT+1];
	arr_hlist_node_t free;
	uint free_num;
	uint blkcnt;
} mcache_cbk_t;

typedef int (* func_mcache_putcall)(ulong arg, uchar *ptrbuf, ulong exarg);
typedef int (* func_mcache_getcall)(ulong arg, uchar *prtbuf, ulong exarg);

extern int init_mcacheblk(mcache_cbk_t **pmccbk);
extern int get_membuf(mcache_cbk_t *pmccbk, func_mcache_getcall func, ulong arg);
extern int put_membuf(mcache_cbk_t *pmccnk, func_mcache_putcall func, ulong arg);
#endif //__MEMCACHE_H__
