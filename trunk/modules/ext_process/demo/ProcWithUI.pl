#!/usr/bin/perl

#
# Very basic example to communicate with ORTD-simulations
# using ld_Proc_shObj, ld_Process_Recv, ld_Process_Send.
# This is a sub-process that performs a
# communication via stdin and stdout (e,g,
# used by printf & scanf in "C".
# The exchanged data is in form of the binary representation used
# by the precessor of the executing system.
# This binary representation is generated/interpreted by pack & 
# unpack in perl and using ld_ConcateData & ld_DisassembleData
# in ORTD.
#
# http://www.perlmonks.org/?node_id=224666



# Send the initial value to the ortd-simulation
# to trigger one simulation step and caused by this
# a response that is send to this perl-script
$val = 12345.0;
$binval = pack( 'd', $val ); 
syswrite(STDOUT, $binval);


# initialise a counter
$i = 1;

for (;;) { # a never ending loop

  # the ORTD-simulation sends exactly 12 Bytes
  # that are received here.

  sysread(STDIN, $InputBin, 12);
  ($Number_int32, $Value_double) = unpack("l d", $InputBin);

  # calculate something 
#  $val = 0.5 + $i + $Value_double;
  $val = $Value_double;

  # by introducing a delay before sending the results to the ORTD-simulation,
  # this simulation can be synchronised by this sub-process
  sleep(1);

  # Send the calculation result to the ortd-simulation 
  $binval = pack( 'd', $val ); 
  syswrite(STDOUT, $binval);

  # some state update
  $i++;
}