

// Interfacing functions are placed in this place


// ADJUST HERE: Add your interfacing functions here



function [sim, out] = ld_synctimer(sim, events, in) // PARSEDOCU_BLOCK
// 
// To beused within an async nested schematic for introducing variable sample times
// 
// For an example see modules/synchronisation/demo/timed_thread.sce
// 
// 


 btype = 15100 + 0; //
 [sim,blk] = libdyn_new_block(sim, events, btype, ipar=[ 0  ], rpar=[ ], ...
                  insizes=[1], outsizes=[1], ...
                  intypes=[ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT]  );

 [sim,blk] = libdyn_conn_equation(sim, blk, list(in) );
 [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

