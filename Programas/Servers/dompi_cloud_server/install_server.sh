#!/bin/sh

if [ -f /usr/local/sbin/$1 ]; then
    /usr/sbin/gmon_stop $1

    sleep 1

    cp $1 /usr/local/sbin/

    /usr/sbin/gmon_start $1
else
    cp $1 /usr/local/sbin/

    ./update-tables.sh . /var/lib/gmonitor server /usr/local/sbin/$1
    ./update-tables.sh . /var/lib/gmonitor funcion

    gmt_pid=`ps -eaf | grep -v grep | grep $SERVER_BIN/gmt | awk '{ print $2; }'`
    kill -HUP $gmt_pid > /dev/null 2>&1
fi
