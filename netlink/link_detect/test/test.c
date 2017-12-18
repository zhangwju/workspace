#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <nanomsg/nn.h>
#include <nanomsg/pair.h>
#include <errno.h>

#define NN_SOCKET_ADDRESS "ipc:///tmp/nn_link_status"

#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"
#define STR2MAC(x) &(x)[0],&(x)[1],&(x)[2],&(x)[3],&(x)[4],&(x)[5]
#define STR_TO_MAC(str, mac) \
    sscanf(str, MACSTR, STR2MAC((mac)))

#define IPSTR "%u.%u.%u.%u"
#define IP2STR(a)  ((a)>>24)&0xff, ((a)>>16)&0xff, ((a)>>8)&0xff, (a)&0xff

struct uplink_info_report {
	char ifname[16];
	int type;
	int subid; //for lte-lines
	int status;	
	int rate;
	uint ip;
	uint mask;
	uint gw;
	uint dns;
};

struct modlue_notify_info {
	int msg_id;
	int msg_irsv;
	char data[0];
};

static int g_nn_fd;
const char *uplink_type[] = {"Fiber", "4G", "Micros", "Satellite", "Lan", "Unknown"};

int link_nn_sock_init()
{
    int ret;
    int nn_fd;

    nn_fd = nn_socket (AF_SP, NN_PAIR);
    if (nn_fd < 0) {
        printf("Failed create socket: %s [%d]",
			nn_strerror(errno), (int) errno);
        return -1; 
    }   
 
    ret = nn_bind(nn_fd, NN_SOCKET_ADDRESS);
    if (ret < 0) {

        printf("Failed bind");
        nn_close(nn_fd);
        return -1; 
    }   
    g_nn_fd = nn_fd;
    printf("Success connect socket ipc:%s\n", NN_SOCKET_ADDRESS);

    return 0;
}

int link_nn_sock_recv(char *recv_buf, int buflen)
{
    int ret = 0;
    ret = nn_recv(g_nn_fd, (void *)recv_buf, buflen, 0); 
    if (ret < 0) {
        printf ("Received request len:%d url:%s", ret, recv_buf);
    }   
    recv_buf[ret] = '\0';

    return ret;
}

static void uplink_info_show(struct uplink_info_report *uplink)
{
	char ip[16] = {0};
	char mask[16] = {0};
	char gw[16] = {0};
	
	sprintf(ip, IPSTR, IP2STR(ntohl(uplink->ip)));
	sprintf(mask, IPSTR, IP2STR(ntohl(uplink->mask)));
	sprintf(gw, IPSTR, IP2STR(ntohl(uplink->gw)));

	printf("Type: %-10s Subid: %-2d Status: %-5s Ip: %-16s Mask: %-16s Gateway: %-16s\n", 
		uplink_type[uplink->type], uplink->subid, (uplink->status == 0)?"down":"up", ip, mask, gw);
}

int main(int argc, char **argv)
{
	char buf[1024] = {0};
	struct modlue_notify_info *notify = NULL;
	struct uplink_info_report *uplink = NULL;

	if (link_nn_sock_init() < 0) {
		return -1;
	}

	while(1) 
	{
    	if (!link_nn_sock_recv(buf, sizeof(buf))) {
        	continue;
    	}

		notify = (struct modlue_notify_info *)buf;
		if (notify->msg_id == 0) {
			uplink = (struct uplink_info_report *)notify->data;
			uplink_info_show(uplink);
		}
	}
	nn_close(g_nn_fd);
	return 0;
}
