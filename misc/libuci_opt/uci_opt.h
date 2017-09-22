#ifndef __UCI_OPT_H__
#define __UCI_OPT_H__
#include <uci.h>

#define MAX_UCI_VALUE_LEN           2048

#ifndef IFNAMESZ
#define IFNAMESZ	32
#endif

/*
enum uci_option_type {
    UCI_TYPE_STRING = 0,
    UCI_TYPE_LIST = 1,
};
*/

enum uci_config_type {
	CMD_GET,
    CMD_SET, 
    CMD_ADD_LIST,
    CMD_DEL_LIST,
    CMD_DEL,
};

int uci_opt_commit(char *package);
char * uci_get_option(const char *package, const char *section, const char *option);
int uci_set_section(const char *package, const char *section_type, const char *section);
int uci_del_option(const char *package, const char *section, const char *option, const char *value, enum uci_option_type type);
int uci_set_option(const char *package, const char *section, const char *option, const char *value, enum uci_option_type type);
#endif //__UCI_OPT_H__
