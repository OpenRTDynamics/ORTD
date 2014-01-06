

// Interfacing functions are placed in this place







function [sim] = ld_Proc_shObj(sim, events, ObjectIdentifyer, Visibility, executable, OptionList) // PARSEDOCU_BLOCK
// 
// Set-up a Process with I/O redirection
//
// Note: only one and at least one parameter is required in OptionList
// 
// EXPERIMENTAL
// 

  // add a postfix that identifies the type of the shared object
  ObjectIdentifyer = ObjectIdentifyer + ".Process_ShObj";



   // pack all parameters into a structure "parlist"
   parlist = new_irparam_set();

   parlist = new_irparam_elemet_ivec(parlist, ascii(executable), 1); // id = 11; A string parameter

   NumberOfCmdOptions = length(OptionList);
   parlist = new_irparam_elemet_ivec(parlist, NumberOfCmdOptions, 10); // id = 10
   parlist = new_irparam_elemet_ivec(parlist, ascii(OptionList(1)), 11); // id = 11; A string parameter

   
   p = combine_irparam(parlist); // convert to two vectors of integers and floating point values respectively

// Set-up the block parameters. There are no I/O ports
  Uipar = [ p.ipar ];
  Urpar = [ p.rpar ];
  btype = 15300 + 1; // Reference to the block's type (computational function). Use the same id you are giving via the "libdyn_compfnlist_add" C-function


  [sim] =  libdyn_CreateSharedObjBlk(sim, btype, ObjectIdentifyer, Visibility, Uipar, Urpar); 
endfunction


function [sim, out, NumBytes] = ld_Process_Recv(sim, events, ObjectIdentifyer, ExpectedBytes, MaxOutsize) 
// 
// Process - receiver block
//
// out *, ORTD.DATATYPE_BINARY - output
// ExpectedBytes * ORTD.DATATYPE_INT32 - the number of bytes to read until an simulation step is triggered
// 
// This is a simulation-synchronising Block. Everytime ExpectedBytes are read from the process's stdout,
// the simulation that contains this blocks goes on for one step.
// 
// EXPERIMENTAL
// 

  printf("Synchronising simulation to Process-Receiver\n");

  // add a postfix that identifies the type of the shared object
  ObjectIdentifyer = ObjectIdentifyer + ".Process_ShObj";

  //
  outtype = ORTD.DATATYPE_BINARY;



   // pack all parameters into a structure "parlist"
   parlist = new_irparam_set();

   parlist = new_irparam_elemet_ivec(parlist, MaxOutsize, 10); // id = 10
   parlist = new_irparam_elemet_ivec(parlist, outtype, 11); // id = 11

   p = combine_irparam(parlist); // convert to two vectors of integers and floating point values respectively

// Set-up the block parameters and I/O ports
  Uipar = [ p.ipar ];
  Urpar = [ p.rpar ];
  btype = 15300 + 2; // Reference to the block's type (computational function). Use the same id you are giving via the "libdyn_compfnlist_add" C-function

  insizes=[1]; // Input port sizes
  outsizes=[MaxOutsize, 1]; // Output port sizes
  dfeed=[1];  // for each output 0 (no df) or 1 (a direct feedthrough to one of the inputs)
  intypes=[ORTD.DATATYPE_INT32]; // datatype for each input port
  outtypes=[outtype, ORTD.DATATYPE_INT32]; // datatype for each output port

  blocktype = 1; // 1-BLOCKTYPE_DYNAMIC (if block uses states), 2-BLOCKTYPE_STATIC (if there is only a static relationship between in- and output)

  // Create the block
  [sim, blk] = libdyn_CreateBlockAutoConfig(sim, events, btype, blocktype, Uipar, Urpar, insizes, outsizes, intypes, outtypes, dfeed, ObjectIdentifyer);
  
//   // connect the inputs
  [sim,blk] = libdyn_conn_equation(sim, blk, list(ExpectedBytes) ); // connect in1 to port 0 and in2 to port 1

   // connect the ouputs
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
  [sim,NumBytes] = libdyn_new_oport_hint(sim, blk, 1);   // 1th port
endfunction

function [sim] = ld_Process_Send(sim, events, SendSize, ObjectIdentifyer, in, insize) 
// 
// To process Send block
//
// in *, ORTD.DATATYPE_BINARY - input
// SendSize *. ORTD.DATATYPE_INT32 - Number of bytes to send
// 
// EXPERIMENTAL
// 

  intype = ORTD.DATATYPE_BINARY;

  // add a postfix that identifies the type of the shared object
  ObjectIdentifyer = ObjectIdentifyer + ".Process_ShObj";


   // pack all parameters into a structure "parlist"
   parlist = new_irparam_set();

   parlist = new_irparam_elemet_ivec(parlist, insize, 10); // id = 10
//    parlist = new_irparam_elemet_ivec(parlist, intype, 11); // id = 11

   p = combine_irparam(parlist); // convert to two vectors of integers and floating point values respectively

// Set-up the block parameters and I/O ports
  Uipar = [ p.ipar ];
  Urpar = [ p.rpar ];
  btype = 15300 + 3; // Reference to the block's type (computational function). Use the same id you are giving via the "libdyn_compfnlist_add" C-function

  insizes=[insize, 1]; // Input port sizes
  outsizes=[]; // Output port sizes
  dfeed=[1];  // for each output 0 (no df) or 1 (a direct feedthrough to one of the inputs)
  intypes=[intype,ORTD.DATATYPE_INT32]; // datatype for each input port
  outtypes=[]; // datatype for each output port

  blocktype = 1; // 1-BLOCKTYPE_DYNAMIC (if block uses states), 2-BLOCKTYPE_STATIC (if there is only a static relationship between in- and output)

  // Create the block
  [sim, blk] = libdyn_CreateBlockAutoConfig(sim, events, btype, blocktype, Uipar, Urpar, insizes, outsizes, intypes, outtypes, dfeed, ObjectIdentifyer);
  
  // connect the inputs
 [sim,blk] = libdyn_conn_equation(sim, blk, list(in, SendSize) ); // connect in1 to port 0 and in2 to port 1

//   // connect the ouputs
//  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction



















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
