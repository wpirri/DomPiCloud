#!/usr/bin/make -f

INCLUDE=-I../../Common/
CXXFLAGS=-g -Wall -Wextra -MMD -O2
LIBS=-lgmqw -lgmshared -lstdc++ -lpq -lcjson
OBJECTS=dompi_cloud_server.o config.o strfunc.o cpgdb.o
OUTPUT=dompi_cloud_server
DEFINE=-D __NO__DEBUG__
CC=g++

#.cc.o:
#	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c $<

all: dompi_cloud_server

dompi_cloud_server.o: dompi_cloud_server.cc
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c dompi_cloud_server.cc

config.o: ../../Common/config.cc ../../Common/config.h
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c ../../Common/config.cc
strfunc.o: ../../Common/strfunc.cc ../../Common/strfunc.h
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c ../../Common/strfunc.cc
cpgdb.o: ../../Common/cpgdb.cc ../../Common/cpgdb.h
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c ../../Common/cpgdb.cc

dompi_cloud_server: $(OBJECTS)
	$(CC) $(CFLAGS) $(CXXFLAGS) $(LFLAGS) $(LIBS) -O $(OBJECTS) -o $(OUTPUT) 

clean:
	$(RM) $(OBJECTS) $(OUTPUT) *.d

install:
	./install_server.sh $(OUTPUT)

uninstall:
	$(RM) /usr/local/sbin/$(OUTPUT)