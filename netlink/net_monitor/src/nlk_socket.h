#ifndef _MGNL_SOCKET_H_
#define _MGNL_SOCKET_H_
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <linux/netlink.h>

struct nlk_socket {
	int 				fd;
	struct sockaddr_nl  addr;
};

struct nlk_socket * nlk_socket_open(int bus);
void nlk_socket_close(struct nlk_socket *nl);
int nlk_socket_bind(struct nlk_socket *nl, unsigned int groups, pid_t pid);
ssize_t nlk_socket_sendto(const struct nlk_socket *nl, const void *buf, size_t len);
ssize_t nlk_socket_recvfrom(const struct nlk_socket *nl, void *buf, size_t bufsiz);

#endif //_MGNL_SOCKET_H_
