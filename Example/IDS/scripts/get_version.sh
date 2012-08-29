#!/bin/bash

BaseDir="/home/xueming.li/Messenger/Example/IDS"
ConfFile="$BaseDir/etc/update.cf"

[ "$1" == "" ] && { echo -e "{\"error\":1,\"message\":\"type is null\",\"result\":{\"version\":\"\"}}"; exit; } 

Ver=`grep -E "^$1\|" $ConfFile|awk -F '|' '{print $4}'`

echo -e "{\"error\":0,\"message\":\"\",\"result\":{\"version\":$Ver}}"
