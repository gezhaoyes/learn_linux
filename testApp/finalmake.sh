#!/bin/sh
arm-linux-gnueabihf-gcc -o test_1_chrdevbase.out  test_1_chrdevbase.c
arm-linux-gnueabihf-gcc -o test_input_key.out  test_input_key.c
cp -f -r *.out /home/gezhao/linux/nfs/nfsroot/lib/modules/4.1.15/