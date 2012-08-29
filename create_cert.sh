#!/bin/bash

if [ "$1" == "" ]
then
	echo "Usage: $0 <outfile>"
else
	openssl genrsa 1024 > $1.pem
	openssl req -new -x509 -nodes -sha1 -days 3650 -key $1.pem >> $1.pem
fi

