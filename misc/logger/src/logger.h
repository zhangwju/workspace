#ifndef __LOGGER_H__
#define __LOGGER_H__

#define LOG_DEFAULT_FILESIZE		(1024 * 1024)

#ifndef MAX_LOG_FILE_NAME_LEN
#define MAX_LOG_FILE_NAME_lEN		128
#endif

#define LOG_DEBUG 	1
#define LOG_INFO	2
#define LOG_WARN	3
#define LOG_ERROR	4

int logger_init(char * logfile, int filesize, int level);
void logger_deinit();
void logger_set_enable();
void logger_set_disable();
void logger_set_level(int level);
void logger_print(const char *format, const char *func, unsigned int line, int level, ...);

#define log_error(format, ...)	logger_print(format, __func__, __LINE__, LOG_ERROR, ## __VA_ARGS__) 

#define log_warn(format, ...)	logger_print(format, __func__, __LINE__, LOG_WARN, ## __VA_ARGS__)

#define log_info(format, ...)	logger_print(format, __func__, __LINE__, LOG_INFO, ## __VA_ARGS__) 

#define log_debug(format, ...)	logger_print(format, __func__, __LINE__, LOG_DEBUG, ## __VA_ARGS__) 

#endif //__LOGGER_H_
