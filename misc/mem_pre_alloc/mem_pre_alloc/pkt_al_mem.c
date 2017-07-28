#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <linux/mman.h>
#include <unistd.h>
#include <errno.h>
#include "pa_mem_struct.h"

int pa_buffer_flow_init(struct pa_flow_table *flow)
{
	uint32_t i;
	arr_hlist_head_t *head;

	head = &flow->free;
	ARR_INIT_HLIST_HEAD(head);
	for (i = 1; i < PA_FLOW_TABLE_SIZE + 1; i ++) {
		arr_hlist_add_head(head, flow->entry, i, node);
		//(flow->entry + (i - 1))->idx = i;
		printf("init:%p\n", (flow->entry + i));
	}
	flow->free_num = PA_FLOW_TABLE_SIZE;
	return 0;
}

void *pa_buffer_flow_alloc(struct pa_flow_table *flow)
{
	uint32_t i, idx;
	arr_hlist_head_t *head;

	head = &flow->free;
	if (!arr_hlist_empty(head)) {
		idx = head->first;
		arr_hlist_remove(head, flow->entry, idx, node);
		flow->free_num--;
		//(flow->entry + (idx - 1))->idx = idx;
		printf("alloc:%p\n", (flow->entry + (idx)));
		return (flow->entry + (idx));
	}
	return NULL;
}

int pa_buffer_flow_free(struct pa_flow_table *flow, struct pa_flow_entry *entry)
{
	uint32_t i, idx;
	arr_hlist_head_t *head;

	if(entry == NULL) {
		return -1;
	}

	head = &flow->free;
	//idx = entry->idx;
	idx = ((char *)entry - (char *)flow->entry)/sizeof(struct pa_flow_entry) + 1;
	arr_hlist_add_tail(head, flow->entry, idx, node);
	flow->free_num++;
	return 0;
}

int pa_buffer_flow_free_walk(struct pa_flow_table *flow)
{
	int idx;
	arr_hlist_head_t *head;

	printf("======walk begin\n");
	head = &flow->free;
	
	arr_hlist_for_each(idx, head, flow->entry, node)
	{
		printf("walk[%d]:%p\n", idx, (flow->entry + (idx)));
	}
	printf("======walk end, num:%d\n", flow->free_num);
}

/**buf 存的是映射内存的首地址*/
int pa_buffer_init(void **buf)
{
	size_t size = sizeof(struct pa_mem_struct);

//	*buf = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_NONBLOCK | MAP_HUGETLB, -1, 0); 
	//size malloc memory size (flow struct)
	*buf = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_NONBLOCK , -1, 0);
	if (*buf == MAP_FAILED) {
		*buf = NULL;
		perror("map mem");
		printf("Error, mmem errno:%d\n", errno);
		return -1;
	}
	
	memset(*buf, 0, size); 
	return 0;
}

int pa_buffer_deinit(void **buf)
{
	size_t size = sizeof(struct pa_mem_struct);

	if (*buf == NULL) {
		return -1;
	}
	munmap(*buf, size); 
	*buf = NULL;
	return 0;
}

