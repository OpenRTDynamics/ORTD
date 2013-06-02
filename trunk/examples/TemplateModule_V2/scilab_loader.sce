

// Interfacing functions are placed in this place




function [sim, out] = ld_TemplateWrite(sim, events, str, in1, in2) // PARSEDOCU_BLOCK
// ADD SOME DOCUMENTATION HERE, that will be copied to the scilab help
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

   // pack all parameters into a structure "parlist"
   parlist = new_irparam_set();

   parlist = new_irparam_elemet_ivec(parlist, parameter1, 10); // id = 10
   parlist = new_irparam_elemet_ivec(parlist, vec, 11); // vector of integers (double vectors are similar, replace ivec with rvec)
   parlist = new_irparam_elemet_ivec(parlist, ascii(str), 12); // id = 12; A string parameter

   p = combine_irparam(parlist); // convert to two vectors of integers and floating point values respectively

// Set-up the block parameters and I/O ports
  Uipar = [ p.ipar ];
  Urpar = [ p.rpar ];
  btype = 999911111 + 0; // Reference to the block's type (computational function). Use the same id you are giving via the "libdyn_compfnlist_add" C-function

  insizes=[1,1]; // Input port sizes
  outsizes=[1]; // Output port sizes
  dfeed=[1];  // for each output 0 (no df) or 1 (a direct feedthrough to one of the inputs)
  intypes=[ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT]; // datatype for each input port
  outtypes=[ORTD.DATATYPE_FLOAT]; // datatype for each output port

  blocktype = 1; // 1-BLOCKTYPE_DYNAMIC (if block uses states), 2-BLOCKTYPE_STATIC (if there is only a static relationship between in- and output)

  // Create the block
  [sim, blk] = libdyn_CreateBlockAutoConfig(sim, events, btype, blocktype, Uipar, Urpar, insizes, outsizes, intypes, outtypes, dfeed);
  
  // connect the inputs
 [sim,blk] = libdyn_conn_equation(sim, blk, list(in1, in2) ); // connect in1 to port 0 and in2 to port 1

  // connect the ouputs
 [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction


// 
// function [sim, out] = ld:Template_shObj(sim, events, ObjectIdentifyer) // PARSEDOCU_BLOCK
// // ADD SOME DOCUMENTATION HERE, that will be copied to the scilab help
// // abs - block
// //
// // 
// 
// // introduce some parameters that are refered to by id's
// parameter1 = 12345;
// vec = [1,2,3];
// 
//    // pack all parameters into a structure "parlist"
//    parlist = new_irparam_set();
// 
//    parlist = new_irparam_elemet_ivec(parlist, parameter1, 10); // id = 10
// //    parlist = new_irparam_elemet_ivec(parlist, vec, 11); // vector of integers (double vectors are similar, replace ivec with rvec)
// //    parlist = new_irparam_elemet_ivec(parlist, ascii(str), 12); // id = 12; A string parameter
// 
//    p = combine_irparam(parlist); // convert to two vectors of integers and floating point values respectively
// 
// // Set-up the block parameters and I/O ports
//   Uipar = [ p.ipar ];
//   Urpar = [ p.rpar ];
//   btype = 999911111 + 10; // Reference to the block's type (computational function). Use the same id you are giving via the "libdyn_compfnlist_add" C-function
// 
//   ObjectIdentifyer = "TestSharedObj";
//   Visibility = 0;
// 
//   // new fn (sim, btype, ObjectIdentifyer, Visibility, Uipar, Urpar)
//   
//   events = 0;
//   insizes=[]; // Input port sizes
//   outsizes=[]; // Output port sizes
//   dfeed=[1];  // for each output 0 (no df) or 1 (a direct feedthrough to one of the inputs)
//   intypes=[]; // datatype for each input port
//   outtypes=[]; // datatype for each output port
// 
//   blocktype = 2; // 1-BLOCKTYPE_DYNAMIC (if block uses states), 2-BLOCKTYPE_STATIC (if there is only a static relationship between in- and output)
// 
//   // Create the block
//   [sim, blk] = libdyn_CreateBlockAutoConfig(sim, events, btype, blocktype, Uipar, Urpar, insizes, outsizes, intypes, outtypes, dfeed, ObjectIdentifyer, Visibility);
// 
//   // ensure the block is included in the simulation even without any I/O ports
//  sim = libdyn_include_block(sim, blk);
//   
//   // end new fn (sim)
//   
// endfunction
// 



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

