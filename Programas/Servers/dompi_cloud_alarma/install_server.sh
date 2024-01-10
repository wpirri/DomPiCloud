#!/bin/sh

if [ -f $1/$2 ]; then
    $1/gmon_stop $2
    sleep 1
    cp $2 $1/
    sleep 1
    $1/gmon_start $2 || echo "Fallo"
else
    cp $2 $1/
    sleep 1
    ./update-tables.sh $3 server $1/$2 || echo "Fallo"
    ./update-tables.sh $3 funcion || echo "Fallo"

    gmt_pid=`ps -eaf | grep -v grep | grep $SERVER_BIN/gmt | awk '{ print $2; }'`
    kill -HUP $gmt_pid > /dev/null 2>&1
fi
