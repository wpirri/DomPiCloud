#!/bin/sh

if [ -f $1/$2 ]; then
    $1/gmon_stop $2

    sleep 1

    cp $2 $1/

    #$1/gmon_start $2
else
    cp $2 $1/

    ./update-tables.sh $3 server $1/$2
    ./update-tables.sh $3 funcion

    gmt_pid=`ps -eaf | grep -v grep | grep $SERVER_BIN/gmt | awk '{ print $2; }'`
    kill -HUP $gmt_pid > /dev/null 2>&1
fi
