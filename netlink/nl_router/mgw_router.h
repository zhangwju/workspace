#ifndef __MGNL_ROUTER_H_
#define __MGNL_ROUTER_H_

#ifndef IFNAMESE
#define IFNAMESE 16
#endif

typedef struct _rtinfo {
	unsigned int 	dst;
	unsigned int 	gw;
	unsigned int	mask;
	int 			tid;
	int 			metric;
	char			ifname[IFNAMESE];
}rtinfo_t;

/* muti gateway route api */
extern void * mgw_route_init();
extern void mgw_route_release(void *rh);
extern int mgw_route_add(void *rh, rtinfo_t *rt);
extern int mgw_route_del(void *rh, rtinfo_t *rt);
extern int mgw_route_get(void *rh);
extern int mgw_route_flush(void *rh, int table);

/* muti gateway rule api */

#endif // __MGNL_ROUTER_H_
