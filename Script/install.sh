#!/bin/sh

#
# Este Script se ejecuta después de descomprimir el archivo
# gmonitor_dompiweb_install.tar.gz 
#

GMON_USER=gmonitor
SYTEM_HOME=/home/$GMON_USER
SYTEM_VAR=/var/$GMON_USER
SYTEM_LIB=/var/lib/$GMON_USER
SYTEM_LOG=/var/log/$GMON_USER

DOCUMENT_ROOT=/var/www/html/dpc
CGI_ROOT=/usr/lib/cgi-bin/


if [ ! -x /usr/bin/mysql ]; then
    echo "No se encuentra mysql"
    echo "Se debe instalar xinetd apache2 php libapache2-mod-php default-mysql-client/default-mysql-server libcjson1"
    exit 1
fi

if [ ! -x /usr/sbin/xinetd ]; then
    echo "No se encuentra xinetd"
    echo "Se debe instalar xinetd apache2 php libapache2-mod-php default-mysql-client/default-mysql-server libcjson1"
    exit 1
fi

if [ ! -x /usr/sbin/apache2 ]; then
    echo "No se encuentra apache2"
    echo "Se debe instalar xinetd apache2 php libapache2-mod-php default-mysql-client/default-mysql-server libcjson1"
    exit 1
fi

if [ ! -x /usr/bin/php ]; then
    echo "No se encuentra php"
    echo "Se debe instalar xinetd apache2 php libapache2-mod-php default-mysql-client/default-mysql-server libcjson1"
    exit 1
fi

if [ ! -x /usr/share/doc/libapache2-mod-php ]; then
    echo "No se encuentra libapache2-mod-php"
    echo "Se debe instalar xinetd apache2 php libapache2-mod-php default-mysql-client/default-mysql-server libcjson1"
    exit 1
fi

if [ ! -x /usr/lib/x86_64-linux-gnu/libcjson.so && ! -L /usr/lib/x86_64-linux-gnu/libcjson.so ]; then
    echo "No se encuentra libcjson.so"
    echo "Se debe instalar xinetd apache2 php libapache2-mod-php default-mysql-client/default-mysql-server libcjson1"
    exit 1
fi

echo "Agregando usuario ${GMON_USER}"
useradd -d $SYTEM_LIB $GMON_USER
passwd -d $GMON_USER

echo "Creando directorios..."
# Creo los directorios necesarios
mkdir -v -p $SYTEM_VAR
chown -v $GMON_USER: $SYTEM_VAR
mkdir -v -p $SYTEM_LIB
chown -v $GMON_USER: $SYTEM_LIB
mkdir -v -p $SYTEM_LOG
chown -v $GMON_USER: $SYTEM_LOG
mkdir -v -p $DOCUMENT_ROOT
# Directorio para upload
mkdir -v -p $DOCUMENT_ROOT/admin/upload
chmod 0777 $DOCUMENT_ROOT/admin/upload
mkdir -v -p $DOCUMENT_ROOT/clients_uploads
chmod 0777 $DOCUMENT_ROOT/clients_uploads

echo "Agregando la configuracion de gmonitor y DomPiWeb..."
# Agrego los la configuración de gmonitor y DomPiWeb
cp -v $SYTEM_HOME/funcion.tab $SYTEM_LIB
cp -v $SYTEM_HOME/funcion_parametro.tab $SYTEM_LIB
cp -v $SYTEM_HOME/server.tab $SYTEM_LIB
cp -v $SYTEM_HOME/server_parametro.tab $SYTEM_LIB
chown -v -R $GMON_USER: $SYTEM_LIB
cp -v $SYTEM_HOME/dompicloud.config /etc/

echo "Agregando script de arranque..."
# Agrego el script de arranque
systemctl daemon-reload
systemctl enable gmonitor.service
#systemctl start gmonitor.service
rm /etc/init.d/gmond
ln -s /usr/local/sbin/gmond /etc/init.d/gmond

echo "Generando arbol web..."
# Copio el arbol web y cgi
cp -rv $SYTEM_HOME/html/* $DOCUMENT_ROOT
cp -rv $SYTEM_HOME/cgi/* $CGI_ROOT

echo "Configurando y reiniciando xinetd..."
# Agregando servicio a /etc/services
x=`grep gmonitor /etc/services`
if [ "X${x}" = "X" ]; then
    echo "gmonitor        5533/tcp        # Gnu-Monitor" >> /etc/services
fi
service xinetd restart

echo "Configurando y reiniciando httpd..."
# Agregando CGI al Apache
a2enmod cgi
service apache2 restart

# Limpiesa final
#rm -rv $SYTEM_HOME/*
echo "Crear e inicializar la base de datos con los script sql en ${SYTEM_HOME}".
echo "Editar el archivo /etc/dompicloud.config con los valores validos"

