

// Interfacing functions are placed in this place




function [sim, out] = ld_startproc(sim, events, exepath, whentorun) // PARSEDOCU_BLOCK
// 
// %PURPOSE: Execute a sub process (EXPERIMENTAL)
//
// out * - output (unused)
// exepath - string: path to executable
// whentorun - 0 - Start process on blocks initialisation and stop on desruction; 
//             1 - Start process on activation in a statemachine and stop on reset
// 
// 

  printf("ld_startproc: starting %s\n", exepath);

 btype = 15300 + 0; // the same id you are giving via the "libdyn_compfnlist_add" C-function
 [sim,blk] = libdyn_new_block(sim, events, btype, ipar=[ 0, 0, 0, 0, whentorun,length(exepath),0, ascii(exepath)  ], rpar=[  ], ...
                  insizes=[], outsizes=[1], ...
                  intypes=[], outtypes=[ORTD.DATATYPE_FLOAT]  );

//  [sim,blk] = libdyn_conn_equation(sim, blk, list(in) );
 [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port


 [sim, out] = ld_gain(sim, events, out, 1); // FIXME: Otherwise the block above may not be initialised
endfunction

function [sim, out] = ld_startproc2(sim, events, exepath, chpwd, prio, whentorun) // PARSEDOCU_BLOCK
// 
// %PURPOSE: Execute a sub process with some more options (EXPERIMENTAL)
//
// out * - output
// exepath - string: path to executable
// chpwn - change current directory before running the process
// prio - Priority (set to zero for now)
// whentorun - 0 - Start process on blocks initialisation and stop on desruction; 
//             1 - Start process on activation in a statemachine and stop on reset
// 
// 

printf("ld_startproc: starting %s\n", exepath);

 btype = 15300 + 0; // the same id you are giving via the "libdyn_compfnlist_add" C-function
 [sim,blk] = libdyn_new_block(sim, events, btype, ipar=[ 0, 0, 0, prio, whentorun,length(exepath), length(chpwd), ascii(exepath), ascii(chpwd)  ], rpar=[  ], ...
                  insizes=[], outsizes=[1], ...
                  intypes=[], outtypes=[ORTD.DATATYPE_FLOAT]  );

//  [sim,blk] = libdyn_conn_equation(sim, blk, list(in) );
 [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port


 [sim, out] = ld_gain(sim, events, out, 1); // FIXME: Otherwise the block above may not be initialised
endfunction
