#ifndef __IFACE_INFO_H_
#define __IFACE_INFO_H_
#include "common.h"

struct iface_info_t {
	char ifname[IFNAMESZ];
	int subid;		//for lte-lines
	int state;
	int type;
	struct iface_info_t *next;
};

struct iface_info_t * iface_node_insert(const char *ifname, int type, int subid, int status);
struct iface_info_t *get_iface_node(const char *ifname);
int iface_node_state_change(struct iface_info_t *if_node,  int state);
void iface_node_release(void);
#endif  //__IFACE_INFO_H_
