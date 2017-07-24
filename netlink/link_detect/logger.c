/*
 *  logger.c
 *
 *  Copyright (c) 2014 Daniel Rocha. All rights reserved.
 *  Modify:
 * 	        1. Teidor Tien, 2015/12/29, in my format
 */

/* -------------------------------------------------------------------------- *
 *                               INCLUDED FILES                               *
 * -------------------------------------------------------------------------- */

#include <stdio.h>    /* fprintf() */
#include <stdlib.h>   /* exit()    */
#include <string.h>   /* strlen()  */
#include <errno.h>    /* errno()   */
#include <time.h>     /* time()    */
#include <stdarg.h>   /* va_list   */
#include <pthread.h>  /* mutex     */
#include "logger.h"

/* -------------------------------------------------------------------------- *
 *                              GLOBAL VARIABLES                              *
 * -------------------------------------------------------------------------- */

#define MAX_LOG_FILE_NAME_LEN 				128
#define DEFAULT_LOG_FILE_SIZE 				512*1000 //512Kb

#define NO_DIR_FILENAME(x) 						strrchr(x,'/')?strrchr(x,'/')+1:x

FILE *logs[3];

static struct {
    int format;
    int level;
	int max_fz;  // MAX filesize, bytes
    const char *ident;
    pthread_mutex_t lock;
    FILE **streams;
} logger = {
    LOG_PRINT_DATE | LOG_PRINT_TIME | LOG_PRINT_TAG | LOG_PRINT_FILE,
    LOG_LEVEL_INFO,
	DEFAULT_LOG_FILE_SIZE,
	NULL,
    PTHREAD_MUTEX_INITIALIZER,
    NULL
};

int log_enable = 0;
char log_file_name[MAX_LOG_FILE_NAME_LEN] = {0};

/* -------------------------------------------------------------------------- *
 *                                 PROTOTYPES                                 *
 * -------------------------------------------------------------------------- */

static const char *log_tag(int level);

static void log_set_fz(int filesize);

static void log_set_ident(const char *ident);

static void log_set_streams(FILE **streams);

static FILE **log_get_streams(void);

static void log_set_format(int format);

static int log_get_format(void);

static void log_set_level(int level);

static int log_get_level(void);

/* -------------------------------------------------------------------------- *
 *                                 FUNCTIONS                                  *
 * -------------------------------------------------------------------------- */

/**
 * Sets the log file where the logs are written. A NULL terminated array
 * is passed as parameter containing a list of FILE pointers
 *
 * @param ident - program identifier
 * @param file - file to record the logs
 * @param filesize - file maximum size
 * @param level - Severity level of the call
 * @param scn_op - output the log  to screen or not<1: yes, 0: no>
 */
int log_init(const char *ident, const char *log_file, int filesize, E_LOG_LEVEL level, int scn_op)
{
    if (log_file == NULL || ident == NULL)
        return 0;
	if (strlen(log_file) >= MAX_LOG_FILE_NAME_LEN)
		return 0;
    if (level < LOG_LEVEL_ALL || level > LOG_LEVEL_OFF)
        return 0;
	if (filesize <= 0)
		return 0;	

	// Set streams 
	strcpy(log_file_name, log_file);
    logs[0] = fopen(log_file_name, "w");
    if (logs[0] == NULL)
        return 0;
	if (scn_op == 1)
    	logs[1] = stderr;
	else 
		logs[1] = NULL;
    logs[2] = NULL;

    log_set_streams(logs);
    log_set_ident(ident);
	log_set_fz(filesize);
    log_set_level(level);
    return 1;
}

/**
 * Sets the output stream identifier where the logs are written.
 *
 * @param ident - Program identifier
 */
static void log_set_ident(const char *ident)
{
    logger.ident = ident;
}

/**
 * Sets the output log file size where the logs are written.
 *
 * @param filesize - Filesize
 */
static void log_set_fz(int filesize)
{
	logger.max_fz = filesize;
}

/**
 * Sets the output streams where the logs are written. A NULL terminated array
 * is passed as parameter containing a list of FILE pointers.
 *
 * @param streams - Array of file pointers
 */
static void log_set_streams(FILE **streams)
{
    logger.streams = streams;
}

/**
 * Gets the current array of output streams. The result of this function is NOT
 * thread-safe.
 *
 * @return Current array of output streams.
 */
static FILE **log_get_streams(void)
{
    FILE **streams;

    streams = logger.streams;

    return streams;
}

/**
 * Sets the logging format.
 *
 * @param format - Logging format flags
 */
static void log_set_format(int format)
{
    logger.format = format;
}

/**
 * Gets the current logging format.
 *
 * @return Current logging format flags.
 */
static int log_get_format(void)
{
    int format;

    format = logger.format;

    return format;
}

/**
 * Sets the logging severity level.
 *
 * @param level - Logging severity level
 */
static void log_set_level(int level)
{
    logger.level = level;
}

/**
 * Gets the current logging severity level.
 *
 * @return Current logging severity level.
 */
static int log_get_level(void)
{
    int level;

    level = logger.level;

    return level;
}

/**
 * Gets the current logging size, if current size bigger than the maximum, override it.
 *
 * @return state.
 */
static int log_check_reset(void){
	long filesize = 0;

	if((filesize=ftell(logger.streams[0]))==-1)
	{
		return 0;
	}
	if (filesize>=logger.max_fz)
	{
		fclose(logger.streams[0]);
		if((logger.streams[0] = fopen(log_file_name, "w")) == NULL) 
		{
			return 0;
		}
	}
	return 1;
}

/**
 * If the severity level of the call is higher than the logger's level, prints
 * the log message to the output stream. If an output stream was not defined,
 * "stdout" will be used instead. Logging a fatal message terminates the
 * program.
 *
 * @param file  - File name
 * @param line  - Line number
 * @param level - Severity level of the call
 * @param ...   - Message to be logged and its arguments
 */
void log_print(const char *file, int line, int level, ...)
{
    FILE *DEFAULT_STREAMS[2] = {stdout, NULL};
    FILE **streams;
    time_t secs;
    struct tm now;
    va_list msg;
    unsigned int i;

	if (log_enable == 0) {
		return;
	}

    pthread_mutex_lock(&logger.lock);

    if (level >= logger.level) {
        streams = logger.streams;
        if (streams == NULL)
            streams = DEFAULT_STREAMS;

        time(&secs);
        localtime_r(&secs, &now);
        now.tm_mon  += 1;
        now.tm_year += 1900;

		if (!log_check_reset())
		{
			pthread_mutex_unlock(&logger.lock);
			exit(1);
		}	

        for (i = 0; streams[i] != NULL; ++i) {
            if (logger.format & LOG_PRINT_DATE)
                fprintf(streams[i], "%04d-%02d-%02d ",
                        now.tm_year, now.tm_mon, now.tm_mday);

            if (logger.format & LOG_PRINT_TIME)
                fprintf(streams[i], "%02d:%02d:%02d ",
                        now.tm_hour, now.tm_min, now.tm_sec);

//           fprintf(streams[i], "%s -", logger.ident);

            if (logger.format & LOG_PRINT_TAG)
               	fprintf(streams[i], "[%s] ", log_tag(level));

            if (logger.format & LOG_PRINT_FILE)
                fprintf(streams[i], "%s():%d ", NO_DIR_FILENAME(file), line);

            va_start(msg, level);
            vfprintf(streams[i], va_arg(msg, const char*), msg);
            va_end(msg);

            fprintf(streams[i], "\n");

            if (level == LOG_LEVEL_FATAL)
                fclose(streams[i]);
			else
                fflush(streams[i]);
        }
    }

    pthread_mutex_unlock(&logger.lock);

    if (level == LOG_LEVEL_FATAL) {
        pthread_mutex_destroy(&logger.lock);
        exit(EXIT_FAILURE);
    }
}

/**
 * Returns the tag associated to a given severity level.
 *
 * @param level - Severity level of the call
 * @return A tag string associated to the input severity level.
 */
static const char *log_tag(int level)
{
    switch (level) {
    case LOG_LEVEL_TRACE:
        return "TRACE";
    case LOG_LEVEL_DEBUG:
        return "DEBUG";
    case LOG_LEVEL_INFO:
        return "INFO ";
    case LOG_LEVEL_WARN:
        return "WARN ";
    case LOG_LEVEL_ERROR:
        return "ERROR";
    case LOG_LEVEL_FATAL:
        return "FATAL";
    default:
        return "     ";
    }
}

/**
 * Shutdown the stream.
 *
 * @param 
 */
void log_exit(void)
{
	int i;
	pthread_mutex_lock(&logger.lock);
	for (i=0; i<3; i++)
	{
		if (logs[i] != NULL)
			fclose(logs[i]);
	}
	pthread_mutex_unlock(&logger.lock);
	return;
}

