#!/bin/sh

function traffic_cop_all()
{
	
	echo  -e  "Interface		|		RX		|		TX		|		Time"
	all_eth=$(cat /proc/net/dev |  tr : " " | awk '{if(NR>=3) print $1}')
	for eth in $all_eth
	do
		echo $eth		
	done
}

function traffic_cop()
{
	eth=$1
	if [ "`cat /proc/net/dev | grep $eth`" == "" ]; then
		echo -e "Interface ${eth} don't exits"
		exit
	fi
	echo  -e  "Interface		|		RX		|		TX		|		Time"

	while [ "1" ]
	do

		RXpre=$(cat /proc/net/dev | grep $eth | tr : " " | awk '{print $2}')
		TXpre=$(cat /proc/net/dev | grep $eth | tr : " " | awk '{print $10}')

		sleep 1
		RXnext=$(cat /proc/net/dev | grep $eth | tr : " " | awk '{print $2}')
		TXnext=$(cat /proc/net/dev | grep $eth | tr : " " | awk '{print $10}')
		clear
		RX=$((${RXnext}-${RXpre}))
		TX=$((${TXnext}-${TXpre}))
		 
		if [ $RX -lt 1024 ];then
			RX="${RX}B/s"
		elif [ $RX -gt 1048576 ];then
			RX=$(echo $RX | awk '{print $1/1048576 "MB/s"}')
		else
			RX=$(echo $RX | awk '{print $1/1024 "KB/s"}')
		fi
		 
		if [ $TX -lt 1024 ];then
			TX="${TX}B/s"
		elif [ $TX -gt 1048576 ];then
			TX=$(echo $TX | awk '{print $1/1048576 "MB/s"}')
		else
			TX=$(echo $TX | awk '{print $1/1024 "KB/s"}')
		fi
		 
		echo -e "$eth			|		$RX	|		$TX	|	`date +'%Y-%m-%d %k:%M:%S'`"
	done
}

function help()
{    
        echo -e "Usage: traffic.sh -i <interface>"
		echo -e "Default all interface"
        echo           
        echo -e "Example: [./traffic -i eth0]"
        echo -e "Commands:"
        echo -e "interface				[physical interface]"
        echo
        echo -e "Options:"  
        echo -e "-h|-help				[help]"
       	echo -e "-i						[interface]"
}    

case $1 in    
    "-h" | "-help")
         help    
    ;;     
    "-i")
		if [ -n "$2" ]; then 
			traffic_cop $2
		else
			help
		fi
	;;
	*)
		help
		#traffic_cop_all
    ;;    
esac
