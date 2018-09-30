#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include "list_queue.h"

#define jobqueue_empty(queue)	(queue->head == NULL)

int jobqueue_init(jobqueue_t *queue)
{
	if (NULL == queue) {
		return -1;
	}

	queue->queue_len = 0;
	queue->head = NULL;
	queue->tail = NULL;
	pthread_mutex_init(&queue->mutex, NULL);
	pthread_cond_init(&queue->cond, NULL);

	return 0;
}

int jobqueue_insert(jobqueue_t *queue, char *data, int datalen)
{
	job_t *job = NULL;

	if (NULL == queue || NULL == data || datalen <= 0) {
		return -1;
	}

	job = (job_t *)malloc(sizeof(job_t) + datalen);
	if (NULL == job) {
		return -1;
	}

	job->next = NULL;
	job->datalen = datalen;
	memcpy(job->data, data, datalen);

	if (NULL == queue->head) {
		queue->head = job;
		queue->tail = job;
	} else {
		queue->tail->next = job;
		queue->tail = job;
	}

	return 0;
}

job_t * jobqueue_get(jobqueue_t *queue)
{
	job_t *job = NULL;

	if (queue) {
		job = queue->head;
		queue->head = NULL;
		queue->tail = NULL;
	}

	return job;
}

int jobs_release(job_t *job)
{
	job_t *tmp_job = NULL;

	while (job) {
		tmp_job = job->next;
		free(job);
		job = tmp_job;
	}

	return 0;
}

int job_release(job_t *job)
{
	if (job) {
		free(job);
		job = NULL;
	}

	return 0;
}

int jobqueue_deinit(jobqueue_t *queue)
{
	if (queue) {
		job_release(queue->head);
		pthread_mutex_destroy(&queue->mutex);
		pthread_cond_destroy(&queue->cond);
	}

	return 0;
}
