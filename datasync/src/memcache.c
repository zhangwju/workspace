#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "memcache.h"

#if 0
int init_mcacheblk(memcache_cbk_t **pmccbk)
{
	uint i, idx; 
	arr_hlist_head_t *head;	
	memcache_cbk_t *mccbk;

	if (NULL == pmccbk) {
		return -1;
	}

	mccbk = (memcache_cbk_t *)malloc(sizeof(memcache_cbk_t));
	if (NULL == *pmmccbk) {
		log_dbg("init memcache failure, error:[%d:%s]", errno, strerror(errno));
		return -1;
	}

	mesmet(mccbk, 0, sizeof(memcache_cbk_t));
	mccbk->blkcnt = MEMBLK_CNT + 1;
	pthread_mutex_init(&(mccbk->mutex), NULL);
	
	head = &mccbk->free;
	ARR_INIT_HLIST_HEAD(head);
	for (i = 1; i < mccbk->blkcnt; i++) {
		arr_hlist_add_head(head, mccbk->entry, i, node);
	}
	mccbk->free_num = MEMBLK_CNT;
	*pmccbk = mccbk;

	return 0;
}

int proc_getbuf(mcache_cbk_t *pmccbk, func_mcache_getcall func, ulong arg)
{
	
}

int proc_putbuf(mcache_cbk_t *pmccbk, func_mcache_putcall func, ulong arg) 
{
	uint i, idx;
	arr_hlist_head_t *head;
	
	head = &pmccbk->free;
	if (!arr_hlist_empty(head)) {
			
	}
}

int get_membuf(mcache_cbk_t *pmccbk, func_mcache_getcall func, ulong arg)
{
	int ret;
	dsync_lock(&(pmccbk->mutex));
	ret = proc_getbuf(pmccbk, func, arg);
	dsync_unlock(&(pmccbk->mutex));

	return ret;
}

int put_membuf(mcache_cbk_t *pmccnk, func_mcache_putcall func, ulong arg)
{
	int ret;
	dsync_lock(&(pmccbk->mutex));
	ret = proc_putbuf(pmccbk, func, arg);
	dsync_unlock(&(pmccbk->mutex));

	return ret;
}

#endif
