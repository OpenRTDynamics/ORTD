#!/bin/sh

# --rtmode 1 prevents from running with real real-time priority, chage this to 0 to have
# full real-time priority. This also requires root cepabilities

libdyn_generic_exec --baserate=50 --rtmode 1 -s oscillator -i 901 -l 0 --master_tcpport 10000
