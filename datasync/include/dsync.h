#ifndef __DSYNC_H__
#define __DSYNC_H__

#include "common.h"
#include "ring_queue.h"

#define MAX_EVENTS  	256
#define MAX_DSYNCMOD	32
#define MAX_ACCEPTS		32
#define MAX_CLIENTS		32
#define MAX_THREADS		4

#define RQUEUE_SIZE		1024

typedef int (* func_dsync_unified)(unsigned char *ptrbuf, int bufarg,  unsigned long arg);
typedef int (* func_dsync_newup)(unsigned int addr, unsigned long arg);

enum {
	act_add = 0,
	act_del,
	act_dump,
	act_clear,
	act_change,
	act_unified,
	acttype_max
};

enum  {
	notify_ok = 0,
	nofify_newup
};

#define CLIENT_NEWUP	0x8000
typedef struct dsyncproc_module {
	char name[32]; /* module name */
	int modid;		/* module identity */
	short valid;
	short srsv;  /* reserve option */
	unsigned long callarg;
	
	func_dsync_unified dsync_call[acttype_max];	
}DSYNCPROC_MODULE, *PDSYNCPROC_MODULE;

#define addcall dsync_call[act_add]
#define delcall dsync_call[act_del]
#define dumpcall dsync_call[act_dump]
#define clearcall dsync_call[act_clear]
#define changecall dsync_call[act_change] 
#define unifiedcall dsync_call[act_unified]

typedef struct smsg_info {
	int sock;
	ushort usrsv;
	ushort use;
	uint errcnt;
	uint recv_ok;
	rqueue_t *rcv_queue;	
}SMSG_INFO, *PSMSG_INFO;

typedef struct cmsg_info {
	int sock;
	int use;
	uint errcnt;
	uint send_ok;
	uint addr;
	rqueue_t *delay_queue;
}CMSG_IFNO, *PCMSG_INFO;

typedef struct dsync_minfo {
	/* server */
	char listen[IFNAMESZ];		/* listen interface */
	ushort sport;				/* listen port */
	ushort usrsv;				/* ushort reserve */
	int servsock;
	int epfd;					/* epoll fd */
	int acccnt;					/* access count */
	int accmask;				/* access mask */
	SMSG_INFO accepts[MAX_CLIENTS];

	/* dsync module callback */
	DSYNCPROC_MODULE dsyncprocs[MAX_DSYNCMOD];

	
	/* task contrl */
	short exec_threads;			/* 1:default */
	short delay_threads;		/* 1:default */
	int irsv;					/* int reserve */
	
	int server_thread_ctrl;
	int recvmsg_thread_ctrl;
	int execdsync_thread_ctrl;
	int delay_thread_ctrl;
	
	pthread_t pid_server_dsync;
	pthread_t pid_recv_dsync;
	pthread_t pid_exec_dsync[MAX_THREADS];
	pthread_t pid_delay_dsync[MAX_THREADS];
	
	/* client */
	int notify_cnt;
	int notify_mask;
	func_dsync_newup dsync_newup;	
	CMSG_IFNO clients[MAX_CLIENTS];
}DSYNC_MINFO, *PDSYNC_MINFO;

extern int dsync_init(PDSYNC_MINFO pdsync, ushort sport, const char *ifname);
extern int dsync_release(PDSYNC_MINFO pdsync);

extern int register_syncmod(PDSYNC_MINFO pdsync, PDSYNCPROC_MODULE pmodule);

extern int do_notify(uint addr, int modid, int acttype, void *data, int dlen, int flag, PDSYNC_MINFO pdsync);
#endif //__DSYNC_H__
