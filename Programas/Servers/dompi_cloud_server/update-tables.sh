#!/bin/sh

cat conf/$2.tab | while read x
do
	if [ "X${x}" != "X" ]
	then
		x=`echo ${x} | sed 's/\r//g' | sed 's/\n//g'`
		if [ "X${2}" = "Xserver" ]
		then
			grep "${x}" $1/$2.tab >/dev/null 2>&1 || echo "${x}${3}" >> $1/$2.tab
		else
			grep "${x}" $1/$2.tab >/dev/null 2>&1 || echo "${x}" >> $1/$2.tab
		fi
	fi
done
