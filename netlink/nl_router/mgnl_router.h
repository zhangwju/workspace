#ifndef __MGNL_ROUTER_H_
#define __MGNL_ROUTER_H_
#include "mgnl_socket.h"

#ifndef IFNAMESIZE
#define IFNAMESIZE 16
#endif

typedef struct nl_rtinfo {
	unsigned int 	src;
	unsigned int 	dst;
	unsigned int 	gw;
	unsigned int	mask;
	int 			tid;
	int 			metric;
	char			ifname[IFNAMESIZE];
}nl_rtinfo_t;

extern void *mgnl_init();
extern void mgnl_release(void *nl);
extern int nl_route_add(struct mgnl_socket *nl, nl_rtinfo_t *rt);
extern int nl_route_del(struct mgnl_socket *nl, nl_rtinfo_t *rt);
extern int nl_route_flush(struct mgnl_socket *nl, int table);
#endif // __MGNL_ROUTER_H_
