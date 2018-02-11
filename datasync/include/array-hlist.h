/*
 * arr-hlist.h This HLIST base on ARRAY.
 */

#ifndef __ARR_HLIST_H__
#define __ARR_HLIST_H__
#include <stdint.h>

typedef struct arr_hlist_node {
	uint32_t	next;
	uint32_t	prev;
} arr_hlist_node_t;

typedef struct arr_hlist_head {
	uint32_t	first;
    uint32_t	tail;
} arr_hlist_head_t;

#define ARR_INIT_HLIST_HEAD(head) do{ \
    (head)->first = 0;                 \
    (head)->tail  = 0;                 \
}while(0)

#define ARR_INIT_HLIST_NODE(node) do{ \
    (node)->next = 0;                  \
    (node)->prev = 0;                  \
}while(0)

#define arr_hlist_empty(head)    (0 == (head)->first)
#define arr_hlist_pos_empty(pos) (0 == pos)
#define arr_hlist_pos_valid(pos) (0 != pos)

#define arr_hlist_add_head(head, table, idx, node) { \
       uint32_t __next = ((arr_hlist_head_t*)(head))->first; \
       (table)[idx].node.prev = 0; \
       (table)[idx].node.next = __next; \
       if (__next) \
           (table)[__next].node.prev = idx; \
       else \
           ((arr_hlist_head_t*)(head))->tail = idx;   \
       ((arr_hlist_head_t*)(head))->first = idx; \
}

#define arr_hlist_add_tail(head, table, idx, node) { \
    uint32_t __tail = ((arr_hlist_head_t*)(head))->tail; \
	(table)[idx].node.prev = __tail; \
	(table)[idx].node.next = 0; \
	if (__tail) \
		(table)[__tail].node.next = idx; \
	else \
		((arr_hlist_head_t*)(head))->first = idx; \
	((arr_hlist_head_t*)(head))->tail = idx; \
}

#define arr_hlist_remove(head, table, idx, node) { \
	uint32_t __prev, __next; \
	__prev = (table)[idx].node.prev; \
	__next = (table)[idx].node.next; \
	if (__prev) \
		(table)[__prev].node.next = __next; \
	else \
		((arr_hlist_head_t*)(head))->first = __next; \
	if (__next) \
		(table)[__next].node.prev = __prev; \
	else \
        ((arr_hlist_head_t*)(head))->tail = __prev; \
	(table)[idx].node.prev = (table)[idx].node.next = 0; \
} 

#define arr_hlist_unhashed(head, table, idx, node) \
	(!(((table)[idx].node.prev) || \
	((arr_hlist_head_t*)(head))->first == idx))

#define arr_hlist_for_each(idx, head, table, node) \
	for (idx = ((arr_hlist_head_t*)(head))->first; idx ; \
	     idx = (table)[idx].node.next)

#define arr_hlist_for_each_safe(idx, n, head, table, node) \
	for (idx = ((arr_hlist_head_t*)(head))->first; \
            idx && ({n = (table)[idx].node.next; 1; }); idx = n)

#define arr_index_in_table(table, entry) ((entry) - (typeof (entry))(table))

#endif /* __ARR_HLIST_H__ */
