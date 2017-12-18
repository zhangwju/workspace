#ifndef __NANO_IPC_H_
#define __NANO_IPC_H_
#define NN_SOCKET_ADDR		"ipc:///tmp/nn_link_status"

extern int nn_socket_ipc_init(int *fd, const char * ipc_addr);
extern int nn_socket_send(int fd,const void * send_buf,int buf_len);
extern void nn_socket_close(int fd);

#endif //__NANO_IPC_H_
