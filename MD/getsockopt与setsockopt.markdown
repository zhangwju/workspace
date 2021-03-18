备注：本文非楼主原创，是楼主在网上发现的。。写的不错，存起来，以备后用


功能描述：
获取或者设置与某个套接字关联的选项。选项可能存在于多层协议中，它们总会出现在最上面的套接字层。
当操作套接字选项时，选项位于的层和选项的名称必须给出。
为了操作套接字层的选项，应该将层的值指定为SOL_SOCKET。
为了操作其它层的选项，控制选项的合适协议号必须给出。
例如，为了表示一个选项由TCP协议解析，层应该设定为协议 号TCP。


用法：
#include
#include

int getsockopt(int sock, int level, int optname, void *optval, socklen_t *optlen);

int setsockopt(int sock, int level, int optname, const void *optval, socklen_t optlen);

参数：  
sock：将要被设置或者获取选项的套接字。
level：选项所在的协议层。
optname：需要访问的选项名。
optval：对于getsockopt()，指向返回选项值的缓冲。
            对于setsockopt()，指向包含新选项值的缓冲。
optlen：对于getsockopt()，作为入口参数时，选项值的最大长度。作为出口参数时，选项值的实际长度。
            对于setsockopt()，现选项的长度。


返回说明：  


成功执行时，返回0。失败返回-1，
errno被设为以下的某个值  
EBADF：               sock不是有效的文件描述词
EFAULT：             optval指向的内存并非有效的进程空间
EINVAL：               在调用setsockopt()时，optlen无效
ENOPROTOOPT：指定的协议层不能识别选项
ENOTSOCK：        sock描述的不是套接字


参数详细说明：

level指定控制套接字的层次.可以取三种值:
  1)SOL_SOCKET:   通用套接字选项.
  2)IPPROTO_IP:      IP选项.
  3)IPPROTO_TCP:  TCP选项.　

optname指定控制的方式(选项的名称),我们下面详细解释　

optval获得或者是设置套接字选项.根据选项名称的数据类型进行转换　


选项名称　　　　　　　　说明　　　　　　　　　　　　　　　　　　数据类型
========================================================================
　　　　　　　　　　　　SOL_SOCKET
------------------------------------------------------------------------
SO_BROADCAST　　　   允许发送广播数据　　　　　　　　　　　　int
SO_DEBUG　　　　　　  允许调试　　　　　　　　　　　　　　　　int
SO_DONTROUTE　　　   不查找路由　　　　　　　　　　　　　　　int
SO_ERROR　　　　　　  获得套接字错误　　　　　　　　　　　　　int
SO_KEEPALIVE　　　　   保持连接　　　　　　　　　　　　　　　　int
SO_LINGER　　　　　　  延迟关闭连接　　　　　　　　　　　　　　struct linger
SO_OOBINLINE　　　　   带外数据放入正常数据流　　　　　　　　　int
SO_RCVBUF　　　　　    接收缓冲区大小　　　　　　　　　　　　　int
SO_SNDBUF　　　　　    发送缓冲区大小　　　　　　　　　　　　　int
SO_RCVLOWAT　　　     接收缓冲区下限　　　　　　　　　　　　　int
SO_SNDLOWAT　　　　  发送缓冲区下限　　　　　　　　　　　　　int
SO_RCVTIMEO　　　　　接收超时　　　　　　　　　　　　　　　　struct timeval
SO_SNDTIMEO　　　　    发送超时　　　　　　　　　　　　　　　　struct timeval
SO_REUSERADDR　　　  允许重用本地地址和端口　　　　　　　　　int
SO_TYPE　　　　　　　  获得套接字类型　　　　　　　　　　　　　int
SO_BSDCOMPAT　　　    与BSD系统兼容　　　　　　　　　　　　　 int
========================================================================
　　　　　　　　　　　　IPPROTO_IP
------------------------------------------------------------------------
IP_HDRINCL　　　　　　 在数据包中包含IP首部　　　　　　　　　　int
IP_OPTINOS　　　　　　 IP首部选项　　　　　　　　　　　　　　　int
IP_TOS　　　　　　　　  服务类型
IP_TTL　　　　　　　　   生存时间　　　　　　　　　　　　　　　　int
========================================================================
　　　　　　　　　　　　IPPRO_TCP
------------------------------------------------------------------------
TCP_MAXSEG　　　　　　　TCP最大数据段的大小　　　　　　　　　　 int
TCP_NODELAY　　　　　　 不使用Nagle算法　　　　　　　　　　　　 int
========================================================================

SO_RCVBUF和SO_SNDBUF每个套接口都有一个发送缓冲区和一个接收缓冲区，使用这两个套接口选项可以改变缺省缓冲区大小。

// 接收缓冲区
int nRecvBuf=32*1024;         //设置为32K
setsockopt(s,SOL_SOCKET,SO_RCVBUF,(const char*)&nRecvBuf,sizeof(int));


//发送缓冲区
int nSendBuf=32*1024;//设置为32K
setsockopt(s,SOL_SOCKET,SO_SNDBUF,(const char*)&nSendBuf,sizeof(int));

注意：

当设置TCP套接口接收缓冲区的大小时，函数调用顺序是很重要的，因为TCP的窗口规模选项是在建立连接时用SYN与对方互换得到的。
对于客户端，O_RCVBUF选项必须在connect之前设置；
对于服务羰，SO_RCVBUF选项必须在listen前设置。

结合原理说明：

1.每个套接口都有一个发送缓冲区和一个接收缓冲区。
接收缓冲区被TCP和UDP用来将接收到的数据一直保存到由应用进程来读。
TCP：
       TCP通告另一端的窗口大小。 TCP套接口接收缓冲区不可能溢出，因为对方不允许发出超过所通告窗口大小的数据。
       这就是TCP的流量控制，如果对方无视窗口大小而发出了超过窗口大小的数据，则接 收方TCP将丢弃它。
UDP：
       当接收到的数据报装不进套接口接收缓冲区时，此数据报就被丢弃。
       UDP是没有流量控制的；快的发送者可以很容易地就淹没慢的接收者，导致接收方的UDP丢弃数据报。

2.我们经常听说tcp协议的三次握手,但三次握手到底是什么，其细节是什么，为什么要这么做呢?
        第一次:客户端发送连接请求给服务器，服务器接收;
        第二次:服务器返回给客户端一个确认码,附带一个从服务器到客户端的连接请求,客户机接收,确认客户端到服务器的连接.
        第三次:客户机返回服务器上次发送请求的确认码,服务器接收,确认服务器到客户端的连接.
我们可以看到:
        1. tcp的每个连接都需要确认.
        2. 客户端到服务器和服务器到客户端的连接是独立的.
我们再想想tcp协议的特点:连接的,可靠的,全双工的,实际上tcp的三次握手正是为了保证这些特性的实现.


3.setsockopt的用法

1.closesocket（一般不会立即关闭而经历TIME_WAIT的过程）后想继续重用该socket：
BOOL bReuseaddr=TRUE;
setsockopt(s,SOL_SOCKET ,SO_REUSEADDR,(const char*)&bReuseaddr,sizeof(BOOL));


2. 如果要已经处于连接状态的soket在调用closesocket后强制关闭，不经历TIME_WAIT的过程：
BOOL bDontLinger = FALSE;
setsockopt(s,SOL_SOCKET,SO_DONTLINGER,(const char*)&bDontLinger,sizeof(BOOL));


3.在send(),recv()过程中有时由于网络状况等原因，发收不能预期进行,而设置收发时限：
int nNetTimeout=1000;//1秒
//发送时限
setsockopt(socket，SOL_S0CKET,SO_SNDTIMEO，(char *)&nNetTimeout,sizeof(int));
//接收时限
setsockopt(socket，SOL_S0CKET,SO_RCVTIMEO，(char *)&nNetTimeout,sizeof(int));


4.在send()的时候，
返回的是实际发送出去的字节(同步)
或发送到socket缓冲区的字节(异步);
系统默认的状态发送和接收一次为8688字节(约为8.5K)；
在实际的过程中发送数据和接收数据量比较大，可以设置socket缓冲区，而避免了send(),recv()不断的循环收发：
// 接收缓冲区
int nRecvBuf=32*1024;//设置为32K
setsockopt(s,SOL_SOCKET,SO_RCVBUF,(const char*)&nRecvBuf,sizeof(int));
//发送缓冲区
int nSendBuf=32*1024;//设置为32K
setsockopt(s,SOL_SOCKET,SO_SNDBUF,(const char*)&nSendBuf,sizeof(int));


5. 如果在发送数据的时，希望不经历由系统缓冲区到socket缓冲区的拷贝而影响程序的性能：
int nZero=0;
setsockopt(socket，SOL_S0CKET,SO_SNDBUF，(char *)&nZero,sizeof(nZero));


6.同上在recv()完成上述功能(默认情况是将socket缓冲区的内容拷贝到系统缓冲区)：
int nZero=0;
setsockopt(socket，SOL_S0CKET,SO_RCVBUF，(char *)&nZero,sizeof(int));


7.一般在发送UDP数据报的时候，希望该socket发送的数据具有广播特性：
BOOL bBroadcast=TRUE;
setsockopt(s,SOL_SOCKET,SO_BROADCAST,(const char*)&bBroadcast,sizeof(BOOL));


8.在client连接服务器过程中，
如果处于非阻塞模式下的socket在connect()的过程中可以设置connect()延时,
直到accpet()被呼叫(本函数设置只有在非阻塞的过程中有显著的作用，在阻塞的函数调用中作用不大)
BOOL bConditionalAccept=TRUE;
setsockopt(s,SOL_SOCKET,SO_CONDITIONAL_ACCEPT,(const char*)&bConditionalAccept,sizeof(BOOL));


9.如果在发送数据的过程中(send()没有完成，还有数据没发送)而调用了closesocket(),
以前我们一般采取的措施是"从容关闭"shutdown(s,SD_BOTH),但是数据是肯定丢失了，
如何设置让程序满足具体应用的要求(即让没发完的数据发送出去后在关闭socket)？
struct linger {
u_short l_onoff;
u_short l_linger;
};
linger m_sLinger;
m_sLinger.l_onoff=1;//(在closesocket()调用,但是还有数据没发送完毕的时候容许逗留)
// 如果m_sLinger.l_onoff=0;则功能和2.)作用相同;
m_sLinger.l_linger=5;//(容许逗留的时间为5秒)
setsockopt(s,SOL_SOCKET,SO_LINGER,(const char*)&m_sLinger,sizeof(linger));



注意：两种套接口的选项：
一种是布尔型选项，允许或禁止一种特性；
另一种是整形或结构选项。
允许一个布尔型选项，则将optval指向非零整形数；
禁止一个选项optval指向一个等于零的整形数。
对于布尔型选项，optlen应等于sizeof(int)；对其他选项，optval指向包含所需选项的整形数或结构，而optlen则为整形数或结构的长度。
SO_LINGER选项用于控制下述情况的行动：套接口上有排队的待发送数据，且closesocket()调用已执行。
参见closesocket()函数中关于SO_LINGER选项对closesocket()语义的影响。
应用程序通过创建一个linger结构来设置相应的操作特性：
   struct linger {
   int l_onoff;
   int l_linger;
};
为了允许SO_LINGER，应用程序应将l_onoff设为非零，将l_linger设为零或需要的超时值（以秒为单位），然后调用setsockopt()。
为了允许SO_DONTLINGER（亦即禁止SO_LINGER），l_onoff应设为零，然后调用setsockopt()。

缺省条件下，一个套接口不能与一个已在使用中的本地地址捆绑（参见bind()）。
但有时会需要“重用”地址。因为每一个连接都由本地地址和远端地址的组合唯一确定，所以只要远端地址不同，两个套接口与一个地址捆绑并无大碍。
为了通知套接口实现不要因为一个地址已被一个套接口使用就不让它与另一个套接口捆绑，应用程序可在bind()调用前先设置SO_REUSEADDR选项。
请注意仅在bind()调用时该选项才被解释；
故此无需（但也无害）将一个不会共用地址的套接口设置该选项，或者在bind()对这个或其他套接口无影响情况下设置或清除这一选项。

一个应用程序可以通过打开SO_KEEPALIVE选项，使得套接口实现在TCP连接情况下允许使用“保持活动”包。
一个套接口实现并不是必需支持“保持活动”，但是如果支持的话，具体的语义将与实现有关。


TCP_NODELAY选项禁止Nagle算法。
Nagle算法通过将未确认的数据存入缓冲区直到蓄足一个包一起发送的方法，来减少主机发送的零碎小数据包的数目。
但对于某些应用来说，这种算法将降低系统性能。
所以TCP_NODELAY可用来将此算法关闭。
应用程序编写者只有在确切了解它的效果并确实需要的情况下，才设置TCP_NODELAY选项，因为设置后对网络性能有明显的负面影响。
TCP_NODELAY是唯一使用IPPROTO_TCP层的选项，其他所有选项都使用SOL_SOCKET层。

如果设置了SO_DEBUG选项，套接口供应商被鼓励（但不是必需）提供输出相应的调试信息。
但产生调试信息的机制以及调试信息的形式已超出本规范的讨论范围。
setsockopt()支持下列选项。其中“类型”表明optval所指数据的类型。 