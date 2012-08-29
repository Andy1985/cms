#!/bin/bash
NLS_LANG="SIMPLIFIED CHINESE_CHINA.ZHS16GBK"
export NLS_LANG

db_conn="tapm/tapm263@192.168.188.16:1521/orcl"

./FileToDB $1 $2 $3 "$db_conn"
