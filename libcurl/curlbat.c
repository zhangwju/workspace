/***************************************************************
 *    Filename:  curlbat.c
 *    Description:  
 *      Created:  11/16/2016 02:28:55 AM
 *      Revision:  none
 *      Compiler:  gcc
 *      Author:	zhangwj
 ***************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/select.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>
#include "curlbat.h"

#define OPEN_DEBUGINFO 1

#ifdef OPEN_DEBUGINFO
#define CU_WARN(fmt, args...) \
    do { \
        printf("[curl-warn]:");\
        printf(fmt "\n", ##args); \
    } while (0)

#define CU_DEBUG(fmt, args...) \
    do { \
        printf("[curl-trace]:");\
        printf(fmt "\n", ##args); \
    } while (0)

#else

#define CU_WARN(fmt, args...)      
#define CU_DEBUG(fmt, args...)     
#endif

#define CU_ERROR(fmt, args...) \
    do { \
        printf("[curl-error]:");\
        printf(fmt "\n", ##args); \
    }while(0)



static size_t
write_header_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
#if 0	
	size_t realsize;
	int slen;
	
	realsize = size * nmemb;
	slen = realsize;
	
	curlbat_req_t *mem = (curlbat_req_t *)userp;
	(mem->reslen + realsize >= RESPON_MAX) ?
			(slen = RESPON_MAX - mem->reslen) : slen;
	
	if (slen > 0) {
	    memcpy(&(mem->respon[mem->reslen]), contents, slen);
	    mem->reslen += slen;
	    mem->retval += slen;
	    mem->respon[mem->reslen] = 0;
	}
#endif   
	
	return (size_t)size * nmemb; 
}

static void curlbat_data_init(curlbat_req_t * batreq, int reqcnt)
{
	int i = 0;

	for (i = 0; i < reqcnt; i++) {
		batreq[i].reslen = 0;   
		batreq[i].retval = 0;
		batreq[i].length = 0;
		batreq[i].status = 0;
		batreq[i].index = i;
	}

	return;
}

static void assble_mult_handle(curlbat_req_t * batreq, int reqcnt, void * curlbat_info)
{
	int cnt, i = 0;
	curlbat_cblk_t *curlbat = NULL;

	curlbat = (curlbat_cblk_t *)curlbat_info;
	
	for (i = 0; i < reqcnt; i++) {
		curlbat->handles[i].curl = curl_easy_init();	
		curl_easy_setopt(curlbat->handles[i].curl, CURLOPT_URL, batreq[i].url);
		curl_easy_setopt(curlbat->handles[i].curl, CURLOPT_PRIVATE, &(batreq[i].index));  
		curl_easy_setopt(curlbat->handles[i].curl, CURLOPT_NOSIGNAL, true);  
		curl_easy_setopt(curlbat->handles[i].curl, CURLOPT_TIMEOUT_MS , 2000);
        curl_easy_setopt(curlbat->handles[i].curl, CURLOPT_NOBODY, 1L);  
        curl_easy_setopt(curlbat->handles[i].curl, CURLOPT_HEADERFUNCTION, write_header_callback);  
        curl_easy_setopt(curlbat->handles[i].curl, CURLOPT_HEADERDATA,(void *)&batreq[i]);
        curl_multi_add_handle(curlbat->mult_handle, curlbat->handles[i].curl);		
	}
	return;
}

static int curl_multi_handle_remove(int reqcnt, void * curlbat_info)
{
	int cnt, i = 0;
	curlbat_cblk_t *curlbat = NULL;
	
	curlbat = (curlbat_cblk_t *)curlbat_info;

	for (i = 0; i < reqcnt; i++) {
		curl_multi_remove_handle(curlbat->mult_handle, curlbat->handles[i].curl);
		curl_easy_cleanup(curlbat->handles[i].curl);
	}
	return 0;	
}

static int curl_multi_handle_info(curlbat_req_t * batreq, void * curlbat_info)
{
	int mqs;
	int *index = NULL;
	long http_status = 0;
	double size = 0.0;
	char *type = NULL;
	curlbat_cblk_t *curlbat = NULL;
	CURL *curl = NULL;
	CURLMsg *msg = NULL;
	CURLcode res;
	
	curlbat = (curlbat_cblk_t *)curlbat_info;
	while ((msg = curl_multi_info_read(curlbat->mult_handle, &mqs))) {

		if (msg->msg == CURLMSG_DONE) {
			res = curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &index);
			if (res != CURLE_OK || NULL == index) {
				return -1;
			}

			res = curl_easy_getinfo(msg->easy_handle, CURLINFO_RESPONSE_CODE , &http_status);
			if (res != CURLE_OK) {
				return -1;
			}
			batreq[*index].status = http_status;

			res = curl_easy_getinfo(msg->easy_handle, CURLINFO_CONTENT_TYPE, &type); /*judge source type (video file application ....)*/
			if (res != CURLE_OK || NULL == type) {   
					return -1; 
			}
			strncpy(batreq[*index].type, type, TYPE_MAX);

			if (http_status == 200) {
				res = curl_easy_getinfo(msg->easy_handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &size); /*get file size*/ 
				if ((res != CURLE_OK) || (size <= 0.0)) { 
					return -1; 
				}
				batreq[*index].length = (unsigned int)size;
			}
		}
	}

	return 0;
}

static int curl_multi_select_handle(curlbat_req_t * batreq, int reqcnt, void * curlbat_info)
{
	int handle_count = -1;
	int max_fd;
	int ret;
	struct timeval tv; 
	long timeout;
	curlbat_cblk_t *curlbat = NULL;
	fd_set fd_read, fd_write;
	fd_set fd_except;

	curlbat = (curlbat_cblk_t *)curlbat_info;

	while(handle_count) {	
		curl_multi_perform(curlbat->mult_handle, &handle_count);

		if (handle_count) {

			FD_ZERO(&fd_read);
			FD_ZERO(&fd_write);
			FD_ZERO(&fd_except);

			if (CURLE_OK != curl_multi_fdset(curlbat->mult_handle, &fd_read, 
				&fd_write, &fd_except, &max_fd)) {
				CU_ERROR("ERROR:curl multi fdset..");
				return -1;
			}
			if (CURLE_OK != curl_multi_timeout(curlbat->mult_handle, &timeout)) {
				CU_ERROR("ERROR: curl multi timeout...");
				return 0;
			}

			if (timeout == -1 || timeout > curlbat->maxusec / 1000) {
				timeout = curlbat->maxusec / 1000;
			}
		
			if (max_fd == -1) {
				usleep(timeout / 1000);
			}
			
			tv.tv_sec = timeout / 1000;			/* set timeout*/
			tv.tv_usec = (timeout % 1000) * 1000;

			ret = select(max_fd + 1, &fd_read, &fd_write, &fd_except, &tv);	
			switch (ret) {
				case -1:
					CU_ERROR("ERROR: Select failure!!");
					return -1;		
					break;
				case 0:
				default:
					do {
					} while( CURLM_CALL_MULTI_PERFORM == 
							curl_multi_perform(curlbat->mult_handle, &handle_count));
					break;
			}
		}
		curl_multi_handle_info(batreq, curlbat_info);
	}
	curl_multi_handle_remove(reqcnt, curlbat_info);

	return 0;
}

int getctype_byurltbl(curlbat_req_t * batreq, int reqcnt, void * curlbat_info)
{
	int ret = 0;

	if (batreq == NULL || curlbat_info == NULL || reqcnt < 0) {
		return -1;
	}
	/* init curbat_req_t value */
	curlbat_data_init(batreq, reqcnt);

	/* add curl handle of multi handles*/
	assble_mult_handle(batreq, reqcnt, curlbat_info); 	

	return curl_multi_select_handle(batreq, reqcnt, curlbat_info); 				
}

void *alloc_init_curlbat(int maxreqcnt, int max_msec, int reponmask)
{
	int i = 0;
	curlbat_cblk_t *curlbat = NULL;
	CURLM *mult_handle = NULL;

	if (maxreqcnt > HANDLE_MAX) {
		CU_ERROR("ERROR: Multi Handle[%d], Request Count[%d]", HANDLE_MAX, maxreqcnt);
		return NULL;
	}
	curlbat = (curlbat_cblk_t *)malloc(maxreqcnt * sizeof(curlbat_cblk_t));
	if(curlbat == NULL) {
		CU_ERROR("curlbat_cblk_t malloc error");
		return NULL;
	}

	/* init multi handles */
	mult_handle = curl_multi_init();
	if (mult_handle == NULL) {
		CU_ERROR("Init mult handler failure");
		curl_global_cleanup();
		return NULL;
	}
	curlbat->mult_handle = mult_handle;
	curlbat->maxcnt = maxreqcnt;
	curlbat->maxusec = max_msec * 1000;
	curlbat->reponmask = reponmask;

	return (void *)curlbat;
}

void curlbat_release(void * curlbat_info)
{
	int i = 0;
	curlbat_cblk_t *curlbat = NULL;

	if (curlbat_info == NULL) {
		CU_ERROR("Cann't release, curlbat_info->[%s]", curlbat_info);	
		return;	
	}

	curlbat = (curlbat_cblk_t *)curlbat_info;
	
	curl_multi_cleanup(curlbat->mult_handle);

	free(curlbat_info);	
	curlbat_info = NULL;
	CU_DEBUG("Release success");
}

void curlbat_multi_show(curlbat_req_t * batreq, int reqcnt)
{
	int i = 0;

	if(batreq == NULL) {
		CU_ERROR("Don't Context show....");
		return;
	}

	for(i = 0; i < reqcnt; i++) {
		printf("--------------Req: %d--------------\n", i);
		printf("request:%s\n", batreq[i].url);
		printf("reponse:%ld\n", batreq[i].status);
		printf("content-length:%u\n", batreq[i].length);
		printf("content-type:%s\n", batreq[i].type);
		printf("content:%s\n", batreq[i].respon);
	}	
}

