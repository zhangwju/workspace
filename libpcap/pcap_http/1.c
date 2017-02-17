#include <stdio.h>  
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <pcap.h>  
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/tcp.h>

#define DEVICE "enp0s3"

void show_ethhdr(struct ethhdr *eth)
{
	
}

void prase_packet(const u_char *buf,  int caplen)
{
	uint16_t e_type;
	uint32_t offset;
	int payload_len;
	const u_char *tcp_payload;
	
	/* ether header */
	struct ethhdr *eth = NULL;
	eth = (struct ethhdr *)buf;
	e_type = ntohs(eth->h_proto);
	offset = sizeof(struct ethhdr);

	printf("----------------eth--------------------\n");
    printf("destination eth addr: %02x:%02x:%02x:%02x:%02x:%02x\n",
        eth->h_dest[0], eth->h_dest[1],
        eth->h_dest[2], eth->h_dest[3],
        eth->h_dest[4], eth->h_dest[5]);
    printf("source eth addr: %02x:%02x:%02x:%02x:%02x:%02x\n",
        eth->h_source[0], eth->h_source[1],
        eth->h_source[2], eth->h_source[3],
        eth->h_source[4], eth->h_source[5]);
    printf("protocol is: %04x\n", ntohs(eth->h_proto));

	/*vlan 802.1q*/	
	while(e_type == ETH_P_8021Q) {
        e_type = (buf[offset+2] << 8) + buf[offset+3];
        offset += 4;
    }   

    if (e_type != ETH_P_IP) {
        return;
    }   

	/* ip header */	
    struct iphdr *ip = (struct iphdr *)(buf + offset);
	e_type = ntohs(ip->protocol);
	offset += sizeof(struct iphdr);
	struct in_addr addr;
	 
	printf("----------------ip--------------------\n");
    printf("version: %d\n", ip->version);
    printf("head len: %d\n", ip->ihl * 4);
    printf("total len: %d\n", ntohs(ip->tot_len));
    printf("ttl: %d\n", ip->ttl);
    printf("protocol: %d\n", ip->protocol);
    printf("check: %x\n", ip->check);
	addr.s_addr = ip->saddr;
    printf("saddr: %s\n", inet_ntoa(addr));
	addr.s_addr = ip->daddr;
    printf("daddr: %s\n", inet_ntoa(addr));

	if(ip->protocol != IPPROTO_TCP) {
		return;
	}

	/*tcp header*/
	struct tcphdr *tcp = (struct tcphdr *)(buf + offset);
	offset += (tcp->doff << 2);
	payload_len = caplen - offset;
	tcp_payload = (buf + offset);
	
	printf("----------------tcp--------------------\n");
	printf("offset: %d\n", offset);
	printf("tcp len: %d\n", sizeof(struct tcphdr));
	printf("tcp->doff: %d\n", tcp->doff * 4);
	printf("source port: %d\n", ntohs(tcp->source));
	printf("dest port: %d\n", ntohs(tcp->dest));
	printf("payload_len: %d\n", payload_len);
	printf("sequence number: %d\n", ntohs(tcp->seq));
	printf("ack sequence: %d\n", ntohs(tcp->ack_seq));

	if(memcmp(tcp_payload, "GET ", 4)) {
        return;
    }

}

void get_packet(u_char *user, const struct pcap_pkthdr *pkthdr, const u_char *packet)
{
	printf("----------------------------start -------------------------------\n");
	printf("Packet length: %d\n", pkthdr->len);  
	printf("Number of bytes: %d\n", pkthdr->caplen);  
  	printf("Recieved time: %s\n", ctime((const time_t *)&pkthdr->ts.tv_sec));

#if 0
	/*print recv packet*/
	int i = 0;
	for(i = 0; i < pkthdr->len; i++) {
		printf("%x\t", packet[i]);
		if(i % 16 == 0) {
		    printf("\n");
		}
	}
	printf("\n\n\n");
#endif
	
	prase_packet(packet, pkthdr->len);
}

int main()  
{  
	char errBuf[PCAP_ERRBUF_SIZE];
	struct pcap_pkthdr packet;  
	pcap_t *dev;
	bpf_u_int32 netp, maskp;
	char *net, *mask;
	struct in_addr addr;
	int ret;

	if(pcap_lookupnet(DEVICE, &netp, &maskp, errBuf)) {
		printf("get net failure\n");
		exit(1);
	}
	addr.s_addr = netp;
	net = inet_ntoa(addr);
	printf("network: %s\n", net);
	
	addr.s_addr = maskp;
	mask = inet_ntoa(addr);
	printf("mask: %s\n", mask);

	dev = pcap_open_live(DEVICE, 65536, 1, 0, errBuf);
	if(NULL == dev) {
		printf("open %s failure\n", DEVICE);
		exit(1);
	}
	
	pcap_loop(dev, 0, get_packet, NULL);
	
	pcap_close(dev);

	return 0; 
} 
	  
