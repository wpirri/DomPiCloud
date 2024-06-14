#!/usr/bin/make -f

include configure.mk

all:
	dos2unix fixeol.sh
	./fixeol.sh
	make -C Programas

clean:
	make -C Programas clean

install:
	sed -i 's/\r//g' Config/*
	make -C Script install
	make -C Programas install

uninstall:
	make -C Script uninstall
	make -C Programas uninstall

installer:
	rm -rf $(RUN_HOME)
	mkdir -p $(RUN_HOME)
	mkdir -p $(RUN_HOME)/cgi
	mkdir -p $(RUN_HOME)/html

	cp -av Programas/Html/* $(RUN_HOME)/html/
	rm $(RUN_HOME)/html/config.php
	rm $(RUN_HOME)/html/Makefile
	cp Programas/Clientes/dompi_cloud_abmuser.cgi/dompi_cloud_abmuser.cgi $(RUN_HOME)/cgi/
	cp Programas/Clientes/dompi_cloud_alarma.cgi/dompi_cloud_alarma.cgi $(RUN_HOME)/cgi/
	cp Programas/Clientes/dompi_cloud_amazon.cgi/dompi_cloud_amazon.cgi $(RUN_HOME)/cgi/
	cp Programas/Clientes/dompi_cloud_auth.cgi/dompi_cloud_auth.cgi $(RUN_HOME)/cgi/
	cp Programas/Clientes/dompi_cloud_mobile.cgi/dompi_cloud_mobile.cgi $(RUN_HOME)/cgi/
	cp Programas/Clientes/dompi_cloud_notif.cgi/dompi_cloud_notif.cgi $(RUN_HOME)/cgi/
	cp Programas/Clientes/dompi_cloud_status.cgi/dompi_cloud_status.cgi $(RUN_HOME)/cgi/
	
	tar cvzf $(UPDATE_FILE) --files-from=update-files.lst

	cp Database/create.sql $(RUN_HOME)/
	cp Database/init.sql $(RUN_HOME)/
	cp Script/install.sh $(RUN_HOME)/
	cp Config/dompicloud.config $(RUN_HOME)/
	cp Config/funcion.tab $(RUN_HOME)/
	cp Config/funcion_parametro.tab $(RUN_HOME)/
	cp Config/server.tab $(RUN_HOME)/
	cp Config/server_parametro.tab $(RUN_HOME)/
	
	tar cvzf $(INSTALL_FILE) --files-from=install-files.lst

	#
	# *******************************************************************************"
	# * Para instalar el sistema:
	# * 
	# * copiar gmonitor_dompicloud_install.tar.gz a /home/gmonitor
	# * ejecutar con el usuario root:
	# * 
	# * cd /
	# * tar xvzf /home/gmonitor/gmonitor_dompicloud_install.tar.gz
	# * cd $(RUN_HOME)
	# * ./install.sh
	# * 
	# * Crear e inicializar la base de datos con los script sql en $(RUN_HOME)
	# * Editar el archivo /etc/dompicloud.config con los valores validos
	# * 
	# *******************************************************************************"
	# * Para actualizar el sistema:
	# * 
	# * Copiar gmonitor_dompicloud_update.tar.gz a /home/gmonitor
	# * Reiniciar el sistema (/etc/init.d/gmond restart)
	# * 
	# *******************************************************************************"
	#
