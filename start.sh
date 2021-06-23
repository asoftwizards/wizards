#!/bin/bash

# Load generic functions
source /usr/local/effi/util/services

MYSELF="`readlink -f $0`"
PROJECT_ROOT="`dirname $MYSELF`"
PROJECT_NAME="GPN"

# switch to project's root directory
cd "$PROJECT_ROOT"

ulimit -c unlimited

create_config CONFIG.example CONFIG || exit $?
$BASH ./stop.sh

check_port "ARB_Port" 1
echo "Starting service arbd..."
$BASH $EFFI_UTIL/process-watcher.sh $EFFI_BIN/arbd file=CONFIG & 
check_port "ARB_Port" 0
sleep 1;

check_port "WWW_Port" 1
echo "Starting service sws..."
$BASH $EFFI_UTIL/process-watcher.sh $EFFI_BIN/dlloader WWW file=CONFIG &

echo "Starting service dlloader"
$BASH $EFFI_UTIL/process-watcher.sh $EFFI_BIN/dlloader $PROJECT_NAME file=CONFIG &
#$EFFI_BIN/dlloader $PROJECT_NAME file=CONFIG &
#valgrind --leak-check=full $EFFI_BIN/dlloader $PROJECT_NAME file=CONFIG &

check_port "WWW_Port" 0

