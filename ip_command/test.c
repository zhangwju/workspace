#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include "ip_common.h"

int main()
{
	int ret = 0;	

	/* route test */
	if (iproute_add(0, 0, inet_addr("192.168.16.1"), 10, 0, "enp0s3") < 0) { /*  test default route */
		fprintf(stderr, "ip route add, error:[%d:%s]\n", errno, strerror(errno));
	}

	if (iproute_add(inet_addr("192.168.16.102"), 32, inet_addr("192.168.16.1"), 10, 0, "enp0s3") < 0) {
		fprintf(stderr, "ip route add, error:[%d:%s]\n", errno, strerror(errno));
	}

	if (iproute_add(inet_addr("192.168.16.102"), 32, inet_addr("192.168.16.1"), 10, 0, "enp0s3") < 0) {
		fprintf(stderr, "ip route add, error:[%d:%s]\n", errno, strerror(errno));
	}

	if (iproute_del(inet_addr("192.168.16.102"), 32, inet_addr("192.168.16.1"), 10, 0, "enp0s3") < 0) {
		fprintf(stderr, "ip route del, error:[%d:%s]\n", errno, strerror(errno));
	}

	if (iproute_del(inet_addr("192.168.16.102"), 32, inet_addr("192.168.16.1"), 10, 0, "enp0s3") < 0) {
		fprintf(stderr, "ip route del, error:[%d:%s]\n", errno, strerror(errno));
	}
	

	/* rule test */
	if (iprule_add(inet_addr("192.168.16.102"), 32, inet_addr("192.168.16.0"), 24, 0x1266, 15, 1266, 0, NULL, NULL) < 0) {
		fprintf(stderr, "ip rule add, error:[%d:%s]\n", errno, strerror(errno));
	}

	if (iprule_add(inet_addr("192.168.16.102"), 32, inet_addr("192.168.16.0"), 24, 0x1266, 15, 1266, 0, NULL, NULL) < 0) {
		fprintf(stderr, "ip rule add, error:[%d:%s]\n", errno, strerror(errno));
	}

	if (iprule_del(inet_addr("192.168.16.102"), 32, inet_addr("192.168.16.0"), 24, 0x1266, 15, 1266, 0, NULL, NULL) < 0) {
		fprintf(stderr, "ip rule del, error:[%d:%s]\n", errno, strerror(errno));
	}

	if (iprule_del(inet_addr("192.168.16.102"), 32, inet_addr("192.168.16.0"), 24, 0x1266, 15, 1266, 0, NULL, NULL) < 0) {
		fprintf(stderr, "ip rule del, error:[%d:%s]\n", errno, strerror(errno));
	}

	return 0;
}
