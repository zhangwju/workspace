

config router 'router1'               
        option interface 'eth0'       
        option target '192.168.5.0'   
        option netmask '255.255.255.0'
        option gateway '192.168.16.1'

除了标准的netlink消息之外，rtnetlink由这些消息类型组成。
# RTM_NEWLINK, RTM_DELLINK, RTM_GETLINK
创建或者删除一个特定的网络接口，或者从一个特定的网络接口上获得信息。这些消息含有一个ifinfomsg类型的结构，紧跟在后面的是一系列的rtattr结构。

# RTM_NEWADDR, RTM_DELADDR, RTM_GETADDR
 添加,删除或者接收一个和接口相关的IP地址的信息。在linux2.2中，一个网口是可以有多个IP地址信息的。这些消息含有一个ifaddrmsg类型的结构，紧跟在后面的是一系列的rtattr结构。
 
/*用来获取ifinfomsg后面的rtattr结构*/
#define IFLA_RTA(r)  ((struct rtattr*)(((char*)(r)) + NLMSG_ALIGN(sizeof(struct ifinfomsg))))

YNOPSIS         top

       #include <asm/types.h>
       #include <linux/netlink.h>
       #include <linux/rtnetlink.h>
       #include <sys/socket.h>

       rtnetlink_socket = socket(AF_NETLINK, int socket_type,
       NETLINK_ROUTE);

       int RTA_OK(struct rtattr *rta, int rtabuflen);

       void *RTA_DATA(struct rtattr *rta);

       unsigned int RTA_PAYLOAD(struct rtattr *rta);

       struct rtattr *RTA_NEXT(struct rtattr *rta, unsigned int rtabuflen);

       unsigned int RTA_LENGTH(unsigned int length);

       unsigned int RTA_SPACE(unsigned int length);
DESCRIPTION         top

       All rtnetlink(7) messages consist of a netlink(7) message header and
       appended attributes.  The attributes should be manipulated only using
       the macros provided here.

       RTA_OK(rta, attrlen) returns true if rta points to a valid routing
       attribute; attrlen is the running length of the attribute buffer.
       When not true then you must assume there are no more attributes in
       the message, even if attrlen is nonzero.

       RTA_DATA(rta) returns a pointer to the start of this attribute's
       data.

       RTA_PAYLOAD(rta) returns the length of this attribute's data.

       RTA_NEXT(rta, attrlen) gets the next attribute after rta.  Calling
       this macro will update attrlen.  You should use RTA_OK to check the
       validity of the returned pointer.

       RTA_LENGTH(len) returns the length which is required for len bytes of
       data plus the header.

       RTA_SPACE(len) returns the amount of space which will be needed in a
       message with len bytes of data.