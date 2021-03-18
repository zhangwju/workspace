#!/bin/sh

#device name
DEV=em2

#downlink speeds 
DOWNLINK=1000 #MAX bandwith
DS_MIN=10
DS_MAX=12

#tc rules
tc qdisc del dev $DEV root
tc qdisc add dev $DEV root handle 10: htb default 256

#create parent class
tc class add dev $DEV parent 10: classid 10:1 htb rate ${DOWNLINK}mbit ceil ${DOWNLINK}mbit prio 0
#downlink control
tc class add dev $DEV parent 10:1 classid 10:12 htb rate ${DS_MIN}mbit ceil ${DS_MAX}mbit prio 1
tc qdisc add dev $DEV parent 10:12 handle 12: sfq perturb 10
tc filter add dev $DEV parent 10:0 protocol ip prio 1 u32 match ip dst 103.248.100.3 flowid 10:12


eg:2

#device name
DEV=em2
#parent number 1:
tc qdisc add dev eth0 root handle 1: htb r2q 1
#child 1:1
tc class add dev eth0 parent 1: classid 1:1 htb rate 10240mbit ceil 10240mbit
#interactive traffic
tc class add dev eth0 parent 1:1 classid 1:10 htb rate 10mbit ceil 10mbit prio 0
#
tc qdisc add dev eth0 parent 1:10 handle 3320: sfq perturb 10

#
tc filter add dev eth0 parent 1:0 protocol ip handle 10 fw flowid 1:10




