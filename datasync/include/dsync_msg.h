#ifndef __DSYNC_MSG_H__
#define __DSYNC_MSG_H__

#include "common.h"
#include "dsync.h"

#define MAX_HDRLEN		(512)
#define MAX_PAYLOAD		(4 << 10) /* 4k */
#define MAX_MSGLEN		(MAX_HDRLEN + MAX_PAYLOAD)
#define MSG_SIGN		"dsyn"
#define MSG_SIGNLEN		4

typedef struct _dsync_msghdr {
	char sign[MSG_SIGNLEN];	
	ushort modid;
	ushort acctype;
	ushort usrsv;
	ushort usrsv1;
	unsigned int msglen;
}DSYNC_MSGHDR, *PDSYNC_MSGHDR;

#define	mret usrsv1

enum {
	MSGERR_OK=0,		/* success */
	MSGERR_SIGN,		/* SIGN error */
	MSGERR_MODID,		/* module id invalid */
	MSGERR_ACCTYPE,		/* access type error */
	MSGERR_DSYNC,		/* data sync error */
	MSGERR_MAX
};

#define FILL_DSYNCHDR(pmsghdr,mod_id,acctyp,datlen,us,us1) \
{\
	memcpy((pmsghdr)->sign, MSG_SIGN, MSG_SIGNLEN);\
	(pmsghdr)->modid = (uchar)mod_id;\
	(pmsghdr)->acctype = (uchar)acctyp;\
	(pmsghdr)->usrsv = (ushort)us;\
	(pmsghdr)->usrsv1 = (ushort)us1;\
	(pmsghdr)->msglen = (uint)datlen;\
}

#define MODID_ISVALID(modid) 		((modid >= 0) && (modid < MAX_DSYNCMOD))
#define ACCTYPE_ISVALID(acctype)	((acctype >=0) && (acctype < acttype_max))	

/*
 * ifname: dasync server listen interface 
 */
extern int init_dsync_server(uint srcaddr, ushort port, const char *ifname);
extern int accept_dmsg(int sock); 

extern int init_dsync_client(uint addr, ushort dstport);

extern int recv_dmsg(int sock, void *buf, int len);
extern int send_dmsg(int sock, void *buf, int len);

#endif //__DSYNC_MSG_H__ 
