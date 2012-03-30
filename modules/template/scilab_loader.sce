

// Interfacing functions are placed in this place


// ADJUST HERE: Add your interfacing functions here

// An example could be the following scilab function. PARSEDOCU_BLOCK is a hint for the documentation generator to generate documentation for this block
//function [sim, out] = ld_abs(sim, events, in) // PARSEDOCU_BLOCK
//// ADD SOME DOCUMENTATION HERE
//// abs - block
////
//// in * - input
//// out * - output
//// 
//// out = abs(in)
//// 
//
//  btype = 60001 + 7; // the same id you are giving via the "libdyn_compfnlist_add" C-function
//  [sim,blk] = libdyn_new_block(sim, events, btype, ipar=[ <ADD INTEGERS HERE>  ], rpar=[ <ADD FLOATING POITN VALUES HERE> ], ...
//                   insizes=[1], outsizes=[1], ...
//                   intypes=[ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT]  );
//
//  [sim,blk] = libdyn_conn_equation(sim, blk, list(in) );
//  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
//endfunction

// if you like to put strings as arguments for the block you could used ipar=[ length(str), ascii(str) ] for example and use "irpar_getstr" within C-Code for decoding (see irpar.h and irpar.c)
