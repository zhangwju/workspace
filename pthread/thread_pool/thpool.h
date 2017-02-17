#ifndef __THPOOL_H__
#define __THPOOL_H__

typedef struct thpool thpool;

struct thpool * thpool_init(int th_nums);

void thpool_destroy(thpool *thpool_p);

int thpool_add_work(thpool *thpool_p,  void (*func)(void *), void *arg);

#endif //__THPOOL_H__
