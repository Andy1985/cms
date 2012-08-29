#!/bin/bash
BaseDir="/home/xueming.li/Messenger/Example/IDS"
ConfFile="$BaseDir/etc/update.cf"

[ "$1" == "" -o "$2" == "" ] && { echo -e "{\"error\":1,\"message\":\"type is null\",\"result\":\"\"}"; exit; } 

line_to_replace=`grep -E "^$1\|" $ConfFile`

[ "$line_to_replace" == "" ] && { echo -e "{\"error\":2,\"message\":\"No type!\",\"result\":\"\"}"; exit; }

version=`echo $line_to_replace|awk -F '|' '{print $4}'`
line_after_replace=`echo $line_to_replace|sed s@"|${version}"@"|$2"@g`

sed s@"$line_to_replace"@"$line_after_replace"@g $ConfFile > /tmp/conf.$$
mv /tmp/conf.$$ $ConfFile

echo -e "{\"error\":0,\"message\":\"\",\"result\":\"OK\"}"
