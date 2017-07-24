/*****************************************
 ******************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "common.h"

#define UPLINK_MAX_CNT		64

struct uplink_info {
	char ifname[IFNAMESZ];
	uint type;
	uint subid; //for 4G	
};

struct uplink_dev {
	int uplinks;
	uplink_info uplink[0];
};

struct link_detect_conf {
	uint period;		/* detect period   (default 30s) */
	struct uplink_dev dev;
};

struct link_detect_conf *g_conf;

pthread_t g_l2_thread_handle;
pthread_t g_l3_thread_handle;

unsigned int g_l3_thread_ctr  = 1;

static void sighandler(int sig)
{
	/* exit process */
	exit(1);
}

int globle_init(void)
{
	signal(SIGTERM, sighandler);
	signal(SIGINT, sighandler);
	
    /* init log file */
    if (!log_init("logserver", "/tmp/link_detect.log", 10*1024*1000, LOG_LEVEL_DEBUG, 0)) {
        return -1; 
    }

	/* init globle config */
	
	return 0;
}

int main(int argc, char **argv)
{	
	if (globle_init() < 0) {
		sighandler(SIGTERM);
		return -1;
	}

	if (pthread_create(&g_l2_thread_handle, 
						NULL,
						(void *)l2_thread_handler,
						NULL) {
		log_dbg("Layer 2 thread created failed");
		goto EXIT;
	}

	if(pthread_create(&g_l3_thread_handle, 
						NULL, 
						(void *)l3_thread_handler, 
						NULL) {
		log_dbg("Layer 3 thread created failed");
		goto EXIT;
	}

	pthread_join(g_l2_thread_handle, NULL);
	pthread_join(g_l3_thread_handle, NULL);
	
	return 0;
}
