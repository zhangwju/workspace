/*
 * =====================================================================================
 *
 *       Filename:  test.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  11/17/2016 08:14:35 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *        Company:  
 *
 * =====================================================================================
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "curlbat.h"

int main(int argc, char **argv)
{
	int i = 0;
	struct timeval tv; 
	curlbat_req_t req[HANDLE_MAX];

	memset(req, 0, sizeof(req));
	printf("multi start......\n");	
	curlbat_cblk_t * curlbat = alloc_init_curlbat(HANDLE_MAX, 30000, 0x0);
	if(curlbat == NULL) {
		printf("ERROR:curlbat %s\n", curlbat);
		return -1;
	}

	strcpy(req[0].url, "http://down.360safe.com/se/360se_setup.exe");
	strcpy(req[1].url, "http://down.360safe.com/360mobilemgr/360box_web.apk");
	strcpy(req[2].url, "http://dl.360safe.com/360DrvMgrInstaller_beta.exe");
	strcpy(req[3].url, "http://down.360safe.com/inst.exe");
	strcpy(req[4].url, "http://down.360safe.com/inst.exe");
	strcpy(req[5].url, "http://down.360safe.com/inst.exe");

	if (getctype_byurltbl(req, 6, (void *)curlbat) < 0) {
		return -1;
	}
	curlbat_multi_show(req, 6);	
	curlbat_release((void*)curlbat);	

	return 0;
}
