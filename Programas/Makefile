#!/usr/bin/make -f

all:
	make -C Clientes
	make -C Servers

clean:
	make -C Clientes clean
	make -C Servers clean

install:
	make -C Clientes install
	make -C Servers install
	make -C Html install 

uninstall:
	make -C Clientes uninstall
	make -C Servers uninstall
	make -C Html uninstall 
