#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include "thpool.h"

#define LOG_DEBUG_ON 1

#ifdef LOG_DEBUG_ON 
#define LOG_DEBUG(fmt, args...) \
	do {  \
		printf("[DEBUG]:");\
		printf(fmt "\n", ##args); \
	} while(0);
#define LOG_INFO(fmt, args...) \
	do { \
		printf("[INFO]:");\
		printf(fmt "\n", ##args); \
	} while(0);
#define LOG_WARNING(fmt, args...) \
	do { \
		printf("[WARNING]:");\
		printf(fmt "\n", ##args); \
	} while(0);
#else
#define LOG_DEBUG(fmt, args...) 
#define LOG_INFO(fmt, args...) 
#define LOG_WARNING(fmt, args...) 
#endif
#define LOG_ERROR(fmt, args...) \
	do{ \
		printf("[ERROR]:");\
		printf(fmt "\n", ##args);\
	}while(0);

/* binary semaphore (信号量)*/
typedef struct bsem {
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	int v;
}bsem;

/* job */
typedef struct job {
	struct job *next;
	void (*function)(void *arg);
	void *arg;
}job;

/* 工作队列 */
typedef struct jobqueue {
	pthread_mutex_t rwmutex;
	job * front;
	job * rear;
	bsem *has_jobs;
	int job_len;
} jobqueue;

typedef struct thread {
	int id;
	pthread_t pthread;
	struct thpool* thpool_p;
}thread;

/* 线程池 */
typedef struct thpool {
	thread **threads;
	volatile int thread_alive;
	volatile int thread_working;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	jobqueue jobqueue;
}thpool;

static void bsem_init(bsem *bsem_p, int v);
static void bsem_wait(bsem *bsem_p);
static void bsem_reset(bsem * bsem_p);
static void bsem_post(bsem * bsem_p);
static void bsem_post_all(bsem * bsem_p);
static void bsem_reset(bsem * bsem_p);

static volatile int threads_keepalive;
static volatile int threads_on_hold;
static void bsem_init(bsem * bsem_p, int v)
{
	if (v <= 0) {
		v = 0;
	} else {
		v = 1;
	}

	pthread_mutex_init(&(bsem_p->mutex), NULL);
	pthread_cond_init(&(bsem_p->cond), NULL);
	bsem_p->v = v;
}

static void bsem_reset(bsem * bsem_p)
{
	bsem_init(bsem_p, 0);
}

static void bsem_wait(bsem *bsem_p)
{
	pthread_mutex_lock(&bsem_p->mutex);
	while(bsem_p->v != 1) {
		pthread_cond_wait(&bsem_p->cond, &bsem_p->mutex);
	}
	bsem_p->v = 0;
	pthread_mutex_unlock(&bsem_p->mutex);
}

static void bsem_post(bsem *bsem_p)
{
	pthread_mutex_lock(&bsem_p->mutex);
		bsem_p->v = 1;
	pthread_cond_signal(&bsem_p->cond);
	pthread_mutex_unlock(&bsem_p->mutex);
}

static void bsem_post_all(bsem *bsem_p)
{
	pthread_mutex_lock(&bsem_p->mutex);
		bsem_p->v = 1;
	pthread_cond_broadcast(&bsem_p->cond);
	pthread_mutex_unlock(&bsem_p->mutex);
}

static int jobqueue_init(jobqueue *jobqueue_p)
{
	jobqueue_p->front = NULL;
	jobqueue_p->rear = NULL;
	jobqueue_p->job_len = 0;

	jobqueue_p->has_jobs = (bsem *)malloc(sizeof(bsem));
	if (NULL == jobqueue_p->has_jobs) {
		return -1;
	}
	if (pthread_mutex_init(&jobqueue_p->rwmutex, NULL) != 0) {
		printf("jobqueue mutex init failure\n");
		return -1;
	}
	
	bsem_init(jobqueue_p->has_jobs, 0);		
	return 0;
}

/* in jobqueue push */
static void jobqueue_push(jobqueue * jobqueue_p, job *new_job)
{
	int jobs = 0;
	pthread_mutex_lock(&(jobqueue_p->rwmutex));
	jobs = jobqueue_p->job_len;
	switch(jobs){
	case 0:			/* jobqueue is empty */
		jobqueue_p->front = new_job;
		jobqueue_p->rear = new_job;
		break;
	default:
		jobqueue_p->rear->next = new_job;
		jobqueue_p->rear = new_job;	
		break;
	}
	jobqueue_p->job_len ++;
	bsem_post(jobqueue_p->has_jobs);
	pthread_mutex_unlock(&jobqueue_p->rwmutex);
}

/* out jobqueue pull */
static struct job * jobqueue_pull(jobqueue * jobqueue_p)
{
	job *job_p = NULL;
	int jobs = 0;
	
	pthread_mutex_lock(&jobqueue_p->rwmutex);
	job_p = jobqueue_p->front;
	jobs = jobqueue_p->job_len;

	switch(jobs) {
	case 0: 		/* jobqueue is empty(don't need handle)*/
		break;
	case 1:			/* only one job in jobqueue */
		jobqueue_p->front = NULL;
		jobqueue_p->rear = NULL;
		jobqueue_p->job_len  = 0;
		break;
	default: 	 /* jobs > 1 in jobqueue (need wake up multi thread)*/
		jobqueue_p->front = job_p->next;
		jobqueue_p->job_len --;
		bsem_post(jobqueue_p->has_jobs);	
		break;
	}
	pthread_mutex_unlock(&jobqueue_p->rwmutex);
	return job_p;
}

static void jobqueue_clear(jobqueue *jobqueue_p)
{
	while(jobqueue_p->job_len) {
		free(jobqueue_pull(jobqueue_p));
	}
	jobqueue_p->front = NULL;
	jobqueue_p->rear = NULL;
	bsem_reset(jobqueue_p->has_jobs);
	jobqueue_p->job_len = 0;
}

static void jobqueue_destroy(jobqueue *jobqueue_p)
{
	jobqueue_clear(jobqueue_p);
	free(jobqueue_p->has_jobs);
}

int thpool_add_work(thpool *thpool_p,  void (*func)(void *), void *arg)
{
	job *new_job = NULL;
	if(NULL == thpool_p) {
		return -1;
	}

	new_job = (job *)malloc(sizeof(job));
	if (NULL == new_job) {
		printf("create new job failure\n");
		return -1;
	}
	new_job->function = func;
	new_job->arg = arg;
	new_job->next = NULL;
	
	/* add a new jobs */
	jobqueue_push(&(thpool_p->jobqueue), new_job);
}

void * thread_do(struct thread * thread_p)
{
	void (*func)(void*);
	void *arg;
	job *job_p = NULL;

	thpool *thpool_p = thread_p->thpool_p;
	
	pthread_mutex_lock(&thpool_p->mutex);
	thpool_p->thread_alive += 1;
	pthread_mutex_unlock(&thpool_p->mutex);

	while(threads_keepalive) {
		bsem_wait(thpool_p->jobqueue.has_jobs);
		if(threads_keepalive) {
			pthread_mutex_lock(&thpool_p->mutex);
			thpool_p->thread_working ++;
			pthread_mutex_unlock(&thpool_p->mutex);
			
			job_p = jobqueue_pull(&thpool_p->jobqueue);/* get a jobs on jobqueue*/
			if (job_p) {
				func = job_p->function;
				arg = job_p->arg;
				func(arg); /* handle */
				free(job_p);
			}
			pthread_mutex_lock(&thpool_p->mutex);
			thpool_p->thread_working --;
			if (!thpool_p->thread_working) {
				pthread_cond_signal(&thpool_p->cond);
			}
			pthread_mutex_unlock(&thpool_p->mutex);
		}
	}
	pthread_mutex_lock(&thpool_p->mutex);
	thpool_p->thread_alive --;
	pthread_mutex_unlock(&thpool_p->mutex);
	LOG_DEBUG("Thread %2d #%x is Exit", thread_p->id, thread_p->pthread);
	return NULL;
}

static int thread_init(thpool *thpool_p, struct thread ** thread_p, int id)
{
	int ret = 0;
	*thread_p = (struct thread *)malloc(sizeof(struct thread));
	if(NULL == *thread_p) {
		printf("Could not allocate memory for threads\n");
		return -1;
	}
	(*thread_p)->thpool_p = thpool_p;
	(*thread_p)->id = id;
	ret = pthread_create(&((*thread_p)->pthread), NULL, (void *)thread_do, (*thread_p));
	if (ret != 0){
		printf("Create thread failure\n");
		exit(1);
	}
	pthread_detach((*thread_p)->pthread);
	LOG_DEBUG("Thread %2d #%x is running", id, (*thread_p)->pthread);
	return 0;
}

void thread_destroy(struct thread * thread_p)
{
	if (thread_p) {
		free(thread_p);
		thread_p = NULL;
	}
}

void thpool_destroy(thpool *thpool_p)
{
	if (NULL == thpool_destroy) {
		return;
	}
	
	volatile int thr_nums = thpool_p->thread_alive;
	time_t start, end;
	double TIMEOUT = 2.0;
	double rv = 0.0;
	int  i = 0;

	threads_keepalive = 0;	
	/* idle threads of exits */
	time(&start);
	while(rv < TIMEOUT && thpool_p->thread_alive) {
		bsem_post_all(thpool_p->jobqueue.has_jobs);
		time(&end);
		rv = difftime(end, start);
	}

	/* Ensure threads exits */
	while(thpool_p->thread_alive) {
		bsem_post_all(thpool_p->jobqueue.has_jobs);
		sleep(1);
	}

	/* Job Queue clear */
	jobqueue_destroy(&thpool_p->jobqueue);

	/* release resource of threads*/
	for(i = 0; i < thr_nums; i++) {
		thread_destroy(thpool_p->threads[i]);
	}
	free(thpool_p->threads);
	free(thpool_p);
}

struct thpool * thpool_init(int th_nums)
{
	int i = 0;
	threads_on_hold = 0;
	threads_keepalive = 1;
	thpool *thpool_p = NULL;
	 
	if (th_nums < 0) {
		th_nums = 0;
	}

	thpool_p = (thpool *)malloc(sizeof (struct thpool));
	if (NULL == thpool_p) {
		printf("create thread pool failure\n");
		return NULL;
	}
	thpool_p->thread_alive = 0;
	thpool_p->thread_working = 0;

	/*init jobqueue*/
	if (jobqueue_init(&thpool_p->jobqueue)) {
		printf("jobqueue init failure\n");
		free(thpool_p);
		return NULL;
	}

	thpool_p->threads = (struct thread **)malloc(sizeof(struct thread *) * th_nums);
	if(NULL == thpool_p->threads) {
		printf("create thpool threads failure\n");
		/* jobqueue destroy */
		jobqueue_destroy(&thpool_p->jobqueue);
		free(thpool_p);
		return NULL;
	}

	pthread_mutex_init(&thpool_p->mutex, NULL);
	pthread_cond_init(&thpool_p->cond, NULL);

	for(i = 0; i < th_nums; i++) {
		thread_init(thpool_p, &(thpool_p->threads[i]), i);
	}
	while (thpool_p->thread_alive != th_nums){}
	
	return thpool_p;
}

void task1(){
    LOG_DEBUG("Thread #%x working on task1", (unsigned int)pthread_self());
}

void task2(){
    LOG_DEBUG("Thread #%x working on task2", (unsigned int)pthread_self());
}

void task3(){
    LOG_DEBUG("Thread #%x working on task3", (unsigned int)pthread_self());
}

int main()
{
    
    int i;
    LOG_DEBUG("Making threadpool with 20 threads");
    thpool * thpool_p  = NULL;
	thpool_p = thpool_init(4);
	if(thpool_p == NULL) {
		return -1;
	}
    
	for (i=0; i<20; i++){
        thpool_add_work(thpool_p, task1, NULL);
        thpool_add_work(thpool_p, task2, NULL);
        thpool_add_work(thpool_p, task3, NULL);
    }; 
 
    sleep(5);
    LOG_DEBUG("Killing threadpool");
    thpool_destroy(thpool_p);
    return 0;
}
