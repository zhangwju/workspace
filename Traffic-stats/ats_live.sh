#!/bin/sh

REDIS="127.0.0.1"
PORT=6379
KEY_STR="ATS_IP:"

ip_list=(
"xxx.xxx.xxx.xxx"
)

function ats_help()
{
	echo "xxxxx"
}

function ping_test()
{
	i=0
	IP="$1"
	while [ $i -lt 2 ]; do
	    ping -c 1 -W 1 $IP 1>/dev/null 2>&1
	    if [ $? -eq 0 ]; then
	        return 0
	    fi  
	    let i+=1
	done
	return 1
}

function loop_detach()
{
	a=0
	len=${#ip_list[@]}
	while [ $a -lt $len ]; do
		ping_test ${ip_list[$a]}
		if [ $? -eq 0 ]; then
			if [ "`redis-cli -p $PORT EXISTS ${KEY_STR}${ip_list[$a]}`" == "0" ]; then
				redis-cli -p $PORT SET ${KEY_STR}${ip_list[$a]} $a 1>/dev/null 2>&1
			fi
		else
			redis-cli -p $PORT DEL ${KEY_STR}${ip_list[$a]}  1>/dev/null 2>&1
		fi
		a=$(($a + 1))
	done
}

function main()
{
	while [ 1 ]; do
		loop_detach
		sleep 5
	done
}

case $1 in 
	"-h" | "-help")
		ats_help
	;;
	*)
		main
	;;	
esac
