二、libpcap的抓包框架
    pcap_lookupdev()函数用于查找网络设备，返回可被pcap_open_live()函数调用的网络设备名指针。
    pcap_open_live()函数用于打开网络设备，并且返回用于捕获网络数据包的数据包捕获描述字。对于此网络设备的操作都要基于此网络设备描述字。
    pcap_lookupnet()函数获得指定网络设备的网络号和掩码。
    pcap_compile()函数用于将用户制定的过滤策略编译到过滤程序中。
    pcap_setfilter()函数用于设置过滤器。
    pcap_loop()函数pcap_dispatch()函数用于捕获数据包，捕获后还可以进行处理，此外pcap_next()和pcap_next_ex()两个函数也可以用来捕获数据包。
    pcap_close()函数用于关闭网络设备，释放资源
    
其实pcap的应用程序格式很简单，总的来说可以可以分为以下5部分，相信看了以下的5部分，大概能对pcap的总体布局有个大概的了解了吧：
1.我们从决定用哪一个接口进行嗅探开始。在Linux中，这可能是eth0，而在BSD系统中则可能是xl1等等。我们也可以用一个字符串来定义这个设备，或者采用pcap提供的接口名来工作。
2.初始化pcap。在这里我们要告诉pcap对什么设备进行嗅探。假如愿意的话，我们还可以嗅探多个设备。怎样区分它们呢？使用 文件句柄。就像打开一个文件进行读写一样，必须命名我们的嗅探“会话”，以此使它们各自区别开来。
3.假如我们只想嗅探特定的传输（如TCP/IP包，发往端口23的包等等），我们必须创建一个规则集合，编译并且使用它。这个过程分为三个相互紧密关联的阶段。规则集合被置于一个字符串内，并且被转换成能被pcap读的格式(因此编译它)。编译实际上就是在我们的程序里调用一个不被外部程序使用的函数。接下来我们要告诉 pcap使用它来过滤出我们想要的那一个会话。
4.最后，我们告诉pcap进入它的主体执行循环。在这个阶段内pcap一直工作到它接收了所有我们想要的包为止。每当它收到一个包就调用另一个已经定义好的函数，这个函数可以做我们想要的任何工作，它可以剖析所部获的包并给用户打印出结果，它可以将结果保存为一个文件，或者什么也不作。
5.在嗅探到所需的数据后，我们要关闭会话并结束。

三、实现libpcap的每一个步骤
（1）设置设备
这是很简单的。有两种方法设置想要嗅探的设备。
第一种，我们可以简单的让用户告诉我们。考察下面的程序：
　　 #include
　　 #include
　　 int main(int argc, char *argv[])
　　 {
　　 char *dev = argv[1];
　　 printf("Device: %s", dev);
　　 return(0);
　　 }
用户通过传递给程序的第一个参数来指定设备。字符串“dev”以pcap能“理解”的格式保存了我们要嗅探的接口的名字（当然，用户必须给了我们一个真正存在的接口）。
另一种也是同样的简单。来看这段程序：
　　 #include
　　 #include
　　 int main()
　　 {
　　 char *dev, errbuf[PCAP_ERRBUF_SIZE];
　　 dev = pcap_lookupdev(errbuf);
　　 printf("Device: %s", dev);
　　 return(0);
　　 }
（2）打开设备进行嗅探
创建一个嗅探会话的任务真的非常简单。为此，我们使用pcap_open_live()函数。此函数的原型（根据pcap的手册页）如下：
　　 pcap_t *pcap_open_live(char *device, int snaplen, int promisc, int to_ms, char *ebuf)
其第一个参数是我们在上一节中指定的设备，snaplen是整形的，它定义了将被pcap捕捉的最大字节数。当promisc设为true时将置指定接口为混杂模式（然而，当它置为false时接口仍处于混杂模式的非凡情况也是有可能的）。to_ms是读取时的超时值，单位是毫秒(假如为0则一直嗅探直到错误发生，为-1则不确定)。最后，ebuf是一个我们可以存入任何错误信息的字符串（就像上面的errbuf）。此函数返回其会话句柄。
混杂模式与非混杂模式的区别：这两种方式区别很大。一般来说，非混杂模式的嗅探器中，主机仅嗅探那些跟它直接有关的通信，如发向它的，从它发出的，或经它路由的等都会被嗅探器捕捉。而在混杂模式中则嗅探传输线路上的所有通信。在非交换式网络中，这将是整个网络的通信。这样做最明显的优点就是使更多的包被嗅探到，它们因你嗅探网络的原因或者对你有帮助，或者没有。但是，混杂模式是可被探测到的。一个主机可以通过高强度的测试判定另一台主机是否正在进行混杂模式的嗅探。其次，它仅在非交换式的网络环境中有效工作（如集线器，或者交换中的ARP层面）。再次，在高负荷的网络中，主机的系统资源将消耗的非常严重。
（3）过滤通信
实现这一过程由pcap_compile()与pcap_setfilter()这两个函数完成。
在使用我们自己的过滤器前必须编译它。过滤表达式被保存在一个字符串中（字符数组）。其句法在tcpdump的手册页中被证实非常好。我建议你亲自阅读它。但是我们将使用简单的测试表达式，这样你可能很轻易理解我的例子。
我们调用pcap_compile()来编译它，其原型是这样定义的：
　　 int pcap_compile(pcap_t *p, strUCt bpf_program *fp, char *str, int optimize, bpf_u_int32 netmask)
第一个参数是会话句柄。接下来的是我们存储被编译的过滤器版本的地址的引用。再接下来的则是表达式本身，存储在规定的字符串格式里。再下边是一个定义表达式是否被优化的整形量（0为false，1为true，标准规定）。最后，我们必须指定应用此过滤器的网络掩码。函数返回-1为失败，其他的任何值都表明是成功的。
表达式被编译之后就可以使用了。现在进入pcap_setfilter()。仿照我们介绍pcap的格式，先来看一看pcap_setfilter()的原型：
　　 int pcap_setfilter(pcap_t *p, struct bpf_program *fp)
这非常直观，第一个参数是会话句柄，第二个参数是被编译表达式版本的引用（可推测出它与pcap_compile()的第二个参数相同）。
下面的代码示例可能能使你更好的理解：
　　 #include
　　 pcap_t *handle; /* 会话的句柄 */
　　 char dev[] = "eth0"; /* 执行嗅探的设备 */
　　 char errbuf[PCAP_ERRBUF_SIZE]; /* 存储错误 信息的字符串 */
　　 struct bpf_program filter; /*已经编译好的过滤表达式*/
　　 char filter_app[] = "port 23"; /* 过滤表达式*/
　　 bpf_u_int32 mask; /* 执行嗅探的设备的网络掩码 */
　　 bpf_u_int32 net; /* 执行嗅探的设备的IP地址 */
　　 pcap_lookupnet(dev, &net, &mask, errbuf);
　　 handle = pcap_open_live(dev, BUFSIZ, 1, 0, errbuf);
　　 pcap_compile(handle, &filter, filter_app, 0, net);
　　 pcap_setfilter(handle, &filter);
这个程序使嗅探器嗅探经由端口23的所有通信，使用混杂模式，设备是eth0。
（4）实际的嗅探
有两种手段捕捉包。我们可以一次只捕捉一个包，也可以进入一个循环，等捕捉到多个包再进行处理。我们将先看看怎样去捕捉单个包，然后再看看使用循环的方法。为此，我们使用函数pcap_next()。
pcap_next()的原型及其简单：
　　 u_char *pcap_next(pcap_t *p, struct pcap_pkthdr *h)
第一个参数是会话句柄，第二个参数是指向一个包括了当前数据包总体信息（被捕捉时的时间，包的长度，其被指定的部分长度）的结构体的指针（在这里只有一个片断，只作为一个示例）。pcap_next()返回一个u_char指针给被这个结构体描述的包。我们将稍后讨论这种实际读取包本身的手段。
　　 这里有一个演示怎样使用pcap_next()来嗅探一个包的例子：
　　 #include
　　 #include
　　 int main()
　　 {
　　 pcap_t *handle; /* 会话句柄 */
　　 char *dev; /* 执行嗅探的设备 */
　　 char errbuf[PCAP_ERRBUF_SIZE]; /* 存储错误信息的字符串 */
　　
　　 struct bpf_program filter; /* 已经编译好的过滤器 */
　　 char filter_app[] = "port 23"; /* 过滤表达式 */
　　 bpf_u_int32 mask; /* 所在网络的掩码 */
　　 bpf_u_int32 net; /* 主机的IP地址 */
　　 struct pcap_pkthdr header; /* 由pcap.h定义 */
　　 const u_char *packet; /* 实际的包 */
　　 /* Define the device */
　　 dev = pcap_lookupdev(errbuf);
　　 /* 探查设备属性 */
　　 pcap_lookupnet(dev, &net, &mask, errbuf);
　　 /* 以混杂模式打开会话 */
　　 handle = pcap_open_live(dev, BUFSIZ, 1, 0, errbuf);
　　 /* 编译并应用过滤器 */
　　 pcap_compile(handle, &filter, filter_app, 0, net);
　　 pcap_setfilter(handle, &filter);
　　 /* 截获一个包 */
　　 packet = pcap_next(handle, &header);
　　 /* 打印它的长度 */
　　 printf("Jacked a packet with length of [%d]
　　 ", header.len);
　　 /* 关闭会话 */
　　 pcap_close(handle);
　　 return(0);
　　 }
这个程序嗅探被pcap_lookupdev()返回的设备并将它置为混杂模式。它发现第一个包经过端口23（telnet）并且告诉用户此包的大小（以字 节为单位）。这个程序又包含了一个新的调用pcap_close()，我们将在后面讨论（尽管它的名字就足够证实它自己的作用）。
实际上很少有嗅探程序会真正的使用pcap_next()。通常，它们使用pcap_loop()或者 pcap_dispatch()（它就是用了pcap_loop()）。
pcap_loop()的原型如下：
　　 int pcap_loop(pcap_t *p, int cnt, pcap_handler callback, u_char *user)
第一个参数是会话句柄，接下来是一个整型，它告诉pcap_loop()在返回前应捕捉多少个数据包（若为负值则表示应该一直工作直至错误发生）。第三个参数是回调函数的名称（正像其标识符所指，无括号）。最后一个参数在有些应用里有用，但更多时候则置为NULL。假设我们有我们自己的想送往回调函数的参数，另外还有pcap_loop()发送的参数，这就需要用到它。很明显，必须是一个u_char类型的指针以确保结果正确；正像我们稍后见到的，pcap使用了很有意思的方法以u_char指针的形势传递信息。pcap_dispatch()的用法几乎相同。唯一不同的是它们如何处理超时（还记得在调用pcap_open_live()时怎样设置超时吗？这就是它起作用的地方）。Pcap_loop()忽略超时而pcap_dispatch()则不。关于它们之间区别的更深入的讨论请参见pcap的手册页。
回调函数的原型：
　　 void got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet);
让我们更细致的考察它。首先，你会注重到该函数返回void类型，这是符合逻辑的，因为pcap_loop()不知道如何去处理一个回调返回值。第一个参数相应于pcap_loop()的最后一个参数。每当回调函数被老婆 调用时，无论最后一个参数传给pcap_loop()什么值，这个值都会传给我们回调函数的第一个参数。第二个参数是pcap头文件定义的，它包括数据包被嗅探的时间、大小等信息。结构体pcap_pkhdr在pcap.h中定义如下：
　　 struct pcap_pkthdr {
　　 struct timeval ts; /* 时间戳 */
　　 bpf_u_int32 caplen; /* 已捕捉部分的长度 */
　　 bpf_u_int32 len; /* 该包的脱机长度 */
　　 };
这些量都相当明了。最后一个参数在它们中是最有意思的，也最让pcap程序新手感到迷惑。这又是一个u_char指针，它包含了被pcap_loop()嗅探到的所有包。
但是你怎样使用这个我们在原型里称为packet的变量呢？一个数据包包含许多属性，因此你可以想象它不只是一个字符串，而实质上是一个结构体的集合（比如，一个TCP/IP包会有一个以太网的头部，一个IP头部，一个TCP头部，还有此包的有效载荷）。这个u_char就是这些结构体的串联版本。为了使用它，我们必须作一些有趣的匹配工作。
下面这些是一些数据包的结构体：
　　 /* 以太网帧头部 */
　　 struct sniff_ethernet {
　　 u_char ether_dhost[ETHER_ADDR_LEN]; /* 目的主机的地址 */
　　 u_char ether_shost[ETHER_ADDR_LEN]; /* 源主机的地址 */
　　 u_short ether_type; /* IP? ARP? RARP? etc */
　　 };
　　 /* IP数据包的头部 */
　　 struct sniff_ip {
　　 #if BYTE_ORDER == LITTLE_ENDIAN
　　 u_int ip_hl:4, /* 头部长度 */
　　 ip_v:4; /* 版本号 */
　　 #if BYTE_ORDER == BIG_ENDIAN
　　 u_int ip_v:4, /* 版本号 */
　　 ip_hl:4; /* 头部长度 */
　　 #endif
　　 #endif /* not _IP_VHL */
　　 u_char ip_tos; /* 服务的类型 */
　　 u_short ip_len; /* 总长度 */
　　 u_short ip_id; /*包标志号 */
　　 u_short ip_off; /* 碎片偏移 */
　　 #define IP_RF 0x8000 /* 保留的碎片标志 */
　　 #define IP_DF 0x4000 /* dont fragment flag */
　　 #define IP_MF 0x2000 /* 多碎片标志*/
　　 #define IP_OFFMASK 0x1fff /*分段位 */
　　 u_char ip_ttl; /* 数据包的生存时间 */
　　 u_char ip_p; /* 所使用的协议 */
　　 u_short ip_sum; /* 校验和 */
　　 struct in_addr ip_src,ip_dst; /* 源地址、目的地址*/
　　 };
　　 /* TCP 数据包的头部 */
　　 struct sniff_tcp {
　　 u_short th_sport; /* 源端口 */
　　 u_short th_dport; /* 目的端口 */
　　 tcp_seq th_seq; /* 包序号 */
　　 tcp_seq th_ack; /* 确认序号 */
　　 #if BYTE_ORDER == LITTLE_ENDIAN
　　 u_int th_x2:4, /* 还没有用到 */
　　 th_off:4; /* 数据偏移 */
　　 #endif
　　 #if BYTE_ORDER == BIG_ENDIAN
　　 u_int th_off:4, /* 数据偏移*/
　　 th_x2:4; /*还没有用到 */
　　 #endif
　　 u_char th_flags;
　　 #define TH_FIN 0x01
　　 #define TH_SYN 0x02
　　 #define TH_RST 0x04
　　 #define TH_PUSH 0x08
　　 #define TH_ACK 0x10
　　 #define TH_URG 0x20
　　 #define TH_ECE 0x40
　　 #define TH_CWR 0x80
　　 #define TH_FLAGS (TH_FINTH_SYNTH_RSTTH_ACKTH_URGTH_ECETH_CWR)
　　 u_short th_win; /* TCP滑动窗口 */
　　 u_short th_sum; /* 头部校验和 */
　　 u_short th_urp; /* 紧急服务位 */
　　 };
pcap嗅探数据包时正是使用的这些结构。接下来，它简单的创建一个u_char字符串并且将这些结构体填入。那么我们怎样才能区分它们呢?预备好见证指针最实用的好处之一吧。
我们再一次假定要对以太网上的TCP/IP包进行处理。同样的手段可以应用于任何数据包，唯一的区别是你实际所使用的结构体的类型。让我们从声明分解u_char包的变量开始：
　　 const struct sniff_ethernet *ethernet; /* 以太网帧头部*/
　　 const struct sniff_ip *ip; /* IP包头部 */
　　 const struct sniff_tcp *tcp; /* TCP包头部 */
　　 const char *payload; /* 数据包的有效载荷*/
　　 /*为了让它的可读性好，我们计算每个结构体中的变量大小*/
　　 int size_ethernet = sizeof(struct sniff_ethernet);
　　 int size_ip = sizeof(struct sniff_ip);
　　 int size_tcp = sizeof(struct sniff_tcp);
　　 现在我们开始让人感到有些神秘的匹配：
　　 ethernet = (struct sniff_ethernet*)(packet);
　　 ip = (struct sniff_ip*)(packet + size_ethernet);
　　 tcp = (struct sniff_tcp*)(packet + size_ethernet + size_ip);
　　 payload = (u_char *)(packet + size_ethernet + size_ip + size_tcp);
　　
此处如何工作？考虑u_char在内存中的层次。基本的，当pcap将这些结构体填入u_char的时候是将这些数据存入一个字符串中，那个字符串将被送入我们的回调函数中。反向转换是这样的，不考虑这些结构体制中的值，它们的大小将是一致的。例如在我的平台上，一个sniff_ethernet结构体的大小是14字节。一个sniff_ip结构体是20字节，一个sniff_tcp结构体也是20字节。 u_char指针正是包含了内存地址的一个变量，这也是指针的实质，它指向内存的一个区域。简单而言，我们说指针指向的地址为x，假如三个结构体恰好线性排列，第一个（sniff_ethernet）被装载到内存地址的x处则我们很轻易的发现其他结构体的地址，让我们以表格显示之：
　　 Variable Location (in bytes)
　　 sniff_ethernet X
　　 sniff_ip X + 14
　　 sniff_tcp X + 14 + 20
　　 payload X + 14 + 20 + 20
结构体sniff_ethernet正好在x处，紧接着它的sniff_ip则位于x加上它本身占用的空间（此例为14字节），依此类推可得全部地址。
注重：你没有假定你的变量也是同样大小是很重要的。你应该总是使用sizeof()来确保尺寸的正确。这是因为这些结构体中的每个成员在不同平台下可以有不同的尺寸。

下面是主要函数接口：
pcap_t *pcap_open_live(char *device, int snaplen,
   int promisc, int to_ms, char *ebuf)
   获得用于捕获网络数据包的数据包捕获描述字。device参数为指定打开
   的网络设备名。snaplen参数定义捕获数据的最大字节数。promisc指定
   是否将网络接口置于混杂模式。to_ms参数指定超时时间（毫秒）。
   ebuf参数则仅在pcap_open_live()函数出错返回NULL时用于传递错误消
   息。
pcap_t *pcap_open_offline(char *fname, char *ebuf)
   打开以前保存捕获数据包的文件，用于读取。fname参数指定打开的文
   件名。该文件中的数据格式与tcpdump和tcpslice兼容。"-"为标准输
   入。ebuf参数则仅在pcap_open_offline()函数出错返回NULL时用于传
   递错误消息。
pcap_dumper_t *pcap_dump_open(pcap_t *p, char *fname)
   打开用于保存捕获数据包的文件，用于写入。fname参数为"-"时表示
   标准输出。出错时返回NULL。p参数为调用pcap_open_offline()或
   pcap_open_live()函数后返回的pcap结构指针。fname参数指定打开
   的文件名。如果返回NULL，则可调用pcap_geterr()函数获取错误消
   息。

char *pcap_lookupdev(char *errbuf)
   用于返回可被pcap_open_live()或pcap_lookupnet()函数调用的网络
   设备名指针。如果函数出错，则返回NULL，同时errbuf中存放相关的
   错误消息。
int pcap_lookupnet(char *device, bpf_u_int32 *netp,
   bpf_u_int32 *maskp, char *errbuf)
   获得指定网络设备的网络号和掩码。netp参数和maskp参数都是
   bpf_u_int32指针。如果函数出错，则返回-1，同时errbuf中存放相
   关的错误消息。
int pcap_dispatch(pcap_t *p, int cnt,
   pcap_handler callback, u_char *user)
   捕获并处理数据包。cnt参数指定函数返回前所处理数据包的最大值。
   cnt=-1表示在一个缓冲区中处理所有的数据包。cnt=0表示处理所有
   数据包，直到产生以下错误之一：读取到EOF；超时读取。callback
   参数指定一个带有三个参数的回调函数，这三个参数为：一个从
   pcap_dispatch()函数传递过来的u_char指针，一个pcap_pkthdr结构
   的指针，和一个数据包大小的u_char指针。如果成功则返回读取到的
   字节数。读取到EOF时则返回零值。出错时则返回-1，此时可调用
   pcap_perror()或pcap_geterr()函数获取错误消息。
int pcap_loop(pcap_t *p, int cnt,
   pcap_handler callback, u_char *user)
   功能基本与pcap_dispatch()函数相同，只不过此函数在cnt个数据包
   被处理或出现错误时才返回，但读取超时不会返回。而如果为
   pcap_open_live()函数指定了一个非零值的超时设置，然后调用
   pcap_dispatch()函数，则当超时发生时pcap_dispatch()函数会返回。
   cnt参数为负值时pcap_loop()函数将始终循环运行，除非出现错误。
void pcap_dump(u_char *user, struct pcap_pkthdr *h,
   u_char *sp)
   向调用pcap_dump_open()函数打开的文件输出一个数据包。该函数可
   作为pcap_dispatch()函数的回调函数。
int pcap_compile(pcap_t *p, struct bpf_program *fp,
   char *str, int optimize, bpf_u_int32 netmask)
   将str参数指定的字符串编译到过滤程序中。fp是一个bpf_program结
   构的指针，在pcap_compile()函数中被赋值。optimize参数控制结果
   代码的优化。netmask参数指定本地网络的网络掩码。
int pcap_setfilter(pcap_t *p, struct bpf_program *fp)
   指定一个过滤程序。fp参数是bpf_program结构指针，通常取自
   pcap_compile()函数调用。出错时返回-1；成功时返回0。
u_char *pcap_next(pcap_t *p, struct pcap_pkthdr *h)
   返回指向下一个数据包的u_char指针。
int pcap_datalink(pcap_t *p)
   返回数据链路层类型，例如DLT_EN10MB。
int pcap_snapshot(pcap_t *p)
   返回pcap_open_live被调用后的snapshot参数值。
int pcap_is_swapped(pcap_t *p)
   返回当前系统主机字节与被打开文件的字节顺序是否不同。
int pcap_major_version(pcap_t *p)
   返回写入被打开文件所使用的pcap函数的主版本号。
int pcap_minor_version(pcap_t *p)
   返回写入被打开文件所使用的pcap函数的辅版本号。
int pcap_stats(pcap_t *p, struct pcap_stat *ps)
   向pcap_stat结构赋值。成功时返回0。这些数值包括了从开始
   捕获数据以来至今共捕获到的数据包统计。如果出错或不支持
   数据包统计，则返回-1，且可调用pcap_perror()或
   pcap_geterr()函数来获取错误消息。
FILE *pcap_file(pcap_t *p)
   返回被打开文件的文件名。
int pcap_fileno(pcap_t *p)
   返回被打开文件的文件描述字号码。
void pcap_perror(pcap_t *p, char *prefix)
   在标准输出设备上显示最后一个pcap库错误消息。以prefix参
   数指定的字符串为消息头。
char *pcap_geterr(pcap_t *p)
   返回最后一个pcap库错误消息。
char *pcap_strerror(int error)
   如果strerror()函数不可用，则可调用pcap_strerror函数替代。
void pcap_close(pcap_t *p)
   关闭p参数相应的文件，并释放资源。