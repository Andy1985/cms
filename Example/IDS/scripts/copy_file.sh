#/bin/bash

BaseDir="/home/xueming.li/Messenger/Example/IDS"
ConfFile="$BaseDir/etc/update.cf"
FileTransferPath="$BaseDir/ids-data"

[ "$1" == "" -o "$2" == "" ] && { echo -e "{\"error\":1,\"message\":\"type is null\",\"result\":\"\"}"; exit; } 

type_line=`grep -E "^$1\|" $ConfFile`
[ "$type_line" == "" ] && { echo -e "{\"error\":2,\"message\":\"No type!\",\"result\":\"\"}"; exit; }

filename=`echo $type_line|awk -F '|' '{print $2}'`
to_path=`echo $type_line|awk -F '|' '{print $3}'`
version=`echo $type_line|awk -F '|' '{print $4}'`

[ ! -f $FileTransferPath/$2 ] && { echo -e "{\"error\":3,\"message\":\"No File!\",\"result\":\"\"}"; exit; }

mv $FileTransferPath/$2 $to_path/$filename
if [ $2 -eq 7 ];then 
	/home/gatewaypostfix/etc/postfix/compilekeys.sh 1>&2
else
	cd $to_path && /home/gatewaypostfix/usr/sbin/postmap $filename 1>&2
fi

$BaseDir/scripts/set_version.sh $1 $2 1>&2

echo -e "{\"error\":0,\"message\":\"\",\"result\":\"OK\"}"
