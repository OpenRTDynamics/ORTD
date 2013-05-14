

// Interfacing functions are placed in this place


// ADJUST HERE: Add your interfacing functions here

// An example could be the following scilab function. PARSEDOCU_BLOCK is a hint for the documentation generator to generate documentation for this block
// function [sim, out] = ld_template(sim, events, in) // PARSEDOCU_BLOCK
// // ADD SOME DOCUMENTATION HERE
// // abs - block
// //
// // in * - input
// // out * - output
// // 
// // out = abs(in)
// // 
// 
//  btype = 60001 + 7; // the same id you are giving via the "libdyn_compfnlist_add" C-function
//  [sim,blk] = libdyn_new_block(sim, events, btype, ipar=[ <ADD INTEGERS HERE>  ], rpar=[ <ADD FLOATING POITN VALUES HERE> ], ...
//                   insizes=[1], outsizes=[1], ...
//                   intypes=[ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT]  );
// 
//  [sim,blk] = libdyn_conn_equation(sim, blk, list(in) );
//  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
// endfunction

// if you like to put strings as arguments for the block you could used ipar=[ length(str), ascii(str) ] for example and use "irpar_getstr" within C-Code for decoding (see irpar.h and irpar.c)


// function [sim, out] = ld_EDFWrite(sim, events, in) // PARSEDOCU_BLOCK
// // ADD SOME DOCUMENTATION HERE
// // abs - block
// //
// // in * - input
// // out * - output
// // 
// // out = abs(in)
// // 
// 
// 
//   Uipar = [ 1     ];
//   Urpar = [ 0.1   ];
//   btype = 60001 + 7; // the same id you are giving via the "libdyn_compfnlist_add" C-function
// 
//   insizes=[1];
//   outsizes=[1]; 
//   dfeed=[1];  // for each output
//   intypes=[ORTD.DATATYPE_FLOAT];
//   outtypes=[ORTD.DATATYPE_FLOAT];
// 
// 
//   ///////////////
//   parlist = new_irparam_set();
// 
//    parlist = new_irparam_elemet_ivec(parlist, insizes, 10); 
//    parlist = new_irparam_elemet_ivec(parlist, outsizes, 11); 
//    parlist = new_irparam_elemet_ivec(parlist, intypes, 12); 
//    parlist = new_irparam_elemet_ivec(parlist, outtypes, 13); 
//    parlist = new_irparam_elemet_ivec(parlist, dfeed, 14); 
// 
//    parlist = new_irparam_elemet_ivec(parlist, Uipar, 20); 
//    parlist = new_irparam_elemet_rvec(parlist, Urpar, 21); 
// 
//    blockparam = combine_irparam(parlist);
// 
//   [sim,blk] = libdyn_new_block(sim, events, btype, ipar=[ blockparam.ipar  ], rpar=[ blockparam.rpar ], ...
//                   insizes, outsizes, ...
//                   intypes, outtypes );
// 
//   /////////////
//   
//  [sim,blk] = libdyn_conn_equation(sim, blk, list(in) );
//  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
// endfunction



function [sim, out] = ld_EDFWrite(sim, events, str, in1, in2) // PARSEDOCU_BLOCK
// ADD SOME DOCUMENTATION HERE
// abs - block
//
// in * - input
// out * - output
// 
// out = abs(in)
// 

// introduce some parameters that are refered to by id's
parameter1 = 12345;
vec = [1,2,3];

   parlist = new_irparam_set();

   parlist = new_irparam_elemet_ivec(parlist, parameter1, 10); 
   parlist = new_irparam_elemet_ivec(parlist, vec, 11); // vector of integers (double vectors are similar, replace ivec with rvec)
   parlist = new_irparam_elemet_ivec(parlist, ascii(str), 12);


   p = combine_irparam(parlist);

// Set-up the block parameters and I/O ports

  Uipar = [ p.ipar ];
  Urpar = [ p.rpar ];
  btype = 15800 + 0; // Reference to the block's type (computational function). Use the same id you are giving via the "libdyn_compfnlist_add" C-function

  insizes=[1,1];
  outsizes=[1]; 
  dfeed=[1];  // for each output 0 (no df) or 1 (a direct feedthrough to one of the inputs)
  intypes=[ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT];
  outtypes=[ORTD.DATATYPE_FLOAT];

  blocktype = 1; // 1-BLOCKTYPE_DYNAMIC, 2-BLOCKTYPE_STATIC

  ///////////////
  [sim, blk] = libdyn_CreateBlockAutoConfig(sim, events, btype, blocktype, Uipar, Urpar, insizes, outsizes, intypes, outtypes, dfeed)
  /////////////
  
 [sim,blk] = libdyn_conn_equation(sim, blk, list(in1, in2) );
 [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction