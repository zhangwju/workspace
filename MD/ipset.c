#!/bin/sh

#downlink traffic
DOWN_SPEED=120
DOWN_MAXSPEED=125

ipset create ip-tc hash:net
ipset add ip-tc 1.1.1.0/24
ipset add ip-tc 10.29.0.0/16
ipset add ip-tc 10.29.251.64/29
ipset add ip-tc 10.88.0.0/13
ipset add ip-tc 10.105.8.0/21
ipset add ip-tc 10.105.16.0/28
ipset add ip-tc 124.196.0.0/16
ipset add ip-tc 182.238.0.0/16
ipset add ip-tc 192.168.230.0/24
ipset add ip-tc 211.148.160.0/19
ipset add ip-tc 221.129.0.0/16

#在em2创建一个根分类，句柄为1，使用htb(分层令牌桶),r2q是没有默认的root(根分类),使整个网络带宽无限制
tc qdisc add dev em2 root handle 1: htb r2q 1

#在根分类的基础上创建一个1：1的子分类,设定下载速度为15mbit最高可以是18mbit(这个值可以根据具体需求进行变更) 
tc class add dev em2 parent 1: classid 1:10 htb rate 15mbit ceil 16mkbit 

#匹配的ip走10号通道
iptables -A PREROUTING -t mangle -i em2 -m set ! --match-set ip-tc dst -j MARK --set-mark 10

iptables -t nat -D PREROUTING 1


