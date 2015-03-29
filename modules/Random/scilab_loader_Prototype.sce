// 
// 
// Interfacing functions are placed in this place
// 
// 
// This is a template from which scilab_loader.sce is automatically produced
// when running the module's makefile.
//
// The placeholder ORTD_BLOCKIDSTART will be repalced when running the Makefile by the 
// contents of the variable blockid_start in the beginning of the Makefile
// 


// 
// ortd_checkpar types:
// 
//     'Signal' 
//     'SignalList' 
//     'SingleValue' 
//     'Vector'
//     'String'
// 
//  e.g.
// 
//   ortd_checkpar(sim, list('Signal', 'in', in) );
//   ortd_checkpar(sim, list('SingleValue', 'gain', gain) );
// 



function [sim, out] = ld_Random(sim, events, Method, Seed) // PARSEDOCU_BLOCK
// 
// Random generator - block
//
// out * - random output
// 
// Method - 0 (normal distribution), 1 (uniform distribution, NOT IMPLEMENTED)
// Seed - The random seed to start with, NOT IMPLEMENTED
//
// The implementation of http://randomlib.sourceforge.net is used.
// 

   // check the input parameters

    ortd_checkpar(sim, list('SingleValue', 'Method', Method) );
    ortd_checkpar(sim, list('SingleValue', 'Seed', Seed) );

// introduce some parameters that are refered to by id's
//    Method = 0; // normal distribution
//    Seed = 0; 

   vec = [0,Method,Seed];

   // pack all parameters into a structure "parlist"
   parlist = new_irparam_set();

   parlist = new_irparam_elemet_ivec(parlist, vec, 11); // vector of integers (double vectors are similar, replace ivec with rvec)


   p = combine_irparam(parlist); // convert to two vectors of integers and floating point values respectively

// Set-up the block parameters and I/O ports
  Uipar = [ p.ipar ];
  Urpar = [ p.rpar ];
  btype = ORTD_BLOCKIDSTART + 0; // Reference to the block's type (computational function). Use the same id you are giving via the "libdyn_compfnlist_add" C-function

  insizes=[]; // Input port sizes
  outsizes=[1]; // Output port sizes
  dfeed=[1];  // for each output 0 (no df) or 1 (a direct feedthrough to one of the inputs)
  intypes=[]; // datatype for each input port
  outtypes=[ORTD.DATATYPE_FLOAT]; // datatype for each output port

  blocktype = 1; // 1-BLOCKTYPE_DYNAMIC (if block uses states), 2-BLOCKTYPE_STATIC (if there is only a static relationship between in- and output)

  // Create the block
  [sim, blk] = libdyn_CreateBlockAutoConfig(sim, events, btype, blocktype, Uipar, Urpar, insizes, outsizes, intypes, outtypes, dfeed);
  
  // connect the ouputs
 [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction



