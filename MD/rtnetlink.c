Rtnetlink 允许对内核路由表进行读和更改，它用于内核与各个子系统之间（路由子系统、IP地址、链接参数等）的通信，用户空间可以通过NET_LINK_ROUTER socket
与内核进行通信。该过程基于netlink消息进行

一些rtnetlink消息在初始头后有一些可选属性
下面是该属性的结构：
struct rtattr {
    unsigned short rta_len;    /* Length of option */
    unsigned short rta_type;   /* Type of option */
    /* Data follows */
};
操作这些属性只可以用RTA_*这些宏来造作
/* Macros to handle rtattributes */
    
#define RTA_ALIGNTO 4
#define RTA_ALIGN(len) ( ((len)+RTA_ALIGNTO-1) & ~(RTA_ALIGNTO-1) )
#define RTA_OK(rta,len) ((len) >= (int)sizeof(struct rtattr) && \
             (rta)->rta_len >= sizeof(struct rtattr) && \
             (rta)->rta_len <= (len))
#define RTA_NEXT(rta,attrlen)   ((attrlen) -= RTA_ALIGN((rta)->rta_len), \
                 (struct rtattr*)(((char*)(rta)) + RTA_ALIGN((rta)->rta_len)))
#define RTA_LENGTH(len) (RTA_ALIGN(sizeof(struct rtattr)) + (len))
#define RTA_SPACE(len)  RTA_ALIGN(RTA_LENGTH(len))
#define RTA_DATA(rta)   ((void*)(((char*)(rta)) + RTA_LENGTH(0)))
#define RTA_PAYLOAD(rta) ((int)((rta)->rta_len) - RTA_LENGTH(0))

/*
Rtnetlink 由下面这些消息类型构成（新加在标准的netlink消息上）
#RTM_NEWLINK, RTM_DELLINK, RTM_GETLINK
创建或者删除一个特定的网络接口，或者从一个特定的网络接口上获得信息。这些消息含有一个ifinfomsg类型的结构，紧跟在后面的是一系列的rtattr结构。
/*****************************************************************
 *      Link layer specific messages.
 ****/

/* struct ifinfomsg
 * passes link level specific information, not dependent
 * on network protocol.
 */

 struct ifinfomsg {
    unsigned char  ifi_family; /* AF_UNSPEC */
    unsigned short ifi_type;   /* Device type */
    int            ifi_index;  /* Interface index */
    unsigned int   ifi_flags;  /* Device flags  */
    unsigned int   ifi_change; /* change mask */
};
/*
* ifi_family: 接口地址类型
* ifi_type: 设备类型
* ifi_index: 是结构唯一的索引
* ifi_flags:  设备标志，可以看netdevice 结构
* ifi_change: 保留值，通常设置为0xFFFFFFFF
*/

ifi_type代表硬件设备的类型：
	ARPHRD_ETHER                   10M以太网
    ARPHRD_PPP                     PPP拨号
    ARPHRDLOOPBACK                 环路设备
	
ifi_flags包含设备的一些标志：
    IFF_UP                            接口正在运行
    IFF_BROADCAST                     有效的广播地址集
    IFF_DEBUG                         内部调试标志
    IFF_LOOPBACK                      这是自环接口
    IFF_POINTOPOINT                   这是点到点的链路设备
    IFF_RUNNING                       资源已分配
    IFF_NOARP                         无arp协议，没有设置第二层目的地址
    IFF_PROMISC                       接口为杂凑(promiscuous)模式
    IFF_NOTRAILERS                    避免使用trailer
    IFF_ALLMULTI                      接收所有组播(multicast)报文
    IFF_MASTER                        主负载平衡群(bundle)
    IFF_SLAVE                         从负载平衡群(bundle)
    IFF_MULTICAST                     支持组播(multicast)
    IFF_PORTSEL                       可以通过ifmap选择介质(media)类型
    IFF_AUTOMEDIA                     自动选择介质
    IFF_DYNAMIC                       接口关闭时丢弃地址	

            Routing attributes（rtattr部分属性，rta_type）

rta_type         value type         description
 ──────────────────────────────────────────────────────────
IFLA_UNSPEC      -                  未说明，未指定的数据
IFLA_ADDRESS     hardware address   L2硬件地址
IFLA_BROADCAST   hardware address   L2广播地址.
IFLA_IFNAME      asciiz string      char型设备名.
IFLA_MTU         unsigned int       MTU of the device.
IFLA_LINK        int                Link type.
IFLA_QDISC       asciiz string      Queueing discipline.
IFLA_STATS       see below          struct rtnl_link_stats的设备信息

//用来获取ifinfomsg后面的rtattr结构
#define IFLA_RTA(r)  ((struct rtattr*)(((char*)(r)) + NLMSG_ALIGN(sizeof(struct ifinfomsg))))

# RTM_NEWADDR, RTM_DELADDR, RTM_GETADDR
 添加,删除或者接收一个和接口相关的IP地址的信息。在linux2.2中，一个网口是可以有多个IP地址信息的。
 这些消息含有一个ifaddrmsg类型的结构，紧跟在后面的是一系列的rtattr结构。
struct ifaddrmsg {
    unsigned char ifa_family;    /* Address type */
    unsigned char ifa_prefixlen; /* Prefixlength of address */
    unsigned char ifa_flags;     /* Address flags */
    unsigned char ifa_scope;     /* Address scope */
    int           ifa_index;     /* Interface index */
              };
/* 
* ifa_family: 地址类型（通常为AF_INET or AF_INET6)）
* ifa_prefixlen: 地址的地址掩码长度，如果改地址定义在这个family
* ifa_flags:
* ifa_scope: 地址的作用域
* ifa_index:  接口索引与接口地址关联
*/
               Attributes
rta_type        value type             description
─────────────────────────────────────────────────────────────
IFA_UNSPEC      -                      unspecified.
IFA_ADDRESS     raw protocol address   接口地址 interface address
IFA_LOCAL       raw protocol address   本地地址 local address
IFA_LABEL       asciiz string          接口名称 name of the interface
IFA_BROADCAST   raw protocol address   广播 broadcast address.
IFA_ANYCAST     raw protocol address   anycast address
IFA_CACHEINFO   struct ifa_cacheinfo   Address information.


/******************************************************************************
 *      Definitions used in routing table administration.
 ****/
RTM_NEWROUTE, RTM_DELROUTE, RTM_GETROUTE
创建，删除或者获取网络设备的路由信息；这些消息包含一个rtmsg结构，其后跟数目可选的rtattr结构。
对于RTM_GETROUTE，设置rtm_dst_len以及rtm_src_len为0表示获取指定路由表的所有条目（entries）。
其它的成员，除了rtm_table、rtm_protocol，0是通配符
struct rtmsg {
    unsigned char       rtm_family;
    unsigned char       rtm_dst_len;
    unsigned char       rtm_src_len;
    unsigned char       rtm_tos;

    unsigned char       rtm_table;  /* Routing table id */
    unsigned char       rtm_protocol;   /* Routing protocol; see below  */
    unsigned char       rtm_scope;  /* See below */
    unsigned char       rtm_type;   /* See below    */

    unsigned        rtm_flags;
};

rtm_type          Route type
───────────────────────────────────────────────────────────
RTN_UNSPEC        unknown route				/*位置路由*/
RTN_UNICAST       a gateway or direct route  /* 网关或直连路由 */
RTN_LOCAL         a local interface route    /* 本地接口路由 */
RTN_BROADCAST     a local broadcast route (sent as a broadcast)  /* 本地广播式接收，发送 */
RTN_ANYCAST       a local broadcast route (sent as a unicast)    /* 本地单播路由 */
RTN_MULTICAST     a multicast route			/* 多播路由 */
RTN_BLACKHOLE     a packet dropping route	/* 丢弃 */
RTN_UNREACHABLE   an unreachable destination /* 目标不可达 */
RTN_PROHIBIT      a packet rejection route	/* 拒绝 */
RTN_THROW         continue routing lookup in another table /* 不在本表 */ 
RTN_NAT           a network address translation rule		/* nat */
RTN_XRESOLVE      refer to an external resolver (not implemented)

rtm_protocol      Route origin. 
───────────────────────────────────────
RTPROT_UNSPEC     unknown
RTPROT_REDIRECT   by an ICMP redirect (currently unused)   /* 通过icmp转发建立路由 （目前没用）*/
RTPROT_KERNEL     by the kernel 	/* 通过内核建立路由 */
RTPROT_BOOT       during boot   	/* 启动时建立路由 */
RTPROT_STATIC     by the administrator  /* 管理员建立 */

rtm_scope is the distance to the destination:

RT_SCOPE_UNIVERSE   global route  
RT_SCOPE_SITE       interior route in the local autonomous system
RT_SCOPE_LINK       route on this link
RT_SCOPE_HOST       route on the local host
RT_SCOPE_NOWHERE    destination doesn't exist 

/* 用户可用范围 */
RT_SCOPE_UNIVERSE and RT_SCOPE_SITE are available to the user.

The rtm_flags have the following meanings:

RTM_F_NOTIFY     if the route changes, notify the user via rtnetlink     
RTM_F_CLONED     route is cloned from another route 
RTM_F_EQUALIZE   a multipath equalizer (not yet implemented)

rtm_table specifies the routing table

RT_TABLE_UNSPEC    an unspecified routing table /* 0 未指定的表 */
RT_TABLE_DEFAULT   the default table			/* 253 默认表 */
RT_TABLE_MAIN      the main table				/* 254 main 表 */	
RT_TABLE_LOCAL     the local table				/* 255 local 表 */

//用户可以使用 RT_TABLE_UNSPEC 到 RT_TABLE_DEFAULT 之间的任意值

            Attributes

rta_type        value type         description
──────────────────────────────────────────────────────────────
RTA_UNSPEC      -                  ignored.
RTA_DST         protocol address   Route destination address.   /* 目的 */
RTA_SRC         protocol address   Route source address.		/* 源地址 */
RTA_IIF         int                Input interface index.		/* 输入设备 index */
RTA_OIF         int                Output interface index.
RTA_GATEWAY     protocol address   The gateway of the route		/* 网关 */
RTA_PRIORITY    int                Priority of route.			/* 优先级 */
RTA_PREFSRC
RTA_METRICS     int                Route metric					/* 路由metric 值*/
RTA_MULTIPATH
RTA_PROTOINFO
RTA_FLOW
RTA_CACHEINFO
