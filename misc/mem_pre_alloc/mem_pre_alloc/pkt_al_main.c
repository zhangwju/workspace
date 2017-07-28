#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>
#include <errno.h>

#define NETMAP_WITH_LIBS
#include "pa_mem_struct.h"

int g_pa_reload_config = 0;
struct pa_mem_struct *g_pa_mem;

int buf_test()
{
	void *p;
	/*init malloc memory*/
	pa_buffer_init((void **)&g_pa_mem);

	pa_buffer_flow_init(&(g_pa_mem->flow));
	//pa_buffer_arr_init(&(g_pa_mem->flow), struct pa_flow_table);
	pa_buffer_flow_free_walk(&(g_pa_mem->flow));

	pa_buffer_flow_alloc(&(g_pa_mem->flow));
	pa_buffer_flow_free_walk(&(g_pa_mem->flow));

	p = pa_buffer_flow_alloc(&(g_pa_mem->flow));
	pa_buffer_flow_free_walk(&(g_pa_mem->flow));

	pa_buffer_flow_alloc(&(g_pa_mem->flow));
	pa_buffer_flow_free_walk(&(g_pa_mem->flow));

	pa_buffer_flow_free(&(g_pa_mem->flow), p);
	pa_buffer_flow_free_walk(&(g_pa_mem->flow));

	pa_buffer_deinit((void **)&g_pa_mem);
	return 0;
}

int main()
{
	buf_test();

	return 0;
}

