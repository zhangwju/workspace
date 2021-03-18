
struct iovec {                    /* Scatter/gather array items */
     void  *iov_base;              /* Starting address */
     size_t iov_len;               /* Number of bytes to transfer */
 };
  /* iov_base: iov_base指向数据包缓冲区，即参数buff，iov_len是buff的长度。msghdr中允许一次传递多个buff，
    以数组的形式组织在 msg_iov中，msg_iovlen就记录数组的长度 （即有多少个buff）
  */
 struct msghdr {
     void         *msg_name;       /* optional address */
     socklen_t     msg_namelen;    /* size of address */
     struct iovec *msg_iov;        /* scatter/gather array */
     size_t        msg_iovlen;     /* # elements in msg_iov */
     void         *msg_control;    /* ancillary data, see below */
     size_t        msg_controllen; /* ancillary data buffer len */
     int           msg_flags;      /* flags on received message */
 };
 /* msg_name： 数据的目的地址，网络包指向sockaddr_in, netlink则指向sockaddr_nl;
    msg_namelen: msg_name 所代表的地址长度
    msg_iov: 指向的是缓冲区数组
    msg_iovlen: 缓冲区数组长度
    msg_control: 辅助数据，控制信息(发送任何的控制信息)
    msg_controllen: 辅助信息长度
    msg_flags: 消息标识
 */
 
netlink 内核数据结构、常用宏及函数：

netlink消息类型：
#define NETLINK_ROUTE       0   /* Routing/device hook              */
#define NETLINK_UNUSED      1   /* Unused number                */
#define NETLINK_USERSOCK    2   /* Reserved for user mode socket protocols  */
#define NETLINK_FIREWALL    3   /* Unused number, formerly ip_queue     */
#define NETLINK_SOCK_DIAG   4   /* socket monitoring                */
#define NETLINK_NFLOG       5   /* netfilter/iptables ULOG */
#define NETLINK_XFRM        6   /* ipsec */
#define NETLINK_SELINUX     7   /* SELinux event notifications */
#define NETLINK_ISCSI       8   /* Open-iSCSI */
#define NETLINK_AUDIT       9   /* auditing */
#define NETLINK_FIB_LOOKUP  10  
#define NETLINK_CONNECTOR   11
#define NETLINK_NETFILTER   12  /* netfilter subsystem */
#define NETLINK_IP6_FW      13
#define NETLINK_DNRTMSG     14  /* DECnet routing messages */
#define NETLINK_KOBJECT_UEVENT  15  /* Kernel messages to userspace */
#define NETLINK_GENERIC     16
/* leave room for NETLINK_DM (DM Events) */
#define NETLINK_SCSITRANSPORT   18  /* SCSI Transports */
#define NETLINK_ECRYPTFS    19
#define NETLINK_RDMA        20
#define NETLINK_CRYPTO      21  /* Crypto layer */

#define NETLINK_INET_DIAG   NETLINK_SOCK_DIAG

#define MAX_LINKS 32 

#define NLMSG_ALIGNTO   4U
/* 宏NLMSG_ALIGN(len)用于得到不小于len且字节对齐的最小数值 */
#define NLMSG_ALIGN(len) ( ((len)+NLMSG_ALIGNTO-1) & ~(NLMSG_ALIGNTO-1) )

/* Netlink 头部长度 */
#define NLMSG_HDRLEN     ((int) NLMSG_ALIGN(sizeof(struct nlmsghdr)))

/* 计算消息数据len的真实消息长度（消息体 +　消息头）*/
#define NLMSG_LENGTH(len) ((len) + NLMSG_HDRLEN)

/* 宏NLMSG_SPACE(len)返回不小于NLMSG_LENGTH(len)且字节对齐的最小数值 */
#define NLMSG_SPACE(len) NLMSG_ALIGN(NLMSG_LENGTH(len))

/* 宏NLMSG_DATA(nlh)用于取得消息的数据部分的首地址，设置和读取消息数据部分时需要使用该宏 */
#define NLMSG_DATA(nlh)  ((void*)(((char*)nlh) + NLMSG_LENGTH(0)))

/* 宏NLMSG_NEXT(nlh,len)用于得到下一个消息的首地址, 同时len 变为剩余消息的长度 */
#define NLMSG_NEXT(nlh,len)  ((len) -= NLMSG_ALIGN((nlh)->nlmsg_len), \
                  (struct nlmsghdr*)(((char*)(nlh)) + NLMSG_ALIGN((nlh)->nlmsg_len)))

/* 判断消息是否 >len */
#define NLMSG_OK(nlh,len) ((len) >= (int)sizeof(struct nlmsghdr) && \
               (nlh)->nlmsg_len >= sizeof(struct nlmsghdr) && \
               (nlh)->nlmsg_len <= (len))

/* NLMSG_PAYLOAD(nlh,len) 用于返回payload的长度*/
#define NLMSG_PAYLOAD(nlh,len) ((nlh)->nlmsg_len - NLMSG_SPACE((len)))

netlink 内核API:

/* netlink_kernel_create内核函数用于创建 内核socket用用户态通信 */
static inline struct sock *
netlink_kernel_create(struct net *net, int unit, struct netlink_kernel_cfg *cfg)
/* net: net指向所在的网络命名空间, 一般默认传入的是&init_net(不需要定义);  定义在net_namespace.c(extern struct net init_net);
   unit：netlink协议类型
   cfg： cfg存放的是netlink内核配置参数（如下）
*/

/* optional Netlink kernel configuration parameters */
struct netlink_kernel_cfg {
    unsigned int    groups;  
    unsigned int    flags;  
    void        (*input)(struct sk_buff *skb); /* input 回调函数 */
    struct mutex    *cb_mutex; 
    void        (*bind)(int group); 
    bool        (*compare)(struct net *net, struct sock *sk);
};
netlink socket 释放函数
extern void netlink_kernel_release(struct sock *sk);

单播netlink_unicast() 和 多播netlink_broadcast()
/* 来发送单播消息 */
extern int netlink_unicast(struct sock *ssk, struct sk_buff *skb, __u32 portid, int nonblock);
/* ssk: netlink socket 
   skb: skb buff 指针
   portid： 通信的端口号
   nonblock：表示该函数是否为非阻塞，如果为1，该函数将在没有接收缓存可利用时立即返回，而如果为0，该函数在没有接收缓存可利用 定时睡眠
*/

/* 用来发送多播消息 */
extern int netlink_broadcast(struct sock *ssk, struct sk_buff *skb, __u32 portid,
                 __u32 group, gfp_t allocation);
/* ssk: 同上（对应netlink_kernel_create 返回值）、
   skb: 内核skb buff
   portid： 端口id
   group: 是所有目标多播组对应掩码的"OR"操作的合值。
   allocation: 指定内核内存分配方式，通常GFP_ATOMIC用于中断上下文，而GFP_KERNEL用于其他场合。
				这个参数的存在是因为该API可能需要分配一个或多个缓冲区来对多播消息进行clone
*/
三、netlink实例
（1）


 