

// Interfacing functions are placed in this place


// ADJUST HERE: Add your interfacing functions here



function [sim, out] = ld_synctimer(sim, events, in) // PARSEDOCU_BLOCK
// %PURPOSE: Timer for synchronisation of a async simulation 
//
// To beused within an async nested schematic for introducing variable sample times (EXPERIMENTAL)
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


function [sim, out] = ld_clock(sim, events) // PARSEDOCU_BLOCK
//
// %PURPOSE: get current system time in [s]
//
// out * - time
// 
// 


  btype = 15100 + 1;
  [sim,blk] = libdyn_new_block(sim, events, btype, ipar=[ 0 ], rpar=[  ], ...
                   insizes=[], outsizes=[1], ...
                   intypes=[], outtypes=[ORTD.DATATYPE_FLOAT]  );


  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction
