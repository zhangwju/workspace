#ifndef __MGNL_ROUTER_H_
#define __MGNL_ROUTER_H_

#ifndef IFNAMESE
#define IFNAMESE 16
#endif

typedef struct nl_rtinfo {
	unsigned int 	src;
	unsigned int 	dst;
	unsigned int 	gw;
	unsigned int	mask;
	int 			tid;
	int 			metric;
	char			ifname[IFNAMESE];
}nl_rtinfo_t;

extern void * mgnl_init();
extern void mgnl_release(void *nl);
extern int nl_route_add(void *nl, nl_rtinfo_t *rt);
extern int nl_route_del(void *nl, nl_rtinfo_t *rt);
extern int nl_route_get(void *nl);
extern int nl_route_flush(void *nl, int table);
#endif // __MGNL_ROUTER_H_
