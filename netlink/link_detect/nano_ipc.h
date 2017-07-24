#ifndef __NANO_IPC_H_
#define __NANO_IPC_H_

extern int nn_socket_ipc_init(int *fd, const char * ipc_addr);
extern int nn_socket_close(int fd);
#endif //__NANO_IPC_H_
