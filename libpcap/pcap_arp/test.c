#include <stdio.h>  
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <pcap.h>
#include <arpa/inet.h>

#define MAXBYTES2CAPTURE 2048
#define ARP_REQUEST 1
#define ARP_REPLY 2

typedef struct arphdr {
	u_int16_t htype;        //hardware type
	u_int16_t ptype;        //protocol type
	u_char hlen;            //hardware address length
	u_char plen;            //protocol address length
	u_int16_t oper;         //operation code
	u_char sha[6];          //sendHardware address
	u_char spa[4];          //sender ip address
	u_char tha[6];          //target hardware address
	u_char tpa[4];          //target ip address
}arphdr_t;

int main(int argc, char **argv)
{
	int i = 0;
	bpf_u_int32 net = 0;
	bpf_u_int32 mask = 0;
	struct bpf_program filter; /*place to store the filter program*/
	char errbuf[PCAP_ERRBUF_SIZE];
	pcap_t *handle = NULL;   /*interface handle*/
	struct pcap_pkthdr pkthdr; /**/
	const unsigned char *packet = NULL; /*received raw data*/
	arphdr_t *arpheader = NULL; /*point to arp header*/
	
	memset(errbuf, 0, PCAP_ERRBUF_SIZE);
	
	if(argc != 2) {
		printf("USAGE: arpsniffer <interface>\n");
		exit(1);
	}
	
	/*open network device for packet capture*/
	handle = pcap_open_live(argv[1], MAXBYTES2CAPTURE, 0, 512, errbuf);
	if (handle == NULL) {
        fprintf(stderr, "Couldn't open device %s: %s\n", argv[1], errbuf);
		exit(1);
	}

	/*look up device network addr and mask*/
	if (pcap_lookupnet(argv[1], &net, &mask, errbuf) == -1) {
        fprintf(stderr, "Couldn't get netmask for device %s: %s\n", argv[1], errbuf);
		exit(1);
    }
	
	/*complie the filter expression of filter program*/
	pcap_compile(handle, &filter, "arp", 0, mask);

	pcap_setfilter(handle, &filter);

	while(1) {
		/*Get one packet if null continue wait*/
		if ((packet = pcap_next(handle, &pkthdr)) == NULL) {
			continue;
		}

		arpheader = (struct arphdr *)(packet + 14); /*Point to the ARP header*/

		printf("\n------------- ARP --------------\n");
		printf("Received Packet Size: %d bytes\n", pkthdr.len);
		printf("Hardware type: %s\n", (ntohs(arpheader->htype) == 1)?"Ethernet":"Unknown");
		printf("Ptotocol type: %s\n", (ntohs(arpheader->ptype) == 0x0800)?"IPv4":"Unknown");
		printf("Operation : %s\n", (ntohs(arpheader->oper) == ARP_REQUEST)?"ARP_REQUEST":"ARP_REPLY");

		/*If is Ethernet and IPv4 print packet contents*/
		if (ntohs(arpheader->htype) == 1 && ntohs(arpheader->ptype) == 0x0800) {
			printf("\nSoucre MAC:%02x:%02x:%02X:%02x:%02x:%02x\n", arpheader->sha[0], arpheader->sha[1], arpheader->sha[2],
																   arpheader->sha[3], arpheader->sha[4], arpheader->sha[5]);
			printf("Soucre IP:%d.%d.%d.%d\n", arpheader->spa[0], arpheader->spa[1], 
												arpheader->spa[2], arpheader->spa[3]);
			printf("\nDestination MAC:%02x:%02x:%02X:%02x:%02x:%02x\n", arpheader->tha[0], arpheader->tha[1], arpheader->tha[2],
																   		arpheader->tha[3], arpheader->tha[4], arpheader->tha[5]);
			printf("Destination IP:%d.%d.%d.%d\n", arpheader->tpa[0], arpheader->tpa[1], 
													arpheader->tpa[2], arpheader->tpa[3]);
		}
		
	}
	
	return 0;
} 


	  
