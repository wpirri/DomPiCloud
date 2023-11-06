#!/bin/sh

DATE=`date "+%Y%m%d%H%M%S"`
DATABASE="DB_DOMPICLOUD"
FILENAME="/var/log/gmonitor/backup-mysql-${DATABASE}-${DATE}.sql"

mysqldump --add-drop-database --databases $DATABASE > $FILENAME


