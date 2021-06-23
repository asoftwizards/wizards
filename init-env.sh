#!/bin/bash

mkdir -p ./doc/types
mkdir -p ./dbpatches/{init,always}

ln -sf -T /usr/local/effi/wnd/BASE ./wnd/BASE
ln -sf -T /usr/local/effi/wnd/Authorizer ./wnd/Authorizer
ln -sf -T /usr/local/reporter/wnd/Reporter ./wnd/Reporter
ln -sf -T /usr/local/effi/wnd/Multimedia ./wnd/Multimedia
ln -sf -T /usr/local/reporter/doc/icons-Reporter ./doc/icons-Reporter

if [ -h "doc/skins" ] ; then rm "doc/skins"; fi
mkdir -p "doc/skins"
ln -sf /usr/local/effi/doc/skins/* -t doc/skins
ln -sf ./blue -T doc/skins/default

mkdir -p ./doc/types
ln -sf /usr/local/effi/doc/types/* ./doc/types
ln -sf /usr/local/effi/conf/sws.conf ./
ln -sf /usr/local/effi/lang/locale.ru_RU ./
ln -sf /usr/local/reporter/locale.ru_RU.Reporter ./
mkdir -p ./dbpatches/{init,always}
cp dbinit.mysql ./dbpatches/init/00_schema.sql
cp ElementsInit.GPN.mysql ./dbpatches/init/10_methods.sql
cp ElementsInit.GPN.mysql ./dbpatches/always/ElementsInit.sql
cp init.sql ./dbpatches/init/30_init.sql
  

