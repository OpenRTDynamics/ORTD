

// Interfacing functions are placed in this place





// An example could be the following scilab function. PARSEDOCU_BLOCK is a hint for the documentation generator to generate documentation for this block
function [sim, out] = ld_scicosblock(sim, events, in, Nin, Nout, identstr) // PARSEDOCU_BLOCK
// 
// ld_scicosblock - include a scicos block / schematic (EXPERIMENTAL)
//
// in * - input
// out * - output
// 
// 

 btype = 15200 + 0;; // the same id you are giving via the "libdyn_compfnlist_add" C-function
 [sim,blk] = libdyn_new_block(sim, events, btype, ipar=[ 0, Nin, Nout, length(identstr), ascii(identstr)  ], rpar=[  ], ...
                  insizes=[Nin], outsizes=[Nout], ...
                  intypes=[ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT]  );

 [sim,blk] = libdyn_conn_equation(sim, blk, list(in) );
 [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

