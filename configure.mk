
# sed -i 's/include "/include </g' *
# sed -i 's/\.h"/\.h>/g' *

RUN_USER=gmonitor
RUN_HOME=/home/gmonitor
INST_USER=root
INST_LIBDIR=/usr/lib
INST_HEADDIR=/usr/local/include/gmonitor
INST_BINDIR=/usr/local/bin
INST_SBINDIR=/usr/local/sbin
INST_ETCDIR=/etc/gmonitor
INST_VARDIR=/var/lib/gmonitor
INST_INCDIR=/usr/local/include
INST_LOGDIR=/var/log/gmonitor
INST_CGIDIR=/usr/lib/cgi-bin
INST_HTMLDIR=/var/www/html/dpc/

DBPATH=/var/lib/DomPiWeb
DATABASE=.DomPiWebDB.sqll3
SQL=/usr/bin/sqlite3

UPDATE_FILE=gmonitor_dompicloud_update.tar.gz
INSTALL_FILE=gmonitor_dompicloud_install.tar.gz

CP=cp
CP_UVA=cp -uva
RM=rm -f
RMR=rm -rf
MKDIR=mkdir -p
CHMOD=chmod
CHOWN=chown

GM_COMM_MSG_LEN=32768

GENERAL_DEFINE=-D __NO__DEBUG__ -D __NO_DEBUG_TCP -DGM_COMM_MSG_LEN=$(GM_COMM_MSG_LEN)
