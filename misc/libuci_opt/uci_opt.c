/*************************************
 * Filename: uci_opt.c
 * Author: zhangwj
 * Date: 2017-09-22
 * Email: zhangwju@gmail.com
 * Warnning:
 *************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "uci_opt.h"

static int uci_get_value(struct uci_option *o, char *value)
{
	struct uci_element *e;
	char list[2048];
	int list_len = 0;
	int slen = 0;

	switch(o->type) {
	case UCI_TYPE_STRING:
		strcpy(value, o->v.string);
		break;
	case UCI_TYPE_LIST:
		uci_foreach_element(&o->v.list, e) {
			if(list_len >= MAX_UCI_VALUE_LEN - 1) {
				break;
			}
			slen = sprintf(list+list_len, "%s,", e->name);
			list_len += slen;
		}
		memcpy(value, list, list_len);
		value[list_len - 1] = '\0';   
		break;
	default:
		printf("<unknown>\n");
		break;
	}

	return 0;
}

char * uci_get_option(const char *package, const char *section, const char *option)
{
	struct uci_context *ctx;
	struct uci_element *e;
	struct uci_ptr ptr;
	char cmd[256];
	static char data[MAX_UCI_VALUE_LEN];
	
	ctx = uci_alloc_context();
	if (!ctx) {
		fprintf(stderr, "Out of memory\n");
		return NULL;
	}

	sprintf(cmd, "%s.%s.%s", package, section, option);
	if ((uci_lookup_ptr(ctx, &ptr, cmd, true) != UCI_OK) || ptr.o == NULL) {
		uci_free_context(ctx);	
		return NULL;
    }
	uci_get_value(ptr.o, data);
	uci_free_context(ctx);	
	return data;
}

int uci_set_section(const char *package, const char *section_type, const char *section)
{
	struct uci_context *ctx;
    int ret = UCI_OK;
	
	ctx = uci_alloc_context();
	if (!ctx) {
		fprintf(stderr, "Out of memory\n");
		return -1;
	}
	
	struct uci_ptr ptr = {
        .package = package,
        .section = section,
        .value = section_type,
    };
	
	ret = uci_set(ctx, &ptr);
	if (ret == UCI_OK)
		ret = uci_save(ctx, ptr.p);

	uci_free_context(ctx);	
	return ret;
}

int uci_del_option(const char *package, const char *section, const char *option, const char *value, enum uci_option_type type)
{
	struct uci_context *ctx;
    int ret = UCI_OK;
	
	ctx = uci_alloc_context();
	if (!ctx) {
		fprintf(stderr, "Out of memory\n");
		return -1;
	}
	
	struct uci_ptr ptr = {
        .package = package,
        .section = section,
        .option = option,
        .value = value,
    };

	switch(type){
	case UCI_TYPE_STRING:
		ret = uci_delete(ctx, &ptr);
		break;
	case UCI_TYPE_LIST:
		ret = uci_del_list(ctx, &ptr);
		break;
	default:
		uci_free_context(ctx);
		return -1;
	}

	if (ret == UCI_OK)
		ret = uci_save(ctx, ptr.p);

	uci_free_context(ctx);
	return ret;
}

int uci_set_option(const char *package, const char *section, const char *option, const char *value, enum uci_option_type type)
{
	struct uci_context *ctx;
    int ret = UCI_OK;
	
	ctx = uci_alloc_context();
	if (!ctx) {
		fprintf(stderr, "Out of memory\n");
		return -1;
	}
	
	struct uci_ptr ptr = {
        .package = package,
        .section = section,
        .option = option,
        .value = value,
    };
	
	switch(type){
	case UCI_TYPE_STRING:
		ret = uci_set(ctx, &ptr);
		break;
	case UCI_TYPE_LIST:
		ret = uci_add_list(ctx, &ptr);
		break;
	default:
		uci_free_context(ctx);
		return -1;
	}

	if (ret == UCI_OK)
		ret = uci_save(ctx, ptr.p);

	uci_free_context(ctx);
	return ret;
}

int uci_opt_commit(char *package)
{
	struct uci_context *ctx;

	struct uci_ptr ptr = {
		.package = package,
	};
	
	ctx = uci_alloc_context();
	if(!ctx) {
		fprintf(stderr, "Out of memory\n");
		return -1;	
	}

	if (uci_lookup_ptr(ctx, &ptr, package, true) != UCI_OK) {
		uci_perror(ctx, "uci_lookup_ptr");
		uci_free_context(ctx);
		return -1;
    }	
	uci_commit(ctx, &ptr.p, false);
	
	if (ptr.p)
		uci_unload(ctx, ptr.p);
	uci_free_context(ctx);

	return 0;
}
