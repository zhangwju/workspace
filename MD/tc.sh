

tc qdisc add dev eth0 root handle 1: htb default 21
tc class add dev $INDEV parent 10: classid 10:1 htb rate ${ds}kbit ceil ${ds}kbit prio 0

tc class add dev $INDEV parent 10:1 classid 10:${IP_LAST} htb rate ${DS_MIN}kbit ceil ${DS_MAX}kbit prio 1
tc qdisc add dev $INDEV parent 10:${IP_LAST} handle ${IP_LAST}: sfq perturb 10
tc filter add dev $INDEV parent 10:0 protocol ip prio 1 u32 match ip dst ${IPADDR} flowid 10:${IP_LAST}

tc qdisc add dev $INDEV handle ffff: ingress
tc filter add dev $INDEV parent ffff: protocol ip prio 50 handle 8 fw police rate ${UPLINK}kbit burst 10k drop flowid :8

