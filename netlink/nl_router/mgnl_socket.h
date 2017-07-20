#ifndef _MGNL_SOCKET_H_
#define _MGNL_SOCKET_H_
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <linux/netlink.h>

struct mgnl_socket {
	int 				fd;
	struct sockaddr_nl  addr;
	unsigned int seq;
	unsigned int dump;
};

struct mgnl_socket *mgnl_socket_open(int bus);
void mgnl_socket_close(struct mgnl_socket *nl);
int mgnl_socket_bind(struct mgnl_socket *nl, unsigned int groups, pid_t pid);
ssize_t mgnl_socket_sendto(const struct mgnl_socket *nl, const void *buf, size_t len);
ssize_t mgnl_socket_recvfrom(const struct mgnl_socket *nl, void *buf, size_t bufsiz);

#endif //_MGNL_SOCKET_H_
