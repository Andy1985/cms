#!/bin/bash

NAME=`hostname`
IP=`ifconfig eth0 | grep 'inet addr' | awk '{ print $2; }' | awk -F':' '{ print $2; }'`
MASK=`ifconfig eth0 | grep 'inet addr' | awk '{ print $4; }' | awk -F':' '{ print $2; }'`

if [ "$1" = "" ];then
	echo -e "{\"error\":0,\"message\":\"\",\"result\":{\"name\":\"$NAME\",\"ip\":\"$IP\",\"mask\":\"$MASK\"}}"
elif [ "$1" = "name" ];then
	echo -e "{\"error\":0,\"message\":\"\",\"result\":{\"name\":\"$NAME\"}}"
elif [ "$1" = "ip" ];then
	echo -e "{\"error\":0,\"message\":\"\",\"result\":{\"ip\":\"$IP\",\"mask\":\"$MASK\"}}"
else
	echo -e "{\"error\":1,\"message\":\"????“$0”????!\",\"result\":{}}"
fi
