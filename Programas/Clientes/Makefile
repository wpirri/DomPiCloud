#!/usr/bin/make -f

all:
	make -C dompi_cloud_notif.cgi
	make -C dompi_cloud_status.cgi
	make -C mobile.cgi
	make -C auth.cgi

clean:
	make -C dompi_cloud_notif.cgi clean
	make -C dompi_cloud_status.cgi clean
	make -C mobile.cgi clean
	make -C auth.cgi clean

install:
	make -C dompi_cloud_notif.cgi install
	make -C dompi_cloud_status.cgi install
	make -C mobile.cgi install
	make -C auth.cgi install

uninstall:
	make -C dompi_cloud_notif.cgi uninstall
	make -C dompi_cloud_status.cgi uninstall
	make -C mobile.cgi uninstall
	make -C auth.cgi uninstall
