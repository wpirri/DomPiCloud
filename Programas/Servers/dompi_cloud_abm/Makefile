#!/usr/bin/make -f

include ../../../configure.mk

INCLUDE=-I../../Common/ `mysql_config --cflags` -I/usr/local/include -I/usr/local/include/gmonitor
CXXFLAGS=-g -Wall -Wextra -MMD -O2
NO_LIBS= -lpq
LIBS=-lgmqw -lgmshared -lstdc++ -lcjson -lssl -lcrypto `mysql_config --libs`
OBJECTS=dompi_cloud_abm.o config.o strfunc.o cmydb.o
OUTPUT=dompi_cloud_abm
DEFINE=$(GENERAL_DEFINE)
CC=g++

#.cc.o:
#	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c $<

all: dompi_cloud_abm

dompi_cloud_abm.o: dompi_cloud_abm.cc
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c dompi_cloud_abm.cc

config.o: ../../Common/config.cc ../../Common/config.h
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c ../../Common/config.cc
strfunc.o: ../../Common/strfunc.cc ../../Common/strfunc.h
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c ../../Common/strfunc.cc
# Para Postgres
#cpgdb.o: ../../Common/cpgdb.cc ../../Common/cpgdb.h
#	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c ../../Common/cpgdb.cc
cmydb.o: ../../Common/cmydb.cc ../../Common/cmydb.h
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c ../../Common/cmydb.cc

dompi_cloud_abm: $(OBJECTS)
	$(CC) $(CFLAGS) $(CXXFLAGS) $(LFLAGS) -O $(OBJECTS) $(LIBS) -o $(OUTPUT) 

clean:
	$(RM) $(OBJECTS) $(OUTPUT) *.d

install:
	$(CHMOD) +x install_server.sh
	$(CHMOD) +x update-tables.sh
	./install_server.sh $(INST_SBINDIR) $(OUTPUT) $(INST_VARDIR)

uninstall:
	$(RM) $(INST_SBINDIR)/$(OUTPUT)
