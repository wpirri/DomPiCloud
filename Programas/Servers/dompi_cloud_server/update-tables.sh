#!/bin/sh
set -x
cat conf/$2.tab | while read line
do
	if [ "X${line}" != "X" ]
	then
		line = `echo $line | sed "s/\r//g" | sed "s/\n//g"`
		if [ "X${2}" = "Xserver" ]
		then
			grep "$line" $1/$2.tab >/dev/null 2>&1 || echo "$line$3" >> $1/$2.tab
		else
			grep "$line" $1/$2.tab >/dev/null 2>&1 || echo "$line" >> $1/$2.tab
		fi
	fi
done
