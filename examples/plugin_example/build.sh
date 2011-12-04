#!/bin/sh

gcc -I../.. -fPIC plugin.c -c -o plugin.o
ld -shared plugin.o -o ortd_plugin.so
