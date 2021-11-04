#!/usr/bin/make -f

DBPATH=/var/lib/DomPiWeb
DATABASE=.DomPiWebDB.sqll3
SQL=/usr/bin/sqlite3

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
