#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

enum {
	L_STRIP = 1,
	R_STRIP
};

static char * _strip(char *str, const char c, const int type)
{
	int slen = 0;
	char *p = NULL;
	char *s = NULL;
	
	if((slen = strlen(str)) == 0) {
		return NULL;
	}
	
	p = str;
	s = str;
	if (c == ' ') {
		if(type == L_STRIP) {
			while(*s && isspace(*s)) {
				s++;
			}
			p = s;	
		} else {
			s = s + slen - 1;
			while(*s && isspace(*s)) {
				s--;
			}
			*(s + 1) = '\0';
		}
	} else {
		if(type == L_STRIP) {
			while(*s && *s == c) {
				s++;
			}
			p = s;	
		} else {
			s = s + slen - 1;
			while(*s && *s == c) {
				s--;
			}
			*(s + 1) = '\0';
		}
	}

	return p;
}

char *l_strip(char *str, const char c)
{
	return _strip(str, c, L_STRIP);
}

char *r_strip(char *str, const char c) 
{
	return _strip(str, c, R_STRIP);
}

char *strip(char *str, const char c) 
{
	char *p = NULL;

	p = _strip(str, c, L_STRIP);
	return _strip(p, c, R_STRIP);
}

