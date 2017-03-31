#!/bin/sh
######################################################################
# Notice:                                                            #
#       If enable qos, ip limit should disabled,                     #   
#       or disable ip limit while qos enable.                        #
######################################################################

DEV="`nvram get wan.wan_if`"
INDEV="`nvram get lan.interface`"
QOS_ENABLE="`nvram get qos.qos_enable`"
IP_LIMIT_ENABLE="`nvram get qos.ip_limit_enable`"
BANDWIDTH="`nvram get qos.bandwidth`"
[ -z $BANDWIDTH ] && BANDWIDTH=4
let UPLINK=${BANDWIDTH}*1000
let DOWNLINK=${BANDWIDTH}*1000
DS_LMT=$(nvram get qos.lmt_all|cut -d, -f2)
US_LMT=$(nvram get qos.lmt_all|cut -d, -f1)


run_tc()
{
let up10=${UPLINK}*1/10
let up30=${UPLINK}*3/10
let up50=${UPLINK}*5/10
let up80=${UPLINK}*8/10
let up90=${UPLINK}*9/10

let ds10=${DOWNLINK}*1/10
let ds30=${DOWNLINK}*3/10
let ds50=${DOWNLINK}*5/10
let ds80=${DOWNLINK}*8/10
let ds90=${DOWNLINK}*9/10

tc qdisc add dev $DEV root handle 1: htb default 256
tc qdisc add dev $INDEV root handle 10: htb default 256

tc class add dev $INDEV parent 10: classid 10:1 htb rate ${ds90}kbit ceil ${ds90}kbit prio 0
tc class add dev $DEV parent 1:0 classid 1:1 htb rate ${up90}kbit ceil ${up90}kbit prio 0

if [ $IP_LIMIT_ENABLE -eq 1 -a $QOS_ENABLE -ne 1 -a $DS_LMT -eq 0 -a $US_LMT -eq 0 ]
then
	nvram list qos.limit_info > /tmp/ip_limit.lst
	while read line
	do
		IPADDR="`echo $line | cut -d, -f1`"

		IP_LAST=`echo $IPADDR | cut -d. -f4`
        US_MAX_BYTES=`echo $line | cut -d, -f2`
		US_MAX=`expr 8 \* $US_MAX_BYTES`
        US_MIN=`echo $US_MAX 8 | awk '{printf "%d", $1/$2}'`
        DS_MAX_BYTES=`echo $line | cut -d, -f3`
		DS_MAX=`expr 8 \* $DS_MAX_BYTES`
                DS_MIN=`echo $DS_MAX 8 | awk '{printf "%d", $1/$2}'`
		
		#Upstream limit
		###############
		if [ $US_MAX_BYTES -ne 0 ]; then
			tc class add dev $DEV parent 1:1 classid 1:${IP_LAST} htb rate ${US_MIN}kbit ceil ${US_MAX}kbit prio 1
			tc qdisc add dev $DEV parent 1:${IP_LAST} handle ${IP_LAST}: sfq perturb 10
			tc filter add dev $DEV parent 1:0 protocol ip prio 1 handle ${IP_LAST} fw classid 1:${IP_LAST}
        fi

		#Downstream limit
		#################
		if [ $DS_MAX_BYTES -ne 0 ]; then
			tc class add dev $INDEV parent 10:1 classid 10:${IP_LAST} htb rate ${DS_MIN}kbit ceil ${DS_MAX}kbit prio 1
			tc qdisc add dev $INDEV parent 10:${IP_LAST} handle ${IP_LAST}: sfq perturb 10
			tc filter add dev $INDEV parent 10:0 protocol ip prio 1 u32 match ip dst ${IPADDR} flowid 10:${IP_LAST}
		fi
	done < /tmp/ip_limit.lst
	rm /tmp/ip_limit.lst
fi

if [ $QOS_ENABLE -eq 0 -a $IP_LIMIT_ENABLE -eq 1 -a $DS_LMT -ne 0 -o $US_LMT -ne 0 ]; then
	arp -n| grep $INDEV | grep -v incomplete |cut -d\( -f2 | cut -d\) -f1 > /tmp/cpe_active
	US_MAX_BYTES=$US_LMT
	US_MAX=`expr 8 \* $US_MAX_BYTES`
	US_MIN=`echo $US_MAX 8 | awk '{printf "%d", $1/$2}'`
	DS_MAX_BYTES=$DS_LMT
	DS_MAX=`expr 8 \* $DS_MAX_BYTES`
	DS_MIN=`echo $DS_MAX 8 | awk '{printf "%d", $1/$2}'`

	while read line 
	do
		IPADDR=$line
        IP_LAST=`echo $IPADDR | cut -d. -f4`	

		# Single ip limit schedule has a higher priority than unified
		#############################################################
		unset SING_US
        unset SING_DS
        SING_LMT=$(nvram list qos.limit_info| grep "$IPADDR,")
		if [ ${#SING_LMT} -ne 0 ]; then
			SING_US=$(echo $SING_LMT|cut -d, -f2)
			SING_DS=$(echo $SING_LMT|cut -d, -f3)
			US_MAX_BYTES=$SING_US
			US_MAX=`expr 8 \* $US_MAX_BYTES`
			US_MIN=`echo $US_MAX 8 | awk '{printf "%d", $1/$2}'`
			DS_MAX_BYTES=$SING_DS
			DS_MAX=`expr 8 \* $DS_MAX_BYTES`
			DS_MIN=`echo $DS_MAX 8 | awk '{printf "%d", $1/$2}'`			

		fi

		#Upstream limit
		###############
		if [ $US_MAX_BYTES -ne 0 ]; then
			tc class add dev $DEV parent 1:1 classid 1:${IP_LAST} htb rate ${US_MIN}kbit ceil ${US_MAX}kbit prio 1
			tc qdisc add dev $DEV parent 1:${IP_LAST} handle ${IP_LAST}: sfq perturb 10
			tc filter add dev $DEV parent 1:0 protocol ip prio 1 handle ${IP_LAST} fw classid 1:${IP_LAST}

	    fi

		#Downstream limit
		#################
		if [ $DS_MAX_BYTES -ne 0 ]; then
			tc class add dev $INDEV parent 10:1 classid 10:${IP_LAST} htb rate ${DS_MIN}kbit ceil ${DS_MAX}kbit prio 1
            tc qdisc add dev $INDEV parent 10:${IP_LAST} handle ${IP_LAST}: sfq perturb 10
            tc filter add dev $INDEV parent 10:0 protocol ip prio 1 u32 match ip dst ${IPADDR} flowid 10:${IP_LAST}
		fi
	done < /tmp/cpe_active
	rm /tmp/cpe_active
fi

if [ $QOS_ENABLE -eq 1 -a $IP_LIMIT_ENABLE -ne 1 ]; then
	tc class add dev $DEV parent 1:1 classid 1:11 htb rate ${up30}kbit ceil ${up80}kbit prio 1
	tc class add dev $DEV parent 1:1 classid 1:12 htb rate ${up50}kbit ceil ${up80}kbit prio 2
	tc class add dev $DEV parent 1:1 classid 1:13 htb rate ${up10}kbit ceil ${up50}kbit prio 3

	tc qdisc add dev $DEV parent 1:11 handle 111: sfq perturb 5
	tc qdisc add dev $DEV parent 1:12 handle 112: sfq perturb 5
	tc qdisc add dev $DEV parent 1:13 handle 113: sfq perturb 5

	tc filter add dev $DEV parent 1:0 protocol ip prio 1 handle 1 fw classid 1:11
	tc filter add dev $DEV parent 1:0 protocol ip prio 2 handle 2 fw classid 1:12
	tc filter add dev $DEV parent 1:0 protocol ip prio 3 handle 3 fw classid 1:13
fi

tc qdisc add dev $INDEV handle ffff: ingress
tc qdisc add dev $DEV handle ffff: ingress

tc filter add dev $INDEV parent ffff: protocol ip prio 50 handle 8 fw police rate ${UPLINK}kbit burst 10k drop flowid :8
tc filter add dev $DEV parent ffff: protocol ip prio 50 handle 8 fw police rate ${DOWNLINK}kbit burst 10k drop flowid :8

}

stop_tc ()
{
	tc qdisc del dev $DEV root 
	tc qdisc del dev $INDEV root
	tc qdisc del dev $DEV handle ffff: ingress
	tc qdisc del dev $INDEV handle ffff: ingress
}

add_tc ()
{
	IPADDR=$1
	SING_US=0
	SING_DS=0
	IPL=$(echo $IPADDR| cut -d. -f4)
	IS_US_RULE_EXIT=$(tc class show dev $DEV|grep "$IPL:")
	IS_DS_RULE_EXIT=$(tc class show dev $INDEV|grep "$IPL:")
	SING_LMT=$(nvram list qos.limit_info| grep "$IPADDR,")

	if [ ${#SING_LMT} -ne 0 ]; then        
		SING_US=$(echo $SING_LMT|cut -d, -f2)
		SING_DS=$(echo $SING_LMT|cut -d, -f3)
	fi

	if [ ${#IS_US_RULE_EXIT} -eq 0 ]; then
		if [ $SING_US -ne 0 ]; then
			US_MAX_BYTES=$SING_US
		else
			US_MAX_BYTES=$US_LMT
		fi
	else
		exit 0
	fi

	if [ ${#IS_DS_RULE_EXIT} -eq 0 ]; then
		if [ $SING_DS -ne 0 ]; then
			DS_MAX_BYTES=$SING_DS
		else
			DS_MAX_BYTES=$DS_LMT
		fi	
	else 
		exit 0
	fi

	US_MAX=`expr 8 \* $US_MAX_BYTES`
	US_MIN=`echo $US_MAX 8 | awk '{printf "%d", $1/$2}'`
	DS_MAX=`expr 8 \* $DS_MAX_BYTES`
	DS_MIN=`echo $DS_MAX 8 | awk '{printf "%d", $1/$2}'`

	#Upstream limit
	###############
	if [ $US_MAX_BYTES -ne 0 ]; then
		tc class add dev $DEV parent 1:1 classid 1:${IPL} htb rate ${US_MIN}kbit ceil ${US_MAX}kbit prio 1
		tc qdisc add dev $DEV parent 1:${IPL} handle ${IPL}: sfq perturb 10
		tc filter add dev $DEV parent 1:0 protocol ip prio 1 handle ${IPL} fw classid 1:${IPL}
	fi

	#Downstream limit
	#################
	if [ $DS_MAX_BYTES -ne 0 ]; then
		tc class add dev $INDEV parent 10:1 classid 10:${IPL} htb rate ${DS_MIN}kbit ceil ${DS_MAX}kbit prio 1
		tc qdisc add dev $INDEV parent 10:${IPL} handle ${IPL}: sfq perturb 10
		tc filter add dev $INDEV parent 10:0 protocol ip prio 1 u32 match ip dst ${IPADDR} flowid 10:${IPL}
	fi
	
}

case $1 in
	"add")
		[ ! -f /tmp/qos.run ] && exit
		if [ ${QOS_ENABLE} -eq 0 -a ${IP_LIMIT_ENABLE} -eq 1 -a $DS_LMT -ne 0 -o $US_LMT -ne 0 ]; then
			IPA=$2
			add_tc $IPA
		fi
		;;
	"start")
	    [ -f /tmp/qos.run ] && exit 
	    if [ ${QOS_ENABLE} -eq 1 -o ${IP_LIMIT_ENABLE} -eq 1 ]; then
	    	run_tc
	    	touch /tmp/qos.run
	    fi
	    ;;
	"stop")
		[ ! -f /tmp/qos.run ] && exit 
	    	stop_tc
		rm -rf /tmp/qos.run
		echo stop
		;;
	"restart")
		if [ ${QOS_ENABLE} -eq 1 -o ${IP_LIMIT_ENABLE} -eq 1 ]; then
			stop_tc
			rm -rf /tmp/qos.run
			sleep 1
			run_tc
			touch /tmp/qos.run
		fi
		;;
	*)
		;;
esac
