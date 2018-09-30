#ifndef __QUEUE_LIST_H__
#define __QUEUE_LIST_H__

typedef struct job {
	struct job *next;
	int datalen;
	char data[0]; /* struct data */
}job_t;

typedef struct jobqueue {
	int queue_len;
	struct job *head;
	struct job *tail;
	pthread_mutex_t	mutex;
	pthread_cond_t	cond;
} jobqueue_t;

int jobqueue_init(jobqueue_t *queue);
int jobqueue_insert(jobqueue_t *queue, char *data, int datalen);
job_t * jobqueue_get(jobqueue_t *queue);
int job_release(job_t *job);
int jobqueue_release(jobqueue_t *queue);

#endif /* __QUEUE_LIST_H__ */
