/*
 *  logger.h
 *
 *  Copyright (c) 2014 Daniel Rocha. All rights reserved.
 */

#ifndef LOGGER_H
#define LOGGER_H

/* -------------------------------------------------------------------------- *
 *                                 CONSTANTS                                  *
 * -------------------------------------------------------------------------- */

#define LOG_PRINT_DATE 				0x01
#define LOG_PRINT_TIME 				0x02
#define LOG_PRINT_FILE 				0x04
#define LOG_PRINT_TAG  				0x08
#define LOG_PRINT_ALL   			0x0f
#define LOG_PRINT_NONE  			0x00

typedef enum {
	LOG_LEVEL_ALL = 				0,
	LOG_LEVEL_TRACE,
	LOG_LEVEL_DEBUG,
	LOG_LEVEL_INFO,
	LOG_LEVEL_WARN,
	LOG_LEVEL_ERROR,
	LOG_LEVEL_FATAL,
	LOG_LEVEL_OFF
}E_LOG_LEVEL;

/*  Entry access of logger,  determined the logger identifier, file path , file size , the logger level and screen output*/
int log_init(const char *ident, const char *log_file, int filesize, E_LOG_LEVEL level, int scn_op);

/** Logs a message. */
void log_print(const char *file, int line, int level, ...);

/* Shutdown the logger stream */
void log_exit(void);

#endif
