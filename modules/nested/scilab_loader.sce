

// Interfacing functions are placed in this place


//function [sim,bid] = libdyn_new_scope(sim, events, str, insize)
//  btype = 10001;
//  str = ascii(str);
//  [sim,bid] = libdyn_new_blk_generic(sim, events, btype, [insize, length(str), str(:)'], []);
//endfunction
//
//function [sim] = ld_scope(sim, events, in, str, insize)
//  [sim,blk] = libdyn_new_scope(sim, events, str, insize);
//  [sim,blk] = libdyn_conn_equation(sim, blk, list(in,0) );
//endfunction


function [sim, outlist] = ld_simnest(sim, ev, swich_signal, inlist, insizes, outsizes, intypes, outtypes, dfeed, schematics_list)
// 
// ld_simnest -- create one (or multiple) nested libdyn simulation within a normal libdyn block
// 		 it is possible to switch between them by an special input signal
// 
// INPUTS:
// 
// ev - events to be forwarded to the nested simulation
// inlist - list( ) of input signals
// insizes - input ports configuration
// outsizes - output ports configuration
// intypes - 
// outtypes -
// dfeed - the block containing all sub-schematics is configured with dfeed
// schematics_list - list( ) of scilab functions defining sub-schematics
// 
// OUTPUTS:
// 
// outlist - list( ) of output signals
// 

endfunction