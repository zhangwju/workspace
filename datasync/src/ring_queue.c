/***************************************
 * Filename: ring_queue.c
 * Author: zhangwju
 * Date: 2018-02-01
 * Email: zhangwju@gmail.com
 ***************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>

#include "common.h"
#include "ring_queue.h"

#define rqueue_full(rq)		(rq->front == ((rq->rear + 1) % rq->rqsize))
#define rqueue_empty(rq)	(rq->front == rq->rear)


/*front del, rear: add */
int rqueue_init(rqueue_t **rq, int rqsize)
{
	int memsize;
	prqueue_t q;

	if (NULL == rq || rqsize <= 0) {
		return -1;
	}
	
	memsize = (sizeof(rqueue_t) + (sizeof(rq_node) * rqsize));
	q = (prqueue_t)malloc(memsize);
	if (NULL == q) {
		log_dbg("ring_queue allocate, Error:[%d:%s]", errno, strerror(errno));
		return -1;
	}

	memset(q, 0, memsize);
	pthread_mutex_init(&q->mutex, NULL);
	q->rqsize = rqsize;
	q->front = 0;
	q->rear = 0;
	*rq = q;

	return q->rqsize;	
}

int rqueue_push(rqueue_t *rq, uchar *data, uint len)
{
	pthread_mutex_lock(&rq->mutex);
	if (rqueue_full(rq)) {
		pthread_mutex_unlock(&rq->mutex);
		return -1;
	}

	memcpy(&rq->node[rq->rear].data, data, len);
	rq->node[rq->rear].dlen = len;
	rq->rear = (rq->rear + 1) % rq->rqsize;
	pthread_mutex_unlock(&rq->mutex);
	
	return 0;
}

int rqueue_poll(rqueue_t *rq, uchar *data, uint len)
{
	uint size;
	pthread_mutex_lock(&rq->mutex);
	if (rqueue_empty(rq)) {
		pthread_mutex_unlock(&rq->mutex);
		return -1;
	}
	size = rq->node[rq->front].dlen;
	if (len < size) {
		size = len;
	}
	memcpy(data, &rq->node[rq->front].data, size);
	rq->front = (rq->front + 1) % rq->rqsize;
	pthread_mutex_unlock(&rq->mutex);

	return size;
}

int rqueue_clear(rqueue_t *rq)
{
	pthread_mutex_lock(&rq->mutex);
	rq->front = 0;
	rq->rear = 0;
	pthread_mutex_unlock(&rq->mutex);

	return 0;
}

int rqueue_release(rqueue_t *rq)
{
	if (rq) {
		pthread_mutex_destroy(&rq->mutex);
		free(rq);
	}
	
	return 0;
}
