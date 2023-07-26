#!/usr/bin/make -f

include ../configure.mk

all: install

install:
	mkdir -p $(DBPATH)
	$(SQL) < create.sql $(DBPATH)/$(DATABASE)
	$(SQL) < init.sql $(DBPATH)/$(DATABASE)
	chown gmonitor: $(DBPATH)/$(DATABASE)
	chmod 0666 $(DBPATH)/$(DATABASE)
	
uninstall:
	$(RM) $(DBPATH)/$(DATABASE)

reinstall: uninstall install
