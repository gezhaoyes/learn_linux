#!/bin/sh
make clean
bear make
sed -i 's/"cc"/"arm-linux-gnueabihf-gcc"/g' compile_commands.json