#!/bin/sh

DATE=`date "+%Y%m%d%H%M%S"`
DATABASE="DB_DOMPICLOUD"
FILENAME="/var/log/gmonitor/backup-mysql-${DATABASE}-${DATE}.sql"

/usr/bin/mysqloptimize DB_DOMPICLOUD
/usr/bin/mysqldump --add-drop-database --databases $DATABASE > $FILENAME


