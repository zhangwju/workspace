/***************************************************************
 *    Filename: test.c
 *    Description: check ip isVaildIp 
 *      Created:  11/16/2016 02:28:55 AM
 *      Revision:  none
 *      Compiler:  gcc
 *      Author:	zhangwj
 ***************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

bool isVaildIp(const char *ip)
{
	int dots = 0; /*字符.的个数*/
	int setions = 0; /*ip每一部分总和（0-255）*/ 

	if (NULL == ip || *ip == '.') {
		return false;
	}

	while (*ip) {
		if (*ip == '.') {
			dots ++;
			if (setions >= 0 && setions <= 255) { /*检查ip是否合法*/
				setions = 0;
				ip++;
				continue;
			}
			return false;
		} 
		else if (*ip >= '0' && *ip <= '9') { /*判断是不是数字*/
			setions = setions * 10 + (*ip - '0'); /*求每一段总和*/
		} else { 
			return false;
		}
		ip++;	
	}

	if (setions >= 0 && setions <= 255) {
		if (dots == 3) {
			return true;
		}
	}

	return false;
}

void help()
{
	printf("Usage: ./test <ip str>\n");
	exit(0);
}

int main(int argc, char **argv)
{
	if (argc != 2) {
		help();	
	}

	if (isVaildIp(argv[1])) {
		printf("Is Vaild Ip-->[%s]\n", argv[1]);
	} else {
		printf("Is Invalid Ip-->[%s]\n", argv[1]);
	}

	return 0;
}
