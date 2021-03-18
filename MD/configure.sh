#!/bin/sh
#Description: Fast Build DNS Environment 

######install webconsole
rpm -qa | grep webconsole | sed 's/^/&rpm --nodeps -e /g' | sh
cd ./software/webconsole/
sh install.sh
cd -
iptables -I INPUT -p tcp -m tcp --dport 80 -j ACCEPT

#######copy target file
cp ./tftpboot/* /var/lib/tftpboot/ -rf 
cp ./unbound.conf  /usr/local/etc/unbound/ -rf 
cp ./unbound-control /usr/local/sbin/ -rf 

unbound-control -c /usr/local/etc/unbound/unbound.conf

##########VM images
cp ./images /var/lib/libvirt/ -rf 

#########vmctl
cp ./vmctl.sh /etc/init.d/ -rf 

chkconfig --add /etc/init.d/vmctl.sh

