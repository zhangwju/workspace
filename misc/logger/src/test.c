/**********************
 * test code 
 * *******************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "logger.h"

int main()
{
	if (logger_init("test.log", 1024*1024, LOG_DEBUG) < 0) {
		return -1;	
	}
	logger_set_enable();

	log_debug("This is debug, %s %d", "first debug", 1);
	log_info(" This is info");
	log_warn(" This is warnning");
	log_error("This is error");
	
	logger_deinit();

	return 0;
}
