/* Filename: l2_detect.c , created by Teidor.Tien, 2016/12/08 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libubox/uloop.h>
#include <libubox/ustream.h>
#include <libubox/utils.h>
#include <libubus.h>

#include "common.h"
#include "nano_ipc.h"
#include "l3_detect.h"
#include "l2_detect.h"
#include "iface_info.h"

/************************************************************************
  *                                                     DEFINITION                                               *
  ************************************************************************/
extern struct linkmgt_info* plkmgt;
extern pthread_rwlock_t g_RWLock;
extern pthread_mutex_t g_mutex;
extern int nn_cli_fd;

/************************************************************************
  *                                                     FUNCTIONS                                               *
  ************************************************************************/
 int netifd_interface_state(
								struct ubus_context *ctx, 
								struct ubus_object *obj,
								struct ubus_request_data *req,
								const char *method,
								struct blob_attr *msg);

int l2_uplink_notify(int uplink_type, int status, int subid, int rate, struct netinfo net) 
{
	struct modlue_notify_info *msg = NULL;
	struct uplink_info_report uplink;

	msg = (struct modlue_notify_info *)malloc(sizeof(struct modlue_notify_info) + sizeof(struct uplink_info_report));
	if (NULL == msg) {
		return -1;
	}
	
	msg->msg_id = 0;
	memset(&uplink, 0, sizeof(uplink));
	uplink.uplink_type = uplink_type;
	uplink.status = status;
	uplink.uplink_subid = subid;
	uplink.rate = rate;
	uplink.ip = htonl(net.ip);
	uplink.mask = htonl(net.mask);
	uplink.gw = htonl(net.gw);
	uplink.dns = htonl(net.dns);
	memcpy((void *)msg->data, (void *)&uplink, sizeof(struct uplink_info_report));

	pthread_mutex_lock(&g_mutex);
	nn_socket_send(nn_cli_fd, (void *)msg, 
		(sizeof(struct modlue_notify_info) + sizeof(struct uplink_info_report)));
	pthread_mutex_unlock(&g_mutex);
	free(msg);

	return 0;
}

struct uplink_info * get_uplink_info_by_ifname(const char *ifname)
{
	int i = 0;
	struct uplinks *dev;
	dev = &plkmgt->dev;

	for (i = 0; i < dev->uplinks; i++) {
		if (!strcmp(dev->uplink[i].ifname, ifname)) {
			return &dev->uplink[i];
		}
	}
	return NULL;
}

static int netifd_report_handle(const char *device, unsigned int state)
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
		uplink->crtflag = 1;
		
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

struct ubus_subscriber ubus_event;
struct ubus_context *ctx = NULL;

static const struct blobmsg_policy new_policy[UBUS_EVENT_MAX] = {
	[UBUS_INTERFACE_EVENT] = { .name = "interface", .type = BLOBMSG_TYPE_STRING },
	[UBUS_DEVICE_NAME_EVENT] = { .name = "devname", .type = BLOBMSG_TYPE_STRING },
	[UBUS_METHOD_EVENT] = { .name = "method", .type = BLOBMSG_TYPE_INT32 },
};

static struct ubus_method linkdetect_obj_method[] =
{
	UBUS_METHOD("ifacenotify", 
				netifd_interface_state, 
				new_policy),
};

static struct ubus_object_type linkdetect_obj_type =
	UBUS_OBJECT_TYPE("mw_iface_forward", linkdetect_obj_method);

static struct ubus_object linkdetect_object = {
	.name = "mw_iface_forward",
	.type = &linkdetect_obj_type,
	.methods = linkdetect_obj_method,
	.n_methods = ARRAY_SIZE(linkdetect_obj_method),
};

int 
netifd_interface_state(
	struct ubus_context *ctx, 
	struct ubus_object *obj,
	struct ubus_request_data *req,
	const char *method,
	struct blob_attr *msg)
{
	struct blob_attr *tb[UBUS_EVENT_MAX];
	unsigned int msg_type;
	char *interface = NULL;
	char *device = NULL;
	
	blobmsg_parse(new_policy, UBUS_EVENT_MAX, tb, blob_data(msg), blob_len(msg));
	if(!tb[UBUS_INTERFACE_EVENT])
	{
		log_dbg("no data contained");
		return -1;
	}
	
	interface = blobmsg_get_string(tb[UBUS_INTERFACE_EVENT]);
	device = blobmsg_get_string(tb[UBUS_DEVICE_NAME_EVENT]);
	msg_type = blobmsg_get_u32(tb[UBUS_METHOD_EVENT]);
	
	if(interface == NULL || device == NULL)
		return -1;

	/* dirty set it */
	if (msg_type == DEV_EVENT_REMOVE)
		msg_type = DEV_EVENT_LINK_DOWN;

	netifd_report_handle(device, msg_type);
	
	return 0;
}

void *l2_thread_handler(void *arg)
{
	int ret;

	uloop_init();
	signal(SIGPIPE, SIG_IGN);
	
	ctx = ubus_connect(NULL);
	if (!ctx) {
		log_dbg("Error ubus_connect");
		return;
	}
	ubus_add_uloop(ctx);

	ret = ubus_add_object(ctx, &linkdetect_object);
	if (ret)
	{
		log_dbg("Failed to add linkdetect_object");
		return;
	}
	ret = ubus_register_subscriber(ctx, &ubus_event);
	if (ret)
	{
		log_dbg("Failed to add linkdetect_object");
		return;
	}

	uloop_run();
	ubus_free(ctx);
	uloop_done();

	return;
}
