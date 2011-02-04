#!/bin/sh
# build a package
# by lcuk on #maemo  liquid@gmail.com

#find . -print0 | xargs -r0 touch
chmod +x debian/rules
dpkg-buildpackage -rfakeroot -i -I.git
# for updating from scratchbox to my personal device
scp ../libliqbase1_0.3.65_armel.deb root@10.0.0.8:/root
ssh root@10.0.0.8 "dpkg -i libliqbase1_0.3.65_armel.deb"



