#!/bin/sh

CHECK_PATH=/var/www/html/dpc/admin/upload
FILE_NAME=gmonitor_dompicloud_update.tar.gz
MOVE_TO=/home/gmonitor


check_for_updates_daemon()
{
	if [ -f $CHECK_PATH/$FILE_NAME ]; then
		mv $CHECK_PATH/$FILE_NAME $MOVE_TO/
		logger "[DOMPICLOUD] Aplicando actualizacion y reiniciando"
		/etc/init.d/gmond stop
		sleep 5
		/etc/init.d/gmond start
	fi
}

check_for_updates_loop()
{
	while true
       	do
		check_for_updates_daemon
		sleep 1
	done
}

check_for_updates_loop &






