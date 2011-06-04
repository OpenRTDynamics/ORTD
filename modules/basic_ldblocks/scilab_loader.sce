

// Interfacing functions are placed in this place



//
// A 2 to 1 switching Block
// inputs = [control_in, signal_in1, signal_in2]
// if control_in > 0 : out = signal_in1
// if control_in < 0 : out = signal_in2
//
function [sim,out] = ld_switch2to1(sim, events, cntrl, in1, in2)
  btype = 60001;
  [sim,blk] = libdyn_new_blk_generic(sim, events, btype, [], []);

  [sim,blk] = libdyn_conn_equation(sim, blk, list(cntrl, in1, in2) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

