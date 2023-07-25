#!/usr/bin/make -f

all:
	make -C dompi_cloud_server
	make -C dompi_amazon_server
	make -C dompi_cloud_abm

clean:
	make -C dompi_cloud_server clean
	make -C dompi_amazon_server clean
	make -C dompi_cloud_abm clean

install:
	make -C dompi_cloud_server install
	make -C dompi_amazon_server install
	make -C dompi_cloud_abm install

uninstall:
	make -C dompi_cloud_server uninstall
	make -C dompi_amazon_server uninstall
	make -C dompi_cloud_abm uninstall 
