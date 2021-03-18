#ip 命令详解：

ip命令的用法如下：   
/* 
　　ip [OPTIONS] OBJECT [COMMAND [ARGUMENTS]]    
　　4.1 ip link set--改变设备的属性. 缩写：set、s    
　　示例1：up/down 起动／关闭设备。    
　　# ip link set dev eth0 up    
　　这个等于传统的 # ifconfig eth0 up(down)    
　　示例2：改变设备传输队列的长度。    
　　参数:txqueuelen NUMBER或者txqlen NUMBER    
　　# ip link set dev eth0 txqueuelen 100    
　　示例3：改变网络设备MTU(最大传输单元)的值。    
　　# ip link set dev eth0 mtu 1500    
　　示例4： 修改网络设备的MAC地址。    
　　参数: address LLADDRESS    
　　# ip link set dev eth0 address 00:01:4f:00:15:f1
*/  

 /*
ip rule 命令：

    Usage: ip rule [ list | add | del ] SELECTOR ACTION （add 添加；del 删除； llist 列表）
    SELECTOR := [ from PREFIX 数据包源地址] [ to PREFIX 数据包目的地址] [ tos TOS 服务类型][ dev STRING 物理接口] [ pref NUMBER ] [fwmark MARK iptables 标签]
    ACTION := [ table TABLE_ID 指定所使用的路由表] [ nat ADDRESS 网络地址转换][ prohibit 丢弃该表| reject 拒绝该包| unreachable 丢弃该包]
    [ flowid CLASSID ]
    TABLE_ID := [ local | main | default | new | NUMBER ]
*/

 linux 系统中，可以自定义从 1－252个路由表，其中，linux系统维护了4个路由表：

 #0#表： 系统保留表
 #253#表： defulte table 没特别指定的默认路由都放在改表
 #254#表： main table 没指明路由表的所有路由放在该表
 #255#表： locale table 保存本地接口地址，广播地址、NAT地址 由系统维护，用户不得更改
	
/*ip link */
#ip link show 
#ip link set dev eth0 up(down) ------>ifconfig eth0 up(down)
#ip link set dev eth0 promisc on
#ip link set dev eth0 mtu 1412

/*ip address */
#ip address show 
#ip address show dev eth0
#ip address add 192.168.99.37/24 brd + dev eth0
#ip address add 192.168.99.37/24 brd + dev eth0 label eth0:0
#ip address flush
#ip address flush eth0
#ip addr add 192.168.0.193/24 dev wlan0
#ip addr del 192.168.0.193/24 dev wlan0

/*ip router */
#ip router show
#ip route add default via 192.168.0.196
#

/* ip rule */