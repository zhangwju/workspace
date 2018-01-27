#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include "nano_ipc.h"
#include "common.h"
#include "logger.h"
#include "l2_detect.h"
#include "l3_detect.h"
#include "iface_info.h"

#define VIRT_PORT		9573

extern struct link_detect_conf *g_conf;
extern pthread_rwlock_t g_RWLock;
extern pthread_mutex_t g_mutex;
extern unsigned int g_virt_thread_ctr;
extern int nn_cli_fd;

struct virt_notify {
    char ifname[IFNAMESZ];
    int status; 
};

int init_virt_server(uint srcaddr, int port)
{
	int sock;
	struct sockaddr_in servaddr;
	
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock < 0) {
		log_dbg("Create Virt socket,Error: %s", strerror(errno));
		return -1;
	}

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(port);

	if(bind(sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
		log_dbg("Bind Virt Soket, Error: %s", strerror(errno));
		close(sock);
		return -1;
	}
	return sock;
}

int accept_dmsg(int servsock)
{
    int accsock;
    int addrsize;
    int rbufsize;
    struct sockaddr_in clientaddr;
                
    //do listen
    if (listen(servsock, 1) < 0) {
        return -1; 
    }   
                    
    //accept socket
    addrsize = sizeof(struct sockaddr);
    accsock = accept(servsock,(struct sockaddr *)&clientaddr, &addrsize);
    if (accsock < 0) {
        return -1; 
    }   
        
    return accsock;
}

static int virt_notify_packet_handle(const char *device, int state)
{
	int ret;
	int status;
	struct uplink_info * uplink = NULL;
	struct netinfo net;
	
	if (device == NULL)
		return -1;

	uplink = get_uplink_info_by_ifname(device);
	if (NULL == uplink){
		return -1;
	}

	if (state == DEV_EVENT_LINK_DOWN) {
		status = UPLINK_STATUS_DOWN;
		uplink->l2_state = 1;
		
		memset(&net, 0, sizeof(net));
		l2_uplink_notify(uplink->type, status, uplink->subid, 0, net);
		/* notify manager module */
		log_dbg("Interface: %-5s Type: %-10s Subid: %-2d Status: %-5s Rate: %d", 
			uplink->ifname, uplink_type[uplink->type], uplink->subid, "down", 0);	
	} else {
		uplink->l2_state = 0;
	}
	
	return 0;
}

void * virt_notify_handle(void *arg)
{
	int size;
	int virt_sock = -1;
	char buf[512];
	socklen_t socklen;
	struct sockaddr_in srcaddr;
	struct modlue_notify_info *msg;
	struct virt_notify *notify;

	while(g_virt_thread_ctr) {
		if(virt_sock < 0) {
			virt_sock = init_virt_server(0, VIRT_PORT);
			if(virt_sock < 0) {
				log_dbg("Init Virt server failure");
				usleep(200000);
				continue;
			}
		}
		size = recvfrom(virt_sock, buf, sizeof(buf), 0, (struct sockaddr *)&srcaddr, &socklen);
		if(size < 0) {
			if (errno == EINTR || errno == EAGAIN)
				continue;
		}

		msg = (struct modlue_notify_info *)buf;
		if(msg->msg_id == 0 && msg->msg_len > 8) {
			notify = (struct virt_notify *)msg->data;
			virt_notify_packet_handle(notify->ifname, notify->status);
		}	
	}
	close(virt_sock); //close virt_socket
}
