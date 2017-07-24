/**************************************** 
 * Filename: iface_info.c
 * Author: zhangwj
 * Date: 2017-07-24
 * Warnning:
 ****************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "iface_info.h"

static struct iface_info_t *iface_node_header = NULL;

struct iface_info_t * iface_node_insert(const char *ifname, uint type, uint subid)
{
	struct iface_info_t *find = NULL;
	struct iface_info_t *cur = NULL;
	
	if (NULL == ifname) {
		return NULL;
	}

	cur = (struct iface_info_t *)malloc(sizeof(struct iface_info_t));
	if(NULL == cur){
		return NULL;
	}

	memset(cur, 0, sizeof(struct if_info_t));
	strncpy(cur->ifname, ifname, IFNAMESZ);
	cur->state = DEV_LINK_UP;
	cur->type = type;
	cur->subid = subid;
	cur->next = NULL;

	if (NULL == iface_node_header){
		iface_node_header = cur;
	} else {
		find = iface_node_header;
		while(find->next) {
			find = find->next;
		}
		find->next = cur;
	}
	
	return cur;
}

struct get_iface_node(const char *ifname)
{
	struct iface_info_t *node = NULL;
	
	if(NULL == ifname) {
		return NULL;
	}

	node = iface_node_header;
	while(node) {
		if(!strcmp(node->ifname, ifname)){
			return node;
		}
		node = node->next;
	}

	return NULL;
}

int iface_node_state_change(struct iface_info_t *if_node,  int state)
{
	if(NULL != if_node) {
		if_node->state = state;
		return 0;
	}

	return -1;
}

void iface_node_release(void)
{
	struct iface_info_t *prev = NULL;
	struct iface_info_t *crnt = iface_node_header;
	
	while (NULL != crnt)
	{
		prev = crnt;
		crnt = crnt->next;
		free(prev);
		prev = NULL;
	}
}

int get_iface_node_list(char dev_lst[][16])
{
	struct iface_info_t *curt = iface_node_header;
	int cnt = 0;
	
	while(curt) {
		strcpy(dev_lst[cnt], curt->ifname);
		curt = curt->next;
		cnt++;
	}
	
	return cnt;
}

