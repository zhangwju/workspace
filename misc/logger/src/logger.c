/**************************************
 * Filename: logger.c
 * Author: zhangwj
 * description: Simple logging Library
 * Date: 2017-05-12
 *************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <errno.h>
#include <pthread.h>
#include "logger.h"

typedef struct __logger_t {
	char *logfile;
	FILE *lfp;
	int enable;
	int level;
	unsigned int size;
	pthread_mutex_t mutex;	
} logger_t;

static logger_t * g_lptr = NULL;

void logger_set_enable()
{
	g_lptr->enable = 1;
}

void logger_set_disable()
{
	g_lptr->enable = 0;
}

void logger_set_level(int level)
{
	g_lptr->level = level; 
}

static char * log_get_levelstr(int level)
{
	char *levelstr = NULL;
	switch(level) {
	case LOG_DEBUG:
		levelstr = "LOG_DEBUG";
		break;
	case LOG_INFO:
		levelstr = "LOG_INFO";
		break;
	case LOG_WARN:
		levelstr = "LOG_WARN";
		break;
	case LOG_ERROR:
		levelstr = "LOG_ERROR";
		break;
	default:
		levelstr= " ";
		break;
	}

	return levelstr;
}

int logger_init(char *logfile, int filesize, int level)
{	
	FILE *lfp = NULL;

	if (NULL == logfile || strlen(logfile) >= MAX_LOG_FILE_NAME_lEN) {
		return -1;
	}

	g_lptr = (logger_t *)malloc(sizeof(logger_t));
	if (NULL == g_lptr) {
		printf("logger malloc memroy failure\n");
		return -1;
	}

	lfp = fopen(logfile, "w");
	if (NULL == lfp) {
		printf("fopen %s, Error[%d:%s]\n", logfile, errno, strerror(errno));
		return -1;
	}
	g_lptr->lfp = lfp;
	g_lptr->level = level;
	g_lptr->logfile = logfile;
	g_lptr->enable = 0;
	if (filesize)
		g_lptr->size = filesize;
	else
		g_lptr->size = LOG_DEFAULT_FILESIZE;	
	pthread_mutex_init(&g_lptr->mutex, NULL);
	return 0;
}

void logger_deinit()
{
	if (g_lptr) {
		pthread_mutex_destroy(&g_lptr->mutex);
		if (g_lptr->lfp) {
			fclose(g_lptr->lfp);
		}
		free(g_lptr);
	}
}

static int log_check_reset()
{
	long fileSize = 0;
	
	if ((fileSize = ftell(g_lptr->lfp)) == -1) {
		printf("An error with log file occurred: %s\n", strerror(errno));
		return 0;
	}   

	if (fileSize >= g_lptr->size) {
	
		fclose(g_lptr->lfp);
		if((g_lptr->lfp = fopen(g_lptr->logfile, "w")) == NULL) {
			printf("Can't open log file: %s", strerror(errno));
			return 0;
		}   
	}

	return 1;
}

static void log_vdebug(const char *format, const char *func, unsigned int line_id, int level, va_list args)
{
	time_t secs;
	struct tm now;

	if (!g_lptr->enable) {
		return;
	}

	if (level <  g_lptr->level) {
		return;
	}

	if (NULL == format) {
		return;
	}

	if (g_lptr->lfp) {
		pthread_mutex_lock(&g_lptr->mutex);
		if (!log_check_reset()) {
			pthread_mutex_unlock(&g_lptr->mutex);
			exit(1);
		}

		time(&secs);
		localtime_r(&secs, &now);
		now.tm_mon  += 1;
		now.tm_year += 1900;

		fprintf(g_lptr->lfp, "%04d-%02d-%02d %02d:%02d:%02d ", 
			now.tm_year, now.tm_mon, now.tm_mday, now.tm_hour, now.tm_min, now.tm_sec);

		fprintf(g_lptr->lfp, "%-9s ", log_get_levelstr(level));

		fprintf(g_lptr->lfp, "%s:%d ", func, line_id);

		vfprintf(g_lptr->lfp, format, args);

		fprintf(g_lptr->lfp, "\n");

		fflush(g_lptr->lfp);
		
		pthread_mutex_unlock(&g_lptr->mutex);
	}

	return;
}

void logger_print(const char *format, const char *func, unsigned int line, int level, ...)
{
	va_list args;

	va_start(args, level);
	log_vdebug(format, func, line, level, args);
	va_end(args);
}

