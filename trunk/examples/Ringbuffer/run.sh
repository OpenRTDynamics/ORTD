#!/bin/sh

#libdyn_generic_exec  -s controller -i 901 -l 10

libdyn_generic_exec --baserate=500 -s Ringbuffer -i 901 -l 0 --master_tcpport 10000
