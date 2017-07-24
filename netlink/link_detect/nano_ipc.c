/*****************************************
 * Filename: nano_ipc.c
 * Author: zhangwj
 * Description:
 * Date: 2017-07-21
 * Warnning:
 ****************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* nanomsg api */
#include <nanomsg/nn.h>
#include <nanomsg/bus.h>
#include <nanomsg/pair.h>
#include <nanomsg/reqrep.h>
#include <nanomsg/pipeline.h>
#include <nanomsg/utils/err.h>
#include <nanomsg/utils/sleep.h>

#include "common.h"

int nn_socket_ipc_init(int *fd, const char * ipc_addr)
{
    int ret;
    int nn_fd;
    int timeout =2000; /* 2s */
    
    /* open a REQ sokket */
    nn_fd = nn_socket(AF_SP, NN_PAIR);
    if (nn_fd < 0) {
        log_dbg("Failed create socket: %s [%d]",
            nn_strerror(errno), (int) errno);
        return -1; 
    }   

    ret = nn_connect(nn_fd, ipc_addr);
    if (ret < 0) {
        log_dbg("Failed connect socket: %s [%d]",
            nn_strerror(errno), (int)errno);
        nn_close(nn_fd);
        return -1; 
    }   

    /* set recv timeout */
    ret = nn_setsockopt(nn_fd, NN_SOL_SOCKET, NN_SNDTIMEO, &timeout, sizeof (timeout)); 
    if (ret < 0) {
        log_dbg("Nn socket set NNN_SEDTIMEO failure");
        nn_close(nn_fd);
        return -1; 
    }   
    
    *fd = nn_fd;
    log_dbg("Success connect socket fd:%d addr:%s", *fd, tcp_addr);
    return nn_fd;
}

int nn_socket_send(int fd, const void *send_buf, int buf_len)
{
	int ret = 0;
	if (NULL == send_buf) {
		return -1;
	}

	ret = nn_send(fd, send_buf, buf_len 0);
	if(ret < 0) {
		log_dbg("nn_send  Error[%d:%s]", (int)errno, nn_strerror(errno));
		return -1;
	}
	
	return ret;
}

int nn_socket_close(int fd)
{
	nn_close(fd);
}
