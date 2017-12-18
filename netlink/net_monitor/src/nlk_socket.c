/******************************
 * Filename: nlk_socket.c
 * Author: zhangwj
 * Description: netlink socket basedef 
 * Date: 2017-07-17
 *******************************/
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include "nlk_socket.h"

int nlk_socket_get_fd(const struct nlk_socket *nl)
{
    return nl->fd;
}

unsigned int nlk_socket_get_portid(const struct nlk_socket *nl)
{
    return nl->addr.nl_pid;
}

struct nlk_socket * nlk_socket_open(int bus)
{
	struct nlk_socket *nl = NULL;
	
	nl = (struct nlk_socket *)calloc(sizeof(struct nlk_socket), 1); 
	if (nl == NULL)
		return NULL;
	
	nl->fd = socket(AF_NETLINK, SOCK_RAW, bus);
	if (nl->fd == -1) {
		free(nl);
		return NULL;
	}   

	return nl; 
}

int nlk_socket_bind(struct nlk_socket *nl, unsigned int groups, pid_t pid)
{
	int ret;
	socklen_t addr_len;
	
	nl->addr.nl_family = AF_NETLINK;
	nl->addr.nl_groups = groups;
	nl->addr.nl_pid = pid;
	
	ret = bind(nl->fd, (struct sockaddr *) &nl->addr, sizeof (nl->addr));
	if (ret < 0)
		return ret;
	
	addr_len = sizeof(nl->addr);
	ret = getsockname(nl->fd, (struct sockaddr *) &nl->addr, &addr_len);
	if (ret < 0)    
		return ret;
	
	if (addr_len != sizeof(nl->addr)) {
		errno = EINVAL;
		return -1; 
	}   
	if (nl->addr.nl_family != AF_NETLINK) {
		errno = EINVAL;
		return -1; 
	}   
	return 0;
}

ssize_t nlk_socket_sendto(const struct nlk_socket *nl, const void *buf, size_t len)
{
	static const struct sockaddr_nl snl = {
		.nl_family = AF_NETLINK
	};
	return sendto(nl->fd, buf, len, 0, 
		(struct sockaddr *) &snl, sizeof(snl));
}

ssize_t nlk_socket_recvfrom(const struct nlk_socket *nl, void *buf, size_t bufsiz)
{
	ssize_t ret;
	struct sockaddr_nl addr;
	struct iovec iov = {
		.iov_base       = buf,
		.iov_len        = bufsiz,
	};
	struct msghdr msg = {
		.msg_name       = &addr,
		.msg_namelen    = sizeof(struct sockaddr_nl),
		.msg_iov        = &iov,
		.msg_iovlen     = 1,
		.msg_control    = NULL,
		.msg_controllen = 0,
		.msg_flags      = 0,
	};
	ret = recvmsg(nl->fd, &msg, 0);
	if (ret == -1)
		return ret;

	if (msg.msg_flags & MSG_TRUNC) {
		errno = ENOSPC;
		return -1;
	}
	if (msg.msg_namelen != sizeof(struct sockaddr_nl)) {
		errno = EINVAL;
		return -1;
	}
	return ret;
}

void nlk_socket_close(struct nlk_socket *nl)
{
	if (nl) {
		if(nl->fd) 
		close(nl->fd);
		free(nl);
		nl = NULL;
	}
}
