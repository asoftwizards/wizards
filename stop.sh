#!/bin/bash
# IDoctor stop script
function stop_module() {
	if [ -f $1.pid ] ; then
 		if ps `cat $1.pid` | grep $1 &> /dev/null ; then
			echo "Stopping $1..."
			PID=`cat $1.pid`;
			rm -f $1.pid;
			kill -9 $PID;
		else
			rm -f $1.pid;
			echo "Module '$1' is already dead."
		fi
	else 
		echo "Module '$1' is not started."
	fi
}

EFFI_ROOT="/usr/local/effi"
EFFI_BIN="$EFFI_ROOT/bin"
PROJECT_ROOT="`dirname $0`"

cd "$PROJECT_ROOT"
#stop_module sws
stop_module GPN
stop_module WWW
stop_module arbd
