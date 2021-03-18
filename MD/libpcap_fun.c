
//pcap_lookupdev()函数用于查找网络设备，返回可被pcap_open_live()函数调用的网络设备名指针。
//pcap_open_live()函数用于打开网络设备，并且返回用于捕获网络数据包的数据包捕获描述字。对于此网络设备的操作都要基于此网络设备描述字。
//pcap_lookupnet()函数获得指定网络设备的网络号和掩码。
//pcap_compile()函数用于将用户制定的过滤策略编译到过滤程序中。
//pcap_setfilter()函数用于设置过滤器。
//pcap_loop()函数pcap_dispatch()函数用于捕获数据包，捕获后还可以进行处理，此外pcap_next()和pcap_next_ex()两个函数也可以用来捕获数据包。
//pcap_close()函数用于关闭网络设备，释放资源。

一、libpcap简介
	libpcap（Packet Capture Library），即数据包捕获函数库，是Unix/Linux平台下的网络数据包捕获函数库。它是一个独立于系统的用户层包捕获的API接口，为底层网络监测提供了一个可移植的框架.
Libpcap可以在绝大多数类unix平台下工作,libpcap库安装也很简单，Libpcap 软件包可从 http://www.tcpdump.org/ 下载，然后依此执行下列三条命令即可安装
./configure
make
make install

二、pcap基本工作流程
	首先，需要确定我们将要嗅探的接口，在linux下是类似eth0的东西。在BSD下是类似xll的东西。可以在一个字符串中声明设备，也可以让pcap提供备选接口（我们想要嗅探的接口）的名字。初始化pcap：此时才真正告诉pcap我们要嗅探的具体接口，只要我们愿意，我们可以嗅探多个接口。但是如何区分多个接口呢，使用文件句柄。就像读写文件时使用文件句柄一样。我们必须给嗅探任务命名，以至于区分不同的嗅探任务。
当我们只想嗅探特殊的流量时（例如，仅仅嗅探TCP/IP包、仅仅嗅探经过端口23的包，等等）我们必须设定一个规则集，“编译”并应用它。这是一个三相的并且紧密联系的过程，规则集存储与字符串中，在“编译”之后会转换成pcap可以读取的格式。“编译过程”实际上是调用自定义的函数完成的，不涉及外部的函数。然后我们可以告诉pcap在我们想要过滤的任何任务上实施。
最后，告诉pcap进入主要的执行循环中，在此阶段，在接收到任何我们想要的包之前pcap将一直循环等待。在每次抓取到一个新的数据包时，它将调用另一个自定义的函数，我们可以在这个函数中肆意妄为，例如，解析数据包并显示数据内容、保存到文件或者什么都不做等等。
当嗅探完美任务完成时，记得关掉任务。

下面我们看一下具体的步骤实施：
（1）确定我们将要嗅探的接口
这一步操作我们可以手动指定接口或者调用pcap库提供的接口来查找网络设备
手动指定：
#include <stdio.h>  
#include <string.h>
#include <stdlib.h>
  
int main(int argc, char **argv)  
{  
    char *dev = argv[1];

    printf("Device: %s\n", dev);
 
  return 0;  
}
使用pcap API查找网络设备
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pcap.h>

int main(int argc, char *argv[])
{
    char *dev, errbuf[PCAP_ERRBUF_SIZE];

    dev = pcap_lookupdev(errbuf);
    if (dev == NULL) {
        fprintf(stderr, "Couldn't find default device: %s\n", errbuf);
        return(2);
    }   

    printf("Device: %s\n", dev);
    return(0);
}
编译时需要连接pcap库 -lpcap
（2）打开嗅探设备
	pcap_t *pcap_open_live(char *device, int snaplen, int promisc, int to_ms, char *errbuf)；
	函数说明：该函数用于打开一个嗅探设备
	参数：device 需要打开的设备
		  snaplen int型，表示pcap可以捕获的最大字节数（最大为65535）
		  promisc 是否开启混杂模式（1打开，0关闭），设置开启混杂模式，需要对应的网卡也开启混杂模式
		  to_ms 是读取时间溢出，单位为毫秒（ms）， 0表示没有时间溢出
		  errbuf 保存错误的返回值
		  
下面是具体实现：
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pcap.h>

int main(int argc, char *argv[])
{
    char *dev, errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *handle;

    dev = pcap_lookupdev(errbuf);
    if (dev == NULL) {
        fprintf(stderr, "Couldn't find default device: %s\n", errbuf);
        return(2);
    }   
    printf("Device: %s\n", dev);
    
    handle = pcap_open_live(dev, BUFSIZ, 1, 1000, errbuf);
    if (handle == NULL) {
        fprintf(stderr, "Couldn't open device %s: %s\n", dev, errbuf);
        return(2);
    }   
    printf("Open Device success!\n");   

    return(0);
}
（2）过滤指定流量
	很多时候，我只需要我们指定的流量，比如我们需要劫持http请求（80端口），劫持DNS服务（53端口），因此，我们大多数时候都不会盲目的抓取全部的报文。
	相关过滤函数pcap_compile()and pcap_setfilter()，当我们调用pcap_open_live()后，我们会得到一个建立的嗅探会话，此时我们就可以开始过滤我们想要流量了；
	过滤器表达式是基于正则表达式来编写的，官网tcpdump有详细的规则说明，在使用过滤器之前我们必须”编译“
	
	int pcap_compile(pcap_t *p, struct bpf_program *fp, char *str, int optimize, bpf_u_int32 netmask)
	函数说明：将str参数指定的字符串编译到过滤程序中。
	参数：  p是嗅探器回话句柄
			fp是一个bpf_program结构的指针，在pcap_compile()函数中被赋值。o
			ptimize参数控制结果代码的优化。
			netmask参数指定本地网络的网络掩码。
编译完过滤表达式后，我们就可以应用它了，下面是int pcap_setfilter()，具体用法看man手册
		  int pcap_setfilter(pcap_t *p, struct bpf_program *fp）
		  第一个参数嗅探器回话句柄，第二参数是存储过滤器编译版本的结构体指针（跟pcap_compile 一个参数一样）
下面是简单实例：
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pcap.h>

int main(int argc, char *argv[])
{
    char *dev, errbuf[PCAP_ERRBUF_SIZE];
    struct bpf_program fp;      /* The compiled filter expression */
    char filter_exp[] = "port 53";  /* The filter expression (filter 53 port)*/
    pcap_t *handle;
    bpf_u_int32 mask;       /* The netmask of our sniffing device */
    bpf_u_int32 net;        /* The IP of our sniffing device */

    dev = pcap_lookupdev(errbuf);
    if (dev == NULL) {
        fprintf(stderr, "Couldn't find default device: %s\n", errbuf);
        return(2);
    }   
    printf("Device: %s\n", dev);

    /*get network mask*/    
    if (pcap_lookupnet(dev, &net, &mask, errbuf) == -1) {
        fprintf(stderr, "Can't get netmask for device %s\n", dev);
        net = 0;
        mask = 0;
    }   

    handle = pcap_open_live(dev, BUFSIZ, 1, 1000, errbuf);
    if (handle == NULL) {
        fprintf(stderr, "Couldn't open device %s: %s\n", dev, errbuf);
        return(2);
    }   
    if (pcap_compile(handle, &fp, filter_exp, 0, net) == -1) {
        fprintf(stderr, "Couldn't parse filter %s: %s\n", filter_exp, pcap_geterr(handle));
        return(2);
    }   
    if (pcap_setfilter(handle, &fp) == -1) {
        fprintf(stderr, "Couldn't install filter %s: %s\n", filter_exp, pcap_geterr(handle));
        return(2);
    }   

    return(0);
}
以上代码中的pcap_lookupnet()函数获得指定网络设备的网络号和掩码。函数原型如下：
int pcap_lookupnet(const char *device, bpf_u_int32 *netp,
               bpf_u_int32 *maskp, char *errbuf);
	参数：device 指定的嗅探设备名称
		  netp 指定设备的网络号
		  maskp 掩码
		  errbuf 保存错误信息
下面是具体实例：
#include <stdio.h>  
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pcap.h> 

#define DEVICE "enp0s3" 
 
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
        return -1; 
    }   
    addr.s_addr = netp;
    net = inet_ntoa(addr);
    printf("network: %s\n", net);
    
    addr.s_addr = maskp;
    mask = inet_ntoa(addr);
    printf("mask: %s\n", mask);

    return 0;
} 
[root@localhost pacp_1st]# ./pacp 
network: 192.168.16.0
mask: 255.255.255.0

通过以上内容，我们已经知道了如何指定获取以及初始化一个嗅探器设备，如何编译及使用过滤器；下面我们就开始进行抓包，抓包程序有抓一次包（pcap_next()）和循环一直抓包几个函数;
下面我们我们先用pcap_next()进行一次抓包
函数原型：
	const u_char *pcap_next(pcap_t *p, struct pcap_pkthdr *h);
	参数：p是嗅探器会话句柄
	h 是一个指向存储数据包概略信息结构体的指针
	struct pcap_pkthdr {
    struct timeval ts;  /* time stamp */ 
    bpf_u_int32 caplen; /* length of portion present */
    bpf_u_int32 len;    /* length this packet (off wire) */
};
//ts——时间戳
//caplen——真正实际捕获的包的长度
//len——这个包的长度

因为在某些情况下你不能保证捕获的包是完整的，例如一个包长1480，但是你捕获到1000的时候，可能因为某些原因就中止捕获了，所以caplen是记录实际捕获的包长，也就是1000，而len就是1480。
下面是使用pcap_next()抓包程序
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pcap.h>

int main(int argc, char *argv[])
{
    pcap_t *handle;         /* Session handle */
    char *dev;          /* The device to sniff on */
    char errbuf[PCAP_ERRBUF_SIZE];  /* Error string */
    struct bpf_program fp;      /* The compiled filter */
    char filter_exp[] = "port 23";  /* The filter expression */
    bpf_u_int32 mask;       /* Our netmask */
    bpf_u_int32 net;        /* Our IP */
    struct pcap_pkthdr header;  /* The header that pcap gives us */
    const u_char *packet;       /* The actual packet */
    
    /* Define the device */
    dev = pcap_lookupdev(errbuf);
    if (dev == NULL) {
        fprintf(stderr, "Couldn't find default device: %s\n", errbuf);
        return(2);
    }   
    /* Find the properties for the device */
    if (pcap_lookupnet(dev, &net, &mask, errbuf) == -1) {
        fprintf(stderr, "Couldn't get netmask for device %s: %s\n", dev, errbuf);
        net = 0;
        mask = 0;
    }   
    /* Open the session in promiscuous mode */
    handle = pcap_open_live(dev, BUFSIZ, 1, 100, errbuf);
    if (handle == NULL) {
        fprintf(stderr, "Couldn't open device %s: %s\n", dev, errbuf);
        return(2);
    }   
    /* Compile and apply the filter */
    if (pcap_compile(handle, &fp, filter_exp, 0, net) == -1) {
        fprintf(stderr, "Couldn't parse filter %s: %s\n", filter_exp, pcap_geterr(handle));
        return(2);
    }   
    if (pcap_setfilter(handle, &fp) == -1) {
        fprintf(stderr, "Couldn't install filter %s: %s\n", filter_exp, pcap_geterr(handle));
        return(2);
    }   
    /* Grab a packet */
    packet = pcap_next(handle, &header);
    /* Print its length */
    printf("Packet length: %d\n", header.len);  
    printf("Number of bytes: %ud\n", header.caplen);  
    printf("Recieved time: %s\n", ctime((const time_t *)&header.ts.tv_sec)); 
    /* And close the session */
    pcap_close(handle);

    return(0);
}
[root@localhost pacp_5th]# ./pacp     
Packet length: 32603
Number of bytes: 3372236960d
Recieved time: Sat Aug 16 07:45:20 4461732


SYNOPSIS
       #include <pcap/pcap.h>
       char errbuf[PCAP_ERRBUF_SIZE];
	   
       pcap_t *pcap_open_live(const char *device, int snaplen,
               int promisc, int to_ms, char *errbuf);	
		
		char errbuf[PCAP_ERRBUF_SIZE];

       int pcap_lookupnet(const char *device, bpf_u_int32 *netp,
               bpf_u_int32 *maskp, char *errbuf);			
			   
      char *pcap_lookupdev(char *errbuf);

       typedef void (*pcap_handler)(u_char *user, const struct pcap_pkthdr *h,
                                   const u_char *bytes);

       int pcap_loop(pcap_t *p, int cnt,
               pcap_handler callback, u_char *user);
			   
       int pcap_dispatch(pcap_t *p, int cnt,
               pcap_handler callback, u_char *user);

       void pcap_close(pcap_t *p);
	          #include <pcap/pcap.h>
			  

       typedef void (*pcap_handler)(u_char *user, const struct pcap_pkthdr *h,
                                   const u_char *bytes);

       int pcap_loop(pcap_t *p, int cnt, pcap_handler callback, u_char *user);
	   /*参数说明：
			功能：循环捕获数据包,不会响应pcap_open_live()函数设置的超时时间
			参数 pcap_t *p: p是嗅探器会话句柄
			参数 cnt：cnt用于设置所捕获数据包的个数，负数的cnt表示pcap_loop永远循环抓包，直到出现错误。
			参数callback：是个回调函数指针，它的原型如下：
			typedef void (*pcap_handler)(u_char *user, const struct pcap_pkthdr *h,
                                   const u_char *bytes);
			参数 user：用来给回调函数传递参数的，在callback函数当中只有第一个user指针是可以留给用户使用的，
			如果你想给callback传递自己参数，那就只能通过pcap_loop的最后一个参数user来实现了*/
			
			struct pcap_pkthdr {
				struct timeval ts;  /* time stamp */ 
				bpf_u_int32 caplen; /* length of portion present */
				bpf_u_int32 len;    /* length this packet (off wire) */
			};
			//ts——时间戳
			//caplen——真正实际捕获的包的长度
			//len——这个包的长度

	/*因为在某些情况下你不能保证捕获的包是完整的，例如一个包长1480，但是你捕获到1000的时候，
可能因为某些原因就中止捕获了，所以caplen是记录实际捕获的包长，也就是1000，而len就是1480。*/

			   