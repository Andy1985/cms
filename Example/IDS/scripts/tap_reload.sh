#!/bin/bash

CMD="/home/gatewaypostfix/usr/sbin/postfix"

$CMD reload

echo -e "{\"error\":$?,\"message\":\"\",\"result\":{}}"
