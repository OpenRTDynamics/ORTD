

// Interfacing functions are placed in this place




function [sim, out] = ld_startproc(sim, events, exepath, whentorun) // PARSEDOCU_BLOCK
// 
// Execute a sub process (EXPERIMENTAL)
//
// out * - output
// 
// 

 btype = 15300 + 0; // the same id you are giving via the "libdyn_compfnlist_add" C-function
 [sim,blk] = libdyn_new_block(sim, events, btype, ipar=[ 0,0, 0, 0, whentorun,length(exepath), ascii(exepath)  ], rpar=[  ], ...
                  insizes=[], outsizes=[1], ...
                  intypes=[], outtypes=[ORTD.DATATYPE_FLOAT]  );

//  [sim,blk] = libdyn_conn_equation(sim, blk, list(in) );
 [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

