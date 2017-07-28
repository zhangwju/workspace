#ifndef __PA_MEM_STRUCT_H__
#define __PA_MEM_STRUCT_H__
#include "array-hlist.h"


#ifndef	PA_FLOW_TABLE_SIZE
//#define PA_FLOW_TABLE_SIZE		(1<<29) /* 2 hundred million */
#define PA_FLOW_TABLE_SIZE		(1<<4) /* 2 hundred million */
#endif

#ifndef	PA_FLOW_HASH_SIZE
//#define PA_FLOW_HASH_SIZE		((1<<28) - 1)
#define PA_FLOW_HASH_SIZE		((1<<3) - 1)
#endif

/* struct info */
struct pa_flow_entry {
	arr_hlist_node_t node;		/* list node */
	uint32_t idx;				/* list idx */
	uint32_t sip;
	uint32_t dip;
	uint16_t sport;
	uint16_t dport;
};

/* table info */
struct pa_flow_table {
	arr_hlist_head_t hash[PA_FLOW_HASH_SIZE];
	struct pa_flow_entry entry[PA_FLOW_TABLE_SIZE + 1];
	arr_hlist_head_t free;
	uint32_t free_num;
};

/* global struct */
struct pa_mem_struct {
	struct pa_flow_table flow;
};


int pa_buffer_init(void **buf);
int pa_buffer_deinit(void **buf);
int pa_buffer_flow_init(struct pa_flow_table *flow);
void *pa_buffer_flow_alloc(struct pa_flow_table *flow);
int pa_buffer_flow_free(struct pa_flow_table *flow, struct pa_flow_entry *entry);
int pa_buffer_flow_free_walk(struct pa_flow_table *flow);

#endif /* __PA_MEM_STRUCT_H__ */
