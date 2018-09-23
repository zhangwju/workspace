#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include "list_queue.h"

jobqueue_t *g_queue = NULL;

void job_handle(job_t *job)
{
    job_t *tmp_job = NULL;
    tmp_job = job;
    while (tmp_job) {
        printf("msg: %s:%d\n", tmp_job->data, tmp_job->datalen);
        tmp_job = job->next;
    }
}

void *work_thread(void *arg) 
{
    char buf[1024] = {0};
    job_t *job = NULL;

    while(1) 
    {
        pthread_mutex_lock(&g_queue->mutex);
        while (NULL == g_queue->job) {
             pthread_cond_wait(&g_queue->cond, &g_queue->mutex);
        }
        job = queue_get(g_queue->job);  
        pthread_mutex_unlock(&g_queue->mutex);
        /* do somethind */
        job_handle(job);
        job_release(job);
    }            
}

void *task_thread(void *arg)
{
    char data[1024];
    int count = 0;

    while (1) {
        pthread_mutex_lock(&g_queue->mutex);
        sprintf(data, "test msg %d\n", count++);
        if (queue_insert(g_queue->job, data, strlen(data))) {
            log_dbg("queue insert failure\n");
        }
        pthread_cond_signal(&g_queue->cond);
        pthread_mutex_unlock(&g_queue->mutex);
    } 
}

int main(int argc, char **argv)
{
    int ret = 0;
    pthread_t work_pid;
    pthread_t task_pid;

    if (queue_init(&g_queue) < 0) {
        log_dbg("queue_init failure\n");
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
    
    queue_release(g_queue);
    return 0;
}
