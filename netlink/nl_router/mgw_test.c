#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include "mgw_router.h"

int main(int argc, char **argv)
{
	rtinfo_t rt;
	void *nl;

	if(argc != 5) {
		printf("Usage: %s <dst> <mask> <gw> <ifname>\n", argv[0]);
		exit(1);
	}

	nl = mgw_route_init();
	if(NULL == nl) {
		return -1;
	}

	memset(&rt, 0, sizeof(rt));	
	rt.dst	 	= inet_addr(argv[1]);
	rt.gw	 	= inet_addr(argv[3]);
	rt.mask	 	= atoi(argv[2]);
	rt.metric 	= 10;
	strcpy(rt.ifname, argv[4]);
	
	mgw_route_add(nl, &rt);
	mgw_route_del(nl, &rt);	
	mgw_route_get(nl);
	mgw_route_release(nl);

	return 0;
}
