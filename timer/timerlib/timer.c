#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <sys/time.h>
#include <errno.h>
#include "timer.h"

#define MAX_USEC	999999

#define TV_LESS_THAN(t1,t2) (((t1).tv_sec < (t2).tv_sec) ||\
                             (((t1).tv_sec == (t2).tv_sec) &&\
                              ((t1).tv_usec < (t2).tv_usec)))

/*
 * Assign struct timeval tgt the time interval between the absolute
 * times t1 and t2.
 * IT IS ASSUMED THAT t1 > t2, use TV_LESS_THAN macro to test it.
 */
#define TV_MINUS(t1,t2,tgt) if ((t1).tv_usec >= (t2).tv_usec) {\
        (tgt).tv_sec = (t1).tv_sec -\
                       (t2).tv_sec;\
        (tgt).tv_usec = (t1).tv_usec -\
                        (t2).tv_usec;\
    }\
    else {\
        (tgt).tv_sec = (t1).tv_sec -\
                       (t2).tv_sec -1;\
        (tgt).tv_usec = (t1).tv_usec +\
                        (MAX_USEC - (t2).tv_usec);\
    }

struct timer {
	struct timeval timeout;
	void 	(*handler)(void *);
	void 	*handler_arg;
	int 	id;
	int 	in_use;
	int 	cancelled;
	struct timer *next;
	struct timer *prev;
};

static struct {
	struct timer 	*first;
	struct timer	*last;
	pthread_mutex_t	mutex;
	pthread_cond_t	cond;
	pthread_t		tid;
	int				cur_id;
}timerq;

static void timer_free(struct timer *t);
static void timer_dequeue(struct timer *t);
static void timer_start(struct timeval *tv);
static void *cronometer(void *arg);

static void timer_free(struct timer *t)
{
	if(NULL == t) {
		return;
	}
	free(t);
	t = NULL;
}

static void timer_dequeue(struct timer *t)
{

    if(t == NULL) { 
		return; 
	}

    if(t->prev == NULL){
        timerq.first = t->next;
    }else {
        t->prev->next = t->next;
    }

    if (t->next == NULL){
        timerq.last = t->prev;
    }else {
        t->next->prev = t->prev;
    }

    timer_free(t);
    return;

}

static void timer_start(struct timeval *abs_tv)
{
	struct itimerval relative = {{0,0}, {0,0}};
	struct timeval abs_now;
	int ret = 0;

	if(NULL == abs_tv) {
		return;
	}

	gettimeofday(&abs_now, NULL);

	if(TV_LESS_THAN(abs_now, *abs_tv)){
	/*timeout in the future*/
		TV_MINUS(*abs_tv, abs_now, relative.it_value);
	}else {
		relative.it_value.tv_sec = 0;
        relative.it_value.tv_usec = 1000;
	}

	setitimer(ITIMER_REAL, &relative, NULL);
	
	return;
}

static void *cronometer(void *arg)
{
	void (*hdl)(void *);
	void *hdl_arg = arg;
	
	sigset_t mask;
	int sig;

	sigemptyset(&mask);
	sigaddset(&mask, SIGALRM);

	/* 设置线程取消点 */
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

	pthread_mutex_lock(&timerq.mutex);
	pthread_cond_signal(&timerq.cond);
	pthread_mutex_unlock(&timerq.mutex);

	while(1) {
		pthread_mutex_lock(&timerq.mutex);
		while(timerq.first == NULL) {
			pthread_cond_wait(&timerq.cond, &timerq.mutex);
		}

		timer_start(&(timerq.first->timeout));
		timerq.first->in_use = 1;
		
		pthread_mutex_unlock(&timerq.mutex);

		/* wait signal int sigwait(const sigset_t *set, int *sig);*/
		sigwait(&mask, &sig);

		if(timerq.first->cancelled == 1) {
			timer_dequeue(timerq.first);
            pthread_mutex_unlock(&timerq.mutex);
            continue;
		}

		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL); //DISABLE PTHREAD_CANCEL
		hdl = timerq.first->handler;
		hdl_arg = timerq.first->handler_arg;
		timer_dequeue(timerq.first);
		pthread_mutex_unlock(&timerq.mutex);
		hdl(hdl_arg);
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	}
	return NULL;
}

int timer_init()
{
	int ret;
	struct timespec ts;
	struct timeval tv;

	ret = pthread_mutex_init(&timerq.mutex, NULL);
	if(ret != 0) {
		return 0;
	}

	ret = pthread_cond_init(&timerq.cond, NULL);
	if(ret != 0) {
		pthread_mutex_destroy(&timerq.mutex);
		return 0;
	}
	timerq.first = NULL;
	timerq.last = NULL;
	timerq.cur_id = 0;

	pthread_mutex_lock(&timerq.mutex);
	ret = pthread_create(&timerq.tid, NULL, cronometer, NULL);
	if(ret != 0) {
		pthread_mutex_unlock(&timerq.mutex);
		pthread_mutex_destroy(&timerq.mutex);
		pthread_cond_destroy(&timerq.cond);
		return 0;
	}
	
    gettimeofday(&tv, NULL);
    ts.tv_sec = tv.tv_sec + 5;
    ts.tv_nsec = tv.tv_usec * 1000;

	ret = pthread_cond_timedwait(&timerq.cond, &timerq.mutex, &ts);
	if (ret != 0) {
		if (ret == ETIMEDOUT) {
			pthread_cancel(timerq.tid);
			pthread_join(timerq.tid, NULL);
			return 0;
		}
		pthread_mutex_destroy(&timerq.mutex);
        pthread_cond_destroy(&timerq.cond);
		return 0;
	}
	pthread_detach(timerq.tid);
	pthread_mutex_unlock(&timerq.mutex);

	return 1;
}

int timer_add(long sec, long usec, void(*hndlr)(void *), void *hndlr_arg)
{
	struct timeval new;
	struct timer *tmp;
	struct timer *app;
	int id = 0;

	if (NULL == hndlr) {
		return -1;
	}

	if ((sec < 0) || (usec < 0) || ((sec == 0) && usec == 0)) {
		return -1;
	}
	
	pthread_mutex_lock(&timerq.mutex);
	app = (struct timer *)malloc(sizeof(struct timer));
	if (NULL == app) {
		perror("malloc falure\n");
		pthread_mutex_unlock(&timerq.mutex);
		return -1;
	}

	/* relative to absolute time for timer */
    gettimeofday(&new, NULL);

    /* add 10^6 microsecond units to seconds */
    new.tv_sec += sec + (new.tv_usec + usec) / 1000000;
    /* keep microseconds inside allowed range */
    new.tv_usec = (new.tv_usec + usec) % 1000000;

	memcpy(&app->timeout, &new, sizeof(struct timeval));
    app->handler = hndlr;
    app->handler_arg = hndlr_arg;

    id = timerq.cur_id++;
    app->id = id;
    app->in_use = 0;
    app->cancelled = 0;

	if (timerq.first == NULL) {
        /* timer queue empty */
        timerq.first = app;
        timerq.last = app;
        app->prev = NULL;
        app->next = NULL;
        pthread_cond_signal(&timerq.cond);
        pthread_mutex_unlock(&timerq.mutex);
        return id;
    }

    /* there is at least a timer in the queue */
    tmp = timerq.first;

    /* find the first timer that expires before app */
    while (tmp != NULL) {

        if (TV_LESS_THAN(app->timeout, tmp->timeout)) {
            break;
        }

        tmp = tmp->next;
    }

    if (tmp == NULL) {
        /* app is the longest timer */
        app->prev = timerq.last;
        app->next = NULL;
        timerq.last->next = app;
        timerq.last = app;

        pthread_mutex_unlock(&timerq.mutex);
        return id;
    }

    if (tmp->prev == NULL) {
        /* app is the shoprtest timer */
        app->prev = NULL;
        app->next = tmp;
        tmp->prev = app;
        timerq.first = app;

        /* start app timer */
        app->in_use = 1;
        tmp->in_use = 0;
        timer_start(&(timerq.first->timeout));
    }
    else {
        app->prev = tmp->prev;
        app->next = tmp;
        tmp->prev->next = app;
        tmp->prev = app;
    }

    pthread_mutex_unlock(&timerq.mutex);
    return id;
}

void timer_destroy()
{

    struct timer 	*t;

    pthread_cancel(timerq.tid);
    pthread_join(timerq.tid, NULL);

    t = timerq.last;

    while (t != NULL) {
		
        timerq.last = t->prev;
        timer_free(t);
        t = timerq.last;
    }

    pthread_mutex_destroy(&timerq.mutex);
    pthread_cond_destroy(&timerq.cond);

    return;
}

