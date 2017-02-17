#ifndef __CURLBAT_H_
#define __CURLBAT_H_

#include <curl/curl.h>	

enum respon_type{
	RSTAT=0, /*respon status*/
	RDATE,
	RCONN,
	CTYPE,
	CLEN,
	CRANGE,
	CENCOD,
	CACHECTRL,
	SETCOOKIW,
	REPON_MAX
};

#define STATUS_MASK			(1<<RSTAT)
#define RDATE_MASK			(1<<RDATE)
#define RCONN_MASK			(1<<RCONN)
#define CTYPE_MASK			(1<<CTYPE)
#define CLEN_MASK				(1<<CLEN)
#define CRANGE_MASK			(1<<CRANGE)
#define CENCODE_MASK		(1<<CENCOD)
#define CACHECTRL_MASK	(1<<CACHECTRL)
#define SETCOOKIW_MASK	(1<<SETCOOKIW)

/*
static int sizemask[] = {0x0, 0x0, 0x1, 0x0, 0x3, \
															0x0, 0x0, 0x0, 0x7};
#define POSTBL_SIZE			((REPON_MAX+sizemask[sizeof(long)])&(^sizemask[sizeof(long)]))
*/

#define POSTBL_SIZE				(((REPON_MAX+sizeof(long)-1)/sizeof(long))*sizeof(long))


//fill with curl arguments
typedef struct _curlbat_handle_s
{
	CURL *curl;
}curlbat_handle_t;

#ifndef uchar
#define uchar unsigned char
#endif

#ifndef ushort
#define ushort unsigned short
#endif

#ifndef ulong
#define ulong unsigned long
#endif

#define REQLEN_MAX				1480
#define RESPON_MAX				4096
#define HANDLE_MAX				512
#define TYPE_MAX				128

#define ERROR					(-1)

//request infor structure
typedef struct _curlbat_reqest_s
{
	ushort urllen;				/* url length*/
	ushort reslen;				/* http header length*/
	ulong length;				/* content-length*/
	int retval;					/* return value */
	int status;					/* reponse code */
	int index;					/* curlbat index */
	char type[TYPE_MAX];		/* content-type */
	char url[REQLEN_MAX];		/* handle url*/
	char respon[RESPON_MAX];    /* http reponse */
	char pos[POSTBL_SIZE];		/* reserve capacity */
}curlbat_req_t;

//curl contorl block structure
typedef struct _curlbat_ctrlblk_s
{
	int maxcnt;								/*Number of per processing */
	int maxusec;							/* time consuming of per processing */
	int reponmask; 							/* reserve capacity */
	CURLM * mult_handle;					/* multi handle*/
	curlbat_handle_t handles[HANDLE_MAX];	/* handle */
}curlbat_cblk_t;

/* ******** */
extern void * alloc_init_curlbat(int maxreqcnt, int max_msec, int reponmask);
extern void curlbat_release(void * curlbat_info);
extern int getctype_byurltbl(curlbat_req_t * batreq, int reqcnt, void * curlbat_info);

#endif /*__CURLBAT_H_*/
