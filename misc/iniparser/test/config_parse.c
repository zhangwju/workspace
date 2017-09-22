/******************************************
 * Filename: iniparse
 * Author: zhangwj
 * Date: 2017-09-22
 * Email: zhangwju@gmail.com
 * Warnning:
 *******************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>  

char *getstr_bylabel(char *str, char *label, char *value)
{
	char *ptrstart;
	char *ptrret = NULL;
	int offset = 0;
	
	ptrstart = strstr(str, label);
	if(ptrstart) {
		ptrstart += strlen(label);
		offset = 0;
		do {
			switch(ptrstart[offset]) {
			case 0:
				break;
			case ' ':
			case '=':
				offset += 1;
				continue;
			default:
				break;
			}
			break;
		}while(1);
		
		ptrstart += offset;
		offset = 0;
		do {
			if((ptrstart[offset] > 0x20) && (ptrstart[offset] < 0x80)) {
				offset += 1;
				continue;
			}
			memcpy(value, ptrstart, offset);
			value[offset] = 0;
			ptrret = value;
			break;
		}while(1);
		
	}
	return ptrret;
}

int get_file_size(char* filename)  
{  
	struct stat statbuf;
	int size;

	stat(filename,&statbuf);  
	size=statbuf.st_size;  

	return size;  
}

int main(int argc, char **argv)
{
	char value[128];
	int fsize = 0;
	char *fbuff;
	FILE *fp;
	int labels;
	int i = 0;

	if(argc < 3) {
		printf("Usage: %s <config> <label1> <label2> ...\n", argv[0]);
		exit(0);
	}

	fp = fopen(argv[1], "r");
	if(NULL == fp) {
		return -1;
	}
	
	fsize = get_file_size(argv[1]);
	fbuff = (char *)malloc(fsize + 1);
	if(fbuff == NULL) {
		fclose(fp);
		return -1;
	}
	fread(fbuff, fsize, 1, fp);
	fclose(fp);

	labels = argc;
	for(i = 2; i < labels; i++) {
		printf("%s=%s\n", argv[i], getstr_bylabel(fbuff, argv[i], value));
	}
	free(fbuff);	

	return 0;	
}
