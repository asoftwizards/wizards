#!/bin/bash

EFFI_ROOT=/usr/local/effi
REPO_ROOT=/usr/local/reporter

DIR_NAME=`dirname "$0"`

VER=`cat "$DIR_NAME"/version`
EFFI_VER=`cat $EFFI_ROOT/version `
DEPFILE="$DIR_NAME/effi-version."`echo -n $VER | sed -re 's/-[0-9d]+$//'`
REPO_VER=`cat $REPO_ROOT/version `
REPO_DEPFILE="$DIR_NAME/reporter-version."`echo -n $VER | sed -re 's/-[0-9d]+$//'`
PATCHER="/usr/local/effi/util/patch.pl"
if ! [ -r CONFIG ] ; then
        cp CONFIG.example CONFIG
fi

RCONF="$EFFI_ROOT/bin/readconf CONFIG"

#export DATABASE="MYSQL"
export DATABASE="MYSQL"
export DBPATCHER_PATCHDIR="$DIR_NAME/dbpatches"

export DB_BASE="`$RCONF DBConnection/Database`"
export DB_USER="`$RCONF DBConnection/UserName`"
export DB_HOST="`$RCONF DBConnection/Host`"

mysql -u$DB_USER << EOF
	create database IF NOT EXISTS $DB_BASE ;
EOF

$PATCHER --register-module "" $VER
$PATCHER --register-module EFFI $EFFI_VER
$PATCHER --register-module EFFI/Authorizer $EFFI_VER
$PATCHER --register-module EFFI/TaskMan $EFFI_VER
$PATCHER --register-module EFFI/multimedia $EFFI_VER
$PATCHER --register-module EFFI/PSS $EFFI_VER
$PATCHER --register-module Reporter $REPO_VER

$PATCHER --dependency EFFI,$DEPFILE,$EFFI_ROOT/dbpatches --dependency Reporter,$REPO_DEPFILE,$REPO_ROOT/dbpatches $@

