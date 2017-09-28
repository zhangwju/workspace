#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include<sys/types.h> 
#include "timer.h"

void timer_handler(void *arg)
{
	printf("%s\n", (char *)arg);
} 

void *thread_func(void *arg)
{
	while(1);
}

int main()
{
	pthread_t pid;
	int ret  = 0;

	sigset_t sigset;
    	sigemptyset(&sigset);
    	sigaddset(&sigset, SIGALRM);
    	pthread_sigmask(SIG_BLOCK, &sigset, NULL);

	if (timer_init() == 0) {
		printf("timer init failure\n");
		return -1;
	}
#if 0
	ret = pthread_create(&pid, NULL, thread_func, NULL);
	if(ret < 0) {
		perror("pthread create failure");
		return -1;
	}
#endif

	ret = timer_add(3, 0, &timer_handler, "this is timer 3");
	printf("ret : %d\n", ret);
	ret = timer_add(2, 0, &timer_handler, "this is timer 2");
	ret = timer_add(1, 0, &timer_handler, "this is timer 1");

	while(1);
	//	pthread_join(pid, NULL);

	return 0;
}
