#ifndef __IP_COMMON_H__
#define __IP_COMMON_H__

int iproute_add(unsigned int dstaddr, int mask, unsigned int gw, int metric, int table, const char *ifname);
int iproute_del(unsigned int dstaddr, int mask, unsigned int gw, int metric, int table, const char *ifname);
int iproute_flush(int table);

int iprule_add(unsigned int src, int src_len, unsigned int dst, unsigned int dst_len, 
	unsigned int fwmark, int pref, int table, int invert, const char *iif, const char *oif);

int iprule_del(unsigned int src, int src_len, unsigned int dst, unsigned int dst_len, 
	unsigned int fwmark, int pref, int table, int invert, const char *iif, const char *oif);

#endif //__IP_COMMON_H__
