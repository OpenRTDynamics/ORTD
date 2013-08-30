

// Interfacing functions are placed in this place


// ADJUST HERE: Add your interfacing functions here


ORTD.ORTD_RT_REALTIMETASK = 1;
ORTD.ORTD_RT_NORMALTASK = 2;



function [sim, out] = ld_synctimer(sim, events, in) // PARSEDOCU_BLOCK
// %PURPOSE: Timer for synchronisation of a async simulation 
//
// To beused within an async nested schematic for introducing variable sample times (EXPERIMENTAL)
// 
// For an example see modules/synchronisation/demo/timed_thread.sce
// 
// This is obsolete and will be removed. Use ld_ClockSync instead.
// 


 btype = 15100 + 0; //
 [sim,blk] = libdyn_new_block(sim, events, btype, ipar=[ 0  ], rpar=[ ], ...
                  insizes=[1], outsizes=[1], ...
                  intypes=[ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT]  );

 [sim,blk] = libdyn_conn_equation(sim, blk, list(in) );
 [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction


function [sim, out] = ld_ClockSync(sim, events, in) // PARSEDOCU_BLOCK
// %PURPOSE: Timer for synchronisation of a async simulation 
//
// To beused within an async nested schematic for introducing variable sample times (EXPERIMENTAL)
// 
// For an example see modules/synchronisation/demo/timed_thread.sce
// 
// New version of ld_synctimer (was experimental), which will be removed.
// 

// introduce some parameters that are refered to by id's

   // pack all parameters into a structure "parlist"
   parlist = new_irparam_set();

//    parlist = new_irparam_elemet_ivec(parlist, parameter1, 10); // id = 10
//    parlist = new_irparam_elemet_ivec(parlist, vec, 11); // vector of integers (double vectors are similar, replace ivec with rvec)
//    parlist = new_irparam_elemet_ivec(parlist, ascii(str), 12); // id = 12; A string parameter

   p = combine_irparam(parlist); // convert to two vectors of integers and floating point values respectively

// Set-up the block parameters and I/O ports
  Uipar = [ p.ipar ];  Urpar = [ p.rpar ];

  // Reference to the block's type (computational function). Use the same id you are giving via the "libdyn_compfnlist_add" C-function
  btype = 15100 + 2; //

  insizes=[1]; // Input port sizes
  outsizes=[1]; // Output port sizes
  dfeed=[1];  // for each output 0 (no df) or 1 (a direct feedthrough to one of the inputs)
  intypes=[ORTD.DATATYPE_FLOAT]; // datatype for each input port
  outtypes=[ORTD.DATATYPE_FLOAT]; // datatype for each output port

  blocktype = 1; // 1-BLOCKTYPE_DYNAMIC (if block uses states), 2-BLOCKTYPE_STATIC (if there is only a static relationship between in- and output)

  // Create the block
  [sim, blk] = libdyn_CreateBlockAutoConfig(sim, events, btype, blocktype, Uipar, Urpar, insizes, outsizes, intypes, outtypes, dfeed);
  
  // connect the inputs
 [sim,blk] = libdyn_conn_equation(sim, blk, list(in) ); // connect in1 to port 0 and in2 to port 1

  // connect the ouputs
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
