#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include "list_queue.h"

#define log_dbg(format, ...) fprintf(stderr, "[%s():%d] "format"\n", __FUNCTION__, __LINE__, ## __VA_ARGS__)

/* custom datastruct */
struct global_data {
	jobqueue_t queue;
};

struct global_data *g_info = NULL;

/* custom do someting */
void job_handle(job_t *job_head)
{
	job_t *job = NULL;

	job = job_head;
	while (job)
	{
		printf("Thread2: %d:%s", job->datalen, job->data);
		job_head = job->next;
		free(job);
		job = job_head;
	}
}

void *work_thread(void *arg) 
{
	jobqueue_t *queue;
	job_t *job_head = NULL;

	log_dbg("enter work thread....\n");
	queue = &g_info->queue;
	while (1)
	{
		pthread_mutex_lock(&queue->mutex);
		while (NULL == queue->head) {
			 pthread_cond_wait(&queue->cond, &queue->mutex);
		}

		/* get jobs */
		job_head = jobqueue_get(queue);
		pthread_mutex_unlock(&queue->mutex);

		if (job_head) {
			/* do something */
			job_handle(job_head);
		}
	}
}

void *task_thread(void *arg)
{
	int count = 0;
	char data[1024];
	jobqueue_t *queue;

	log_dbg("enter task_thread...\n");
	queue = &g_info->queue;
	while (1)
	{
		pthread_mutex_lock(&queue->mutex);

		sprintf(data, "test msg %d\n", count++);
		printf("Thread1: %d:%s", (int)strlen(data), data);
		/* insert queue */
		if (jobqueue_insert(queue, data, strlen(data)) == 0) {
			pthread_cond_signal(&queue->cond);
		}
		pthread_mutex_unlock(&queue->mutex);

	//	usleep(500);
		sleep(2);
	}
}

int global_init(void)
{
	g_info = (struct global_data *)malloc(sizeof(struct global_data));
	if (NULL == g_info)	{
		log_dbg("global data init failure\n");
		return -1;
	}

	/* init queue */
	jobqueue_init(&g_info->queue);

	return 0;
}

int global_deinit(void)
{
	if (g_info) {
		jobqueue_deinit(&g_info->queue);
		free(g_info);
		g_info = NULL;
	}

	return 0;
}

int main(int argc, char **argv)
{
	int ret = 0;
	pthread_t work_pid;
	pthread_t task_pid;

	if (global_init() < 0) {
		return -1;
	}

	ret = pthread_create(&work_pid, NULL, work_thread, NULL);
	if (ret < 0) {
		log_dbg("work thread create failure, %s:%d\n", strerror(errno), errno);
		return -1;
	}

	ret = pthread_create(&task_pid, NULL, task_thread, NULL);
	if (ret < 0) {
		log_dbg("task thread create failure, %s:%d\n", strerror(errno), errno);
		return -1;
	}

	pthread_join(work_pid, NULL);
	pthread_join(task_pid, NULL);

	return 0;
}
