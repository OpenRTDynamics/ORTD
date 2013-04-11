function [sim, out] = ld_AndroidSensors(sim, events, in, rate) // PARSEDOCU_BLOCK
// %PURPOSE: Read out Android Sensors and synchronise the simulation to them
//
// Special: SYNC_BLOCK (use only one block of this type in an asynchronous running sub-simulation)
//
// out - vector of size 10 containing the sensor values
// in - when in becomes one, the synchronisation loop is interrupted
// 


 btype = 15500 + 0; //
 [sim,blk] = libdyn_new_block(sim, events, btype, ipar=[ 0  ], rpar=[ rate ], ...
                  insizes=[1], outsizes=[10], ...
                  intypes=[ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT]  );

 [sim,blk] = libdyn_conn_equation(sim, blk, list(in) );
 [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction
