#!/bin/sh
make clean
bear make
sed -i 's/"cc"/"arm-linux-gnueabihf-gcc"/g' compile_commands.json
cp -f -r *.ko /home/gezhao/linux/nfs/nfsroot/lib/modules/4.1.15/
