#ifndef __RING_QUEUE_H__
#define __RING_QUEUE_H__

#include "common.h"

#define RQUEUE_BUFSIZE		(5120) //5k

typedef struct rq_node {
	int priority;
	uint dlen;
	uchar data[RQUEUE_BUFSIZE];
}rq_node, *prq_node;

typedef struct ring_queue {
	int front;
	int rear;
	int rqsize;
	pthread_mutex_t mutex;
	rq_node node[0];
}rqueue_t, *prqueue_t;

/* ring queue oops */
extern int rqueue_init(rqueue_t **rq, int rqsize);
extern int rqueue_push(rqueue_t *rq, uchar *data, uint len);
extern int rqueue_poll(rqueue_t *rq, uchar *data, uint len);
extern int rqueue_clear(rqueue_t *rq);
extern int rqueue_release(rqueue_t *rq);

#endif //__RING_QUEUE_H__
