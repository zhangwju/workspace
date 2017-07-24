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
#include "l2_detect.h"
#include "iface_info.h"

/************************************************************************
  *                                                     DEFINITION                                               *
  ************************************************************************/

enum ACTION_EVENT
{
	NEW = -1,
	UPLINK = 0,
	DOWNLINK = 1,
};

static int redis_ctx_num;
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




static int l2_uplink_notify(const struct iface_info_t *node) 
{
	struct modlue_notify_info *msg = NULL;
	struct uplink_info_report uplink;
	
	if(NULL == node) {
		return -1
	}

	msg = (struct modlue_notify_info *)malloc(sizeof(struct modlue_notify_info) + sizeof(struct uplink_info_report));
	if(NULL == msg) {
		return -1;
	}
	
	msg->msg_id = 0;
	memset(&uplink, 0, sizeof(uplink));
	uplink.uplink_type = node->type;
	uplink.status = node->state;
	uplink.uplink_subid = node->subid;
	memcpy((void *)msg->data, (void *)&uplink, sizeof(struct uplink_info_report));

	pthread_mutex_lock(&g_mutex);
	nn_socket_send((void *)msg, 
		sizeof(struct modlue_notify_info) + sizeof(struct uplink_info_report)), 
		nn_cli_fd);
	pthread_mutex_unlock(&g_mutex);
	free(msg);
}

struct uplink_info * get_uplink_info_by_ifname(const char *ifname)
{
	int i = 0;
	struct uplink_dev *dev;
	dev = g_conf->dev;

	for(i = 0; i < dev->uplinks; i++)
		if (!strcmp(dev->uplink[i].ifname, ifname)) {
			return dev->uplink[i];
		}
	}
	return NULL;
}


static int netifd_report_handle(const char *device, unsigned int state)
{
	int ret;
	int uplink_type;
	int id = 0;
	struct uplink_info * uplink = NULL;
	struct iface_info_t *node = NULL; 
	
	if (device == NULL)
		return STATUS_NOK;

	// log_dbg("Query interface name=%s", uci_ifname);
	RWLOCK_RLOCK(&g_RWLock);
	node = get_iface_node(device);
	RWLOCK_RUNLOCK(&g_RWLock);

	/* 
	  * Cant not find node from the adsl node table
	  * so this is a new addtion
	  */
	if (state == DEV_EVENT_LINK_UP && node == NULL)
	{
		char value[32] = {0};

		uplink = get_uplink_info_by_ifname(device);
		if (NULL == uplink) {
			return -1;
		}
		
		/* Then, store the adsl into the memory node list */
		RWLOCK_WLOCK(&g_RWLock);
		node = iface_node_insert(device, uplink->type, uplink->subid);
		RWLOCK_WUNLOCK(&g_RWLock);

		/* notify manager module */
		l2_uplink_notify(node);
		
	}
	else if (node != NULL && state != node->state)
	{
		RWLOCK_WLOCK(&g_RWLock);
		iface_node_state_change(node, state);
		RWLOCK_WUNLOCK(&g_RWLock);

		/* notify manager module */
		l2_uplink_notify(node);
	
	}
	return STATUS_OK;
	
}


struct ubus_subscriber ubus_event;
struct ubus_context *ctx = NULL;

static const struct blobmsg_policy new_policy[UBUS_EVENT_MAX] = {
	[UBUS_INTERFACE_EVENT] = { .name = "interface", .type = BLOBMSG_TYPE_STRING },
	[UBUS_DEVICE_NAME_EVENT] = { .name = "devname", .type = BLOBMSG_TYPE_STRING },
	[UBUS_METHOD_EVENT] = { .name = "method", .type = BLOBMSG_TYPE_INT32 },
};

static struct ubus_method heartbeat_obj_method[] =
{
	UBUS_METHOD("ifacenotify", 
				netifd_interface_state, 
				new_policy),
};

static struct ubus_object_type heartbeat_obj_type =
	UBUS_OBJECT_TYPE("mw_iface_forward", heartbeat_obj_method);

static struct ubus_object heartbeat_object = {
	.name = "mw_iface_forward",
	.type = &heartbeat_obj_type,
	.methods = heartbeat_obj_method,
	.n_methods = ARRAY_SIZE(heartbeat_obj_method),
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
	int mark = 0, mark_offset;
	unsigned int msg_type;
	char *interface = NULL;
	char *device = NULL;
	char uci_ifname[32] = {0};
	struct adsl_node_tb * node = NULL; 
	
	blobmsg_parse(new_policy, UBUS_EVENT_MAX, tb, blob_data(msg), blob_len(msg));
	if(!tb[UBUS_INTERFACE_EVENT])
	{
		log_dbg("no data contained");
		return -1;
	}
	
	interface = blobmsg_get_string(tb[UBUS_INTERFACE_EVENT]);
	device = blobmsg_get_string(tb[UBUS_DEVICE_NAME_EVENT]);
	msg_type = blobmsg_get_u32(tb[UBUS_METHOD_EVENT]);
	
	log_dbg("interface=%s, device=%s, method=%d", interface, device, msg_type);

	if(interface == NULL || device == NULL)
		return -1;

	/* dirty set it */
	if (msg_type == DEV_EVENT_REMOVE)
		msg_type = DEV_EVENT_LINK_DOWN;

	netifd_report_handle(device, msg_type);
	
	return 0;
}

void l2_thread_handler(void* arg)
{
	int ret;
	redis_ctx_num = *(int *)arg;

	uloop_init();
	signal(SIGPIPE, SIG_IGN);
	ctx = ubus_connect(NULL);

	if (!ctx) {
		log_dbg("Error ubus_connect");
		return;
	}
	ubus_add_uloop(ctx);

	ret = ubus_add_object(ctx, &heartbeat_object);
	if (ret)
	{
		log_dbg("Failed to add heartbeat_object");
		return;
	}
	ret = ubus_register_subscriber(ctx, &ubus_event);
	if (ret)
	{
		log_dbg("Failed to add heartbeat_object");
		return;
	}
	uloop_run();
	ubus_free(ctx);
	uloop_done();
	return;
}

