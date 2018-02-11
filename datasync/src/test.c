#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>

#include "dsync.h"

#ifndef IFNAMESZ
#define IFNAMESZ	32
#endif

struct dsync_demoinfo {
	char name[IFNAMESZ];
	int demoid;
	int data;
};

int dsync_demoadd(unsigned char *ptrbuf, int bufarg,  unsigned long arg)
{
	struct dsync_demoinfo *demo = (struct dsync_demoinfo *)ptrbuf;
	log_dbg("dsync_demoadd, data:%d", demo->data);

	return 0;
}

int dsync_demodel(unsigned char *ptrbuf, int bufarg,  unsigned long arg)
{
	struct dsync_demoinfo *demo = (struct dsync_demoinfo *)ptrbuf;
	log_dbg("dsync_demodel, data:%d", demo->data);

	return 0;
}

static struct dsync_demoinfo msginfo = {
	.name = "dsync_msginfo",
	.demoid = 110,
};

static DSYNCPROC_MODULE pmodule = {
	.name = "dsync_demo",
	.modid = 1,
	.addcall = dsync_demoadd,
	.delcall = dsync_demodel
};

int main(int argc, char **argv)
{
	int i, ret;
	int type;
	int count;
	uint dstaddr;
	DSYNC_MINFO dsync;

	if (argc < 2) {
		fprintf(stderr, "Usage: %s <0/1> <ipaddr>\n", argv[0]);
		exit(0);
	}

	ret = dsync_init(&dsync, 9574, NULL);
	if (ret < 0) {
		log_dbg("dsync_init faliure");
		return -1;
	}

	type = atoi(argv[1]);
	switch(type) {
	case 0: /* dsync server */
		ret = register_syncmod(&dsync, &pmodule);
		if (ret < 0) {
			log_dbg("register %s moudle failure", pmodule.name);
			return -1;
		}
		do {
			sleep(5);
		} while(1);
		break;
	case 1: /* dsync client */
		if(!argv[2]) {
			log_dbg("dstaddr is NULL");
			return -1;
		}
		
		count = 100;
		if (argv[3]) {
			count = atoi(argv[3]);
		}

		dstaddr = ntohl(inet_addr(argv[2]));
		for (i = 0; i < count; i++) {
			msginfo.data = i;
			ret = do_notify(dstaddr, 1, 0, (void *)&msginfo, sizeof(msginfo), 0, &dsync);
			if (ret == 1) {
				do_notify(dstaddr, 1, 0, (void *)&msginfo, sizeof(msginfo), 0, &dsync);
			}
			do_notify(dstaddr, 1, 1, (void *)&msginfo, sizeof(msginfo), 0, &dsync);
		}	
		do {
			sleep(2);
		} while(1);
		break;
	default:
		log_dbg("unknown type");
		break;
	}

	dsync_release(&dsync);

	return 0;
}
