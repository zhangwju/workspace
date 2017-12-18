/***********************************************
 * Filename: sockbase.c
 * Author: zhangwj
 * Date: 2017-07-03
 * Email:
 ***********************************************/
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>

#ifndef IPLEN
#define IPLEN 16
#endif

#ifndef HWADDRLEN
#define HWADDRLEN 6
#endif 

#ifndef IFNAMSIZ
#define IFNAMSIZ 16
#endif

#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"
#define STR2MAC(x) &(x)[0],&(x)[1],&(x)[2],&(x)[3],&(x)[4],&(x)[5]
#define STR_TO_MAC(str, mac) \
    sscanf(str, MACSTR, STR2MAC((mac)))

#define IPSTR "%u.%u.%u.%u"
#define IP2STR(a) (a)&0xff, ((a)>>8)&0xff, ((a)>>16)&0xff, ((a)>>24)&0xff

typedef struct if_info
{
	char ip[IPLEN];
	char bro_ip[IPLEN];
	unsigned char mac[HWADDRLEN];
}if_info_t;

/* get dev ip address */
int get_ip_by_name(const char *ifname, char *ip)
{	
	int sock = 0;
	struct sockaddr_in sin;
	struct ifreq ifr;

	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ - 1] = 0;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock < 0) {
		perror("socket");
		return -1;
	}
	
	if(ioctl(sock,	SIOCGIFADDR,	&ifr) == 0) {
		memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
		memcpy(ip, inet_ntoa(sin.sin_addr), IPLEN);
		return 0;	
	}

	close(sock);
	return -1;
}

int get_broip_by_name(const char *ifname, char *bro_ip)
{
	int sock = 0;
	struct sockaddr_in sin;
	struct ifreq ifr;

	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ - 1] = 0;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock < 0) {
		perror("socket");
		return -1;
	}
	
	if(ioctl(sock, SIOCGIFBRDADDR, &ifr) == 0) {
		memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
		memcpy(bro_ip, inet_ntoa(sin.sin_addr), IPLEN);
		return 0;	
	}

	close(sock);
	return -1;
	
}

int get_mac_by_name(const char *ifname, char * mac)
{
	int sock = 0;
	struct ifreq ifr;
	unsigned char tmp_mac[6];

	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ - 1] = 0;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock < 0) {
		perror("socket");
		return -1;
	}
	
	if(ioctl(sock, SIOCGIFHWADDR, &ifr) == 0) {
		memcpy(tmp_mac, ifr.ifr_hwaddr.sa_data, 6);
		sprintf(mac, MACSTR, MAC2STR(tmp_mac));
		return 0;	
	}

	close(sock);
	return -1;
	
}

int get_linkstat_by_name(const char *ifname)
{	
	int sock = 0;
	struct ifreq ifr;
	int flags = -1;
	
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock < 0) {
		perror("socket");
		return -1;
	}
	
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	if(ioctl(sock, SIOCGIFFLAGS, &ifr) == 0) {
		if(ifr.ifr_flags & IFF_RUNNING) {
			flags = 0;
		}
	}

	return flags;	
}

/* setsockopt SO_RCVTIMEO */
int sock_set_recv_timeout(int sock, struct timeval *timeout)
{
	if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (void *)timeout, sizeof(struct timeval)) < 0) {
		perror("setsockopt SO_RCVTIMEO");
		return -1;
	}

	return 0;
}

/* setsockopt SO_SNDTIMEO */
int sock_set_send_timeout(int sock, struct timeval *timeout)
{
	if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (void *)timeout, sizeof(struct timeval)) < 0) {
		perror("setsockopt SO_SNDTIMEO");
		return -1;
	}

	return 0;
}

/* setsockopt SO_BROADCAST */
int sock_set_broadcast(int sock)
{
	int brodcast = 1;
	
	if(setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &brodcast, sizeof(brodcast)) < 0) {
		perror("setsockopt SO_BROADCAST");
		return -1;
	}

	return 0;
}

/* setsockopt SO_BINDTODEVICE */
int sock_bind_to_device(int sock, const char *ifname)
{
	struct ifreq ifr;

	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	if(setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr)) < 0) {
		perror("setsockopt");
		return -1;
	}

	return 0;
}

int AddMulticastMembership(int s, const char *mcast_addr, const char *ifaddr)
{
	struct ip_mreq imr;	/* Ip multicast membership */

	/* setting up imr structure */
	imr.imr_multiaddr.s_addr = inet_addr(mcast_addr);
	/*imr.imr_interface.s_addr = htonl(INADDR_ANY);*/
	imr.imr_interface.s_addr = inet_addr(ifaddr);

	if (setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *)&imr, sizeof(struct ip_mreq)) < 0) {
		perror("setsockopt(IP_ADD_MEMBERSHIP)");
		return -1;
	}
	return 0;
}

int DropMulticastMembership(int s, const char *mcast_addr, const char *ifaddr)
{
	struct ip_mreq imr;	/* Ip multicast membership */

	/* setting up imr structure */
	imr.imr_multiaddr.s_addr = inet_addr(mcast_addr);
	/*imr.imr_interface.s_addr = htonl(INADDR_ANY);*/
	imr.imr_interface.s_addr = inet_addr(ifaddr);

	if (setsockopt(s, IPPROTO_IP, IP_DROP_MEMBERSHIP, (void *)&imr, sizeof(struct ip_mreq)) < 0) {
		perror("setsockopt(IP_DROP_MEMBERSHIP)");
		return -1;
	}
	return 0;
}

void test_pthread(pthread_t tid) 
{
    int pthread_kill_err;
    pthread_kill_err = pthread_kill(tid,0);

    if(pthread_kill_err == ESRCH) {
        printf("thread not exists\n");  
    }else if(pthread_kill_err == EINVAL) {
        printf("singnal invalid\n");
    } else {
        printf("thread is alive\n");
    }   
}

int main(int argc, char **argv)
{
	char ip[16];
	char bro_ip[16];
	char mac[20];

	if(argc != 2) {
		printf("Usage: %s <ifname>\n", argv[0]);
		return 0;
	}
	
	if (!get_linkstat_by_name(argv[1])) {
		printf("%s running\n", argv[1]);
	}
	
	if (!get_ip_by_name(argv[1], ip)) {
		printf("ip: %s\n", ip);
	}

	if (!get_broip_by_name(argv[1], bro_ip)) {
		printf("bro_ip: %s\n", bro_ip);
	}

	if (!get_mac_by_name(argv[1], mac)) {
		printf("mac: %s\n", mac);
	}

	return 0;
}
