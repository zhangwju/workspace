#ifndef __QUEUE_LIST_H__
#define __QUEUE_LIST_H__

#define log_dbg(format, ...) fprintf(stderr, "[%s():%d] "format"\n", __FUNCTION__, __LINE__, ## __VA_ARGS__)

typedef struct job {
    struct job *next;
    int datalen;
    char data[0];
}job_t;

typedef struct jobqueue {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    struct job *job;
} jobqueue_t;

job_t *queue_get(job_t *job);
int queue_insert(job_t *job, char *data, int datalen);
int job_release(job_t *job);
int queue_release(jobqueue_t *queue);
#endif /* __QUEUE_LIST_H__ */
