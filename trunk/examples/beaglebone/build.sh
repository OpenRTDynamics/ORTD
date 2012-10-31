#!/bin/sh

gcc -I../.. -fPIC plugin.c -c -o plugin.o
gcc -fPIC gpio.c -c  -o gpio.o
ld -shared plugin.o gpio.o -o ortd_plugin.so
