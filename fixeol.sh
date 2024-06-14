#!/bin/sh

find . -name "Makefile" -exec dos2unix {} \;
find Config/ -exec dos2unix {} \;
find Database/ -exec dos2unix {} \;
find Script/ -exec dos2unix {} \;
find Programas/ -name "*.sh" -exec dos2unix {} \;
find Programas -name "*.h" -exec dos2unix {} \;
find Programas -name "*.c" -exec dos2unix {} \;
find Programas -name "*.cc" -exec dos2unix {} \;
find Programas -name "*.tab" -exec dos2unix {} \;

