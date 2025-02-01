#!/bin/sh

OBJ=$1
TMP=$2
SBIN=$3
VAR=$4

if [ -f $SBIN/$OBJ ]; then
    $SBIN/gmon_stop $OBJ
    sleep 1
    cp $TMP/$OBJ $SBIN/
    sleep 1
    $SBIN/gmon_start $OBJ || echo "Fallo"
else
    cp $TMP/$OBJ $SBIN/
    sleep 1
    ./update-tables.sh $VAR server $SBIN/$OBJ || echo "Fallo"
    ./update-tables.sh $VAR funcion || echo "Fallo"

    gmt_pid=`ps -eaf | grep -v grep | grep $SBIN/gmt | awk '{ print $2; }'`
    if [ "X${gmt_pid}" != "X" ]; then
        kill -HUP $gmt_pid > /dev/null 2>&1
    fi
fi
