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



function [sim, out] = ld_TemplateWrite(sim, events, str, in1, in2) // PARSEDOCU_BLOCK
// ADD SOME DOCUMENTATION HERE, that will be copied to the scilab help
// abs - block
//
// in * - input
// out * - output
// 
// out = abs(in)
// 

   // check the input parameters
   ortd_checkpar(sim, list('String', 'str', str) );
   ortd_checkpar(sim, list('Signal', 'in1', in1) );
   ortd_checkpar(sim, list('Signal', 'in2', in2) );
//    ortd_checkpar(sim, list('SingleValue', 'gain', gain) );


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
  btype = ORTD_BLOCKIDSTART + 0; // Reference to the block's type (computational function). Use the same id you are giving via the "libdyn_compfnlist_add" C-function

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
// How to use Shared Objects
// 
// 

function [sim] = ld_Template_shObj(sim, events, ObjectIdentifyer, Visibility) // PARSEDOCU_BLOCK
// ADD SOME DOCUMENTATION HERE, that will be copied to the scilab help
// 
// This function creates a shared object
// 

  // add a postfix that identifies the type of the shared object
  ObjectIdentifyer = ObjectIdentifyer + ".Template_ShObj";


// introduce some parameters that are refered to by id's
   parameter1 = 12345;
   vec = [1,2,3];

   // pack all parameters into a structure "parlist"
   parlist = new_irparam_set();

   parlist = new_irparam_elemet_ivec(parlist, parameter1, 10); // id = 10
//    parlist = new_irparam_elemet_ivec(parlist, vec, 11); // vector of integers (double vectors are similar, replace ivec with rvec)
   parlist = new_irparam_elemet_ivec(parlist, ascii("This is a string parameter"), 12); // id = 12; A string parameter

   p = combine_irparam(parlist); // convert to two vectors of integers and floating point values respectively

// Set-up the block parameters. There are no I/O ports
  Uipar = [ p.ipar ];
  Urpar = [ p.rpar ];
  btype = ORTD_BLOCKIDSTART + 10; // Reference to the block's type (computational function). Use the same id you are giving via the "libdyn_compfnlist_add" C-function

  [sim] =  libdyn_CreateSharedObjBlk(sim, btype, ObjectIdentifyer, Visibility, Uipar, Urpar); 
endfunction


function [sim, out] = ld_Template_AccessShObj(sim, events, ObjectIdentifyer, in1, in2) // PARSEDOCU_BLOCK
// ADD SOME DOCUMENTATION HERE, that will be copied to the scilab help
// 
// This function can use a shared object created by ld_Template_shObj
// 
// 
// 

  // add a postfix that identifies the type of the shared object
  ObjectIdentifyer = ObjectIdentifyer + ".Template_ShObj";

// introduce some parameters that are refered to by id's
parameter1 = 12345;
vec = [1,2,3];

   // pack all parameters into a structure "parlist"
   parlist = new_irparam_set();

   parlist = new_irparam_elemet_ivec(parlist, parameter1, 10); // id = 10

   p = combine_irparam(parlist); // convert to two vectors of integers and floating point values respectively

// Set-up the block parameters and I/O ports
  Uipar = [ p.ipar ];
  Urpar = [ p.rpar ];
  btype = ORTD_BLOCKIDSTART + 11; // Reference to the block's type (computational function). Use the same id you are giving via the "libdyn_compfnlist_add" C-function

  insizes=[1,1]; // Input port sizes
  outsizes=[1]; // Output port sizes
  dfeed=[1];  // for each output 0 (no df) or 1 (a direct feedthrough to one of the inputs)
  intypes=[ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT]; // datatype for each input port
  outtypes=[ORTD.DATATYPE_FLOAT]; // datatype for each output port

  blocktype = 1; // 1-BLOCKTYPE_DYNAMIC (if block uses states), 2-BLOCKTYPE_STATIC (if there is only a static relationship between in- and output)

  // Create the block
  [sim, blk] = libdyn_CreateBlockAutoConfig(sim, events, btype, blocktype, Uipar, Urpar, insizes, outsizes, intypes, outtypes, dfeed, ObjectIdentifyer);
  
  // connect the inputs
 [sim,blk] = libdyn_conn_equation(sim, blk, list(in1, in2) ); // connect in1 to port 0 and in2 to port 1

  // connect the ouputs
 [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction
