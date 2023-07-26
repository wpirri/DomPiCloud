#!/usr/bin/make -f

include ../../configure.mk

DOCUMENT_ROOT=

all: none

none:
	echo "implementado solamente make install"

install:
	$(MKDIR) $(INST_HTMLDIR)
	$(CP_UVA) * $(INST_HTMLDIR)
	$(MKDIR) $(INST_HTMLDIR)/admin/upload
	$(CHMOD) 0777 $(INST_HTMLDIR)/admin/upload

uninstall:
	$(RMR) $(INST_HTMLDIR)

reinstall:
	$(RMR) $(INST_HTMLDIR)
	$(MKDIR) $(INST_HTMLDIR)
	$(CP_UVA) * $(INST_HTMLDIR)
