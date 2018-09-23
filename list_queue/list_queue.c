#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include "list_queue.h"

int queue_init(jobqueue_t **queue)
{
    jobqueue_t *p_queue = NULL;

    if (NULL == queue) {
        return -1;
    }
    
    p_queue = (jobqueue_t *)malloc(sizeof(jobqueue_t));
    if (NULL == p_queue) {
        log_dbg("malloc, %s:%d\n", strerror(errno), errno); 
        return -1;       
    }

    memset(p_queue, 0, sizeof(jobqueue_t));
    pthread_mutex_init(&p_queue->mutex, NULL);
    pthread_cond_init(&p_queue->cond, NULL);

    *queue = p_queue;

    return 0;
}

int queue_insert(job_t *job, char *data, int datalen)
{
    job_t *tmp_job = NULL;
    if (NULL == data) {
        return -1;
    }
    
    tmp_job = (job_t *)malloc(sizeof(job_t) + datalen);
    if (NULL == tmp_job) {
        return -1;
    }

    job->datalen = datalen;
    memcpy(job->data, data, datalen);
    tmp_job->next = NULL;

    if (job == NULL) {
        job = tmp_job;
    } else {
        tmp_job->next = job;
        job = tmp_job;
    }
    return 0;
}

job_t * queue_get(job_t *job)
{
    job_t * tmp_job = NULL;
    if (job) {
        tmp_job = job;
        job = NULL;
    }

    return tmp_job;
}

int job_release(job_t *job)
{
    job_t *tmp_job = NULL;
    
    while (job) {
        tmp_job = job->next;
        free(job);
        job = tmp_job;
    }   
    job = NULL;
    return 0;
}

int queue_release(jobqueue_t *queue)
{
    if (queue) {
        job_release(queue->job);
        free(queue);
        queue = NULL;
    }
    return 0;
}

