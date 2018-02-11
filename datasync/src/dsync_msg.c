#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>

#include "common.h"

#define RMSG_LOOP		3

int send_dmsg(int sock, void *buf, int len)
{
	int i;
	int offset = 0;

	for (i = 0; i < RMSG_LOOP; i++) {
		offset += send(sock, buf, len, 0);
		if (offset >= len) {
			break;
		}
	}
	return offset;
}

int recv_dmsg(int sock, void *buf, int len)
{
	int i;
	int offset = 0;
	
	for (i = 0; i < RMSG_LOOP; i++) {
		offset += recv(sock, buf, len, 0);
		if (offset >= len) {
			break;
		}
	}
	return offset;
}

int accept_dmsg(int sock) 
{
	int accept_sock;
	int backlog;
	int timeout;
	struct sockaddr_in clientaddr;
	socklen_t addrlen;
	
	backlog = 1;
	if (listen(sock, backlog) < 0) {
		return -1;
	}
	
	addrlen = sizeof(struct sockaddr);
	accept_sock = accept(sock, (struct sockaddr *)&clientaddr, &addrlen);
	if (accept_sock < 0) {
		log_dbg("Accept dmsg, Error:[%d:%s]", errno, strerror);
		return -1;
	}
	timeout = 2000;
	setsockopt(accept_sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(int));
	setsockopt(accept_sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(int));

	log_dbg("accept: %s:%d, sockfd: %d", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port), accept_sock);
	return accept_sock;	
}

int init_dsync_server(uint srcaddr, ushort port, const char *ifname)
{
	int timeout;
	int opt_val;
	int sockfd;
	struct ifreq data;
	struct sockaddr_in servaddr;
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		log_dbg("Init dsync server, Error:[%d:%s]", errno, strerror(errno));
		return -1;
	}

	memset(&servaddr, 0, sizeof(struct sockaddr_in));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(srcaddr);
	servaddr.sin_port = htons(port);

	if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr)) < 0) {
		log_dbg("dsync server bind, error:[%d:%s]", errno, strerror(errno));
		close(sockfd);
		return -1;
	}

	if (*ifname) {
		strncpy(data.ifr_name, ifname, IFNAMESZ);
		if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, (char *)&data, sizeof(data))  < 0) {
			log_dbg("Set Socket option bind to device: %s failed", ifname);
			close(sockfd);
			return -1; 
		}
	} 

	timeout = 2000;
	opt_val = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(int));
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt_val, sizeof(opt_val));

	return sockfd;
}

int init_dsync_client(uint addr, ushort dstport)
{
	int bufsize;
	int timeout;
	int sockfd;
	struct sockaddr_in dstaddr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		log_dbg("Init dsync client, Error:[%d:%s]", errno, strerror(errno));
		return -1;
	}
	memset(&dstaddr, 0, sizeof(dstaddr));
	dstaddr.sin_family = AF_INET;
	dstaddr.sin_addr.s_addr = htonl(addr);
	dstaddr.sin_port = htons(dstport);
	
	timeout = 2000;
	setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(int));
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(int));

	bufsize = (4 << 10); //4k
	setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (char *)&bufsize, sizeof(int));

	if (connect(sockfd, (struct sockaddr *)&dstaddr, sizeof(struct sockaddr)) < 0) {
		log_dbg("dsync client, Error:[%d:%s]", errno, strerror(errno));
		close(sockfd);
		return -1;
	}
	return sockfd;	
}
