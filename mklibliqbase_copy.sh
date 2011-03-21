#!/bin/sh
# make and copy 
# by lcuk on #maemo  liquid@gmail.com

cd ~/svn_tab/libliqbase/
if make; then 
  scp -r ./src/libliqbase.so.1 root@10.0.0.8:/usr/lib
  scp -r ./media/* root@10.0.0.8:/usr/share/liqbase/libliqbase/media
fi
