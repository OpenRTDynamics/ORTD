#!/bin/sh

#libdyn_generic_exec  -s controller -i 901 -l 10

libdyn_generic_exec --baserate=100 -s read_xs2_ArmMotion -i 901 -l 0 --master_tcpport 10000
