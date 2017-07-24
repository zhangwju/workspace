#ifndef __LINE_INFO_H_
#define __LINE_INFO_H_
#include "common.h"

typedef struct iface_info_t {
	char ifname[IFNAMESZ];
	int subid;		//for lte-lines
	int state;
	int type;
	struct iface_info_t *next;
};
#endif  //__LINE_INFO_H_