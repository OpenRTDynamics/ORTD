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






// 
// How to use Shared Objects
// 
// 

function [sim] = ld_I2CDevice_shObj(sim, events, ObjectIdentifyer, Visibility, I2CDevicename, I2Caddr) // PARSEDOCU_BLOCK
// 
// %PURPOSE: I2C device connection
// 
// This function creates a shared object for I2C communications
// using the linux kernel's I2C-interface. (linux/i2c-dev.h, linux/i2c.h
// https://www.kernel.org/doc/Documentation/i2c/dev-interface )
// Hardware supporting this interface is e.g. available at the following systems
// 
// *) Beaglebone
// *) Rasperry Pi
// 
// An I2C device at adress I2Caddr ( numbering as used by i2cdetect; Please compare to other notations!)
// is opened via the I2C-bus given by I2CDevicename (e.g. "/dev/i2c-0", "/dev/i2c-1", ...)
// 
// Currently there are only functions for writing to the bus.
// 

  // add a postfix that identifies the type of the shared object
  ObjectIdentifyer = ObjectIdentifyer + ".I2CDevice_ShObj";


// introduce some parameters that are refered to by id's

   // pack all parameters into a structure "parlist"
   parlist = new_irparam_set();

   parlist = new_irparam_elemet_ivec(parlist, I2Caddr, 10); // id = 10
   parlist = new_irparam_elemet_ivec(parlist, ascii(I2CDevicename), 12); // id = 12; A string parameter

   p = combine_irparam(parlist); // convert to two vectors of integers and floating point values respectively

// Set-up the block parameters. There are no I/O ports
  Uipar = [ p.ipar ];
  Urpar = [ p.rpar ];
  btype = ORTD_BLOCKIDSTART + 10; // Reference to the block's type (computational function). Use the same id you are giving via the "libdyn_compfnlist_add" C-function

  [sim] =  libdyn_CreateSharedObjBlk(sim, btype, ObjectIdentifyer, Visibility, Uipar, Urpar); 
endfunction


function [sim] = ld_I2CDevice_Write(sim, events, ObjectIdentifyer, Register, in) // PARSEDOCU_BLOCK
// 
// %PURPOSE: Write to a one byte register of an I2C-device
// 
// 
// 

  ortd_checkpar(sim, list('SingleValue', 'Register', Register) );
  ortd_checkpar(sim, list('Signal', 'in', in) );

  // add a postfix that identifies the type of the shared object
  ObjectIdentifyer = ObjectIdentifyer + ".I2CDevice_ShObj";

// introduce some parameters that are refered to by id's


   // pack all parameters into a structure "parlist"
   parlist = new_irparam_set();

   parlist = new_irparam_elemet_ivec(parlist, Register, 10); // id = 10

   p = combine_irparam(parlist); // convert to two vectors of integers and floating point values respectively

// Set-up the block parameters and I/O ports
  Uipar = [ p.ipar ];
  Urpar = [ p.rpar ];
  btype = ORTD_BLOCKIDSTART + 11; // Reference to the block's type (computational function). Use the same id you are giving via the "libdyn_compfnlist_add" C-function

  insizes=[1]; // Input port sizes
  outsizes=[]; // Output port sizes
  dfeed=[1];  // for each output 0 (no df) or 1 (a direct feedthrough to one of the inputs)
  intypes=[ORTD.DATATYPE_INT32]; // datatype for each input port
  outtypes=[]; // datatype for each output port

  blocktype = 1; // 1-BLOCKTYPE_DYNAMIC (if block uses states), 2-BLOCKTYPE_STATIC (if there is only a static relationship between in- and output)

  // Create the block
  [sim, blk] = libdyn_CreateBlockAutoConfig(sim, events, btype, blocktype, Uipar, Urpar, insizes, outsizes, intypes, outtypes, dfeed, ObjectIdentifyer);
  
  // connect the inputs
 [sim,blk] = libdyn_conn_equation(sim, blk, list(in) ); // connect in1 to port 0 and in2 to port 1

  // connect the ouputs
//  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim] = ld_I2CDevice_BufferWrite(sim, events, ObjectIdentifyer, in, vecsize) // PARSEDOCU_BLOCK
// 
// %PURPOSE: Raw write of vecsize bytes to an I2C-bus.
// 
// The Values from multiple calls of this block are buffered untile they are
// send by ld_I2CDevice_Transmit.
// 
// Note: The maximal buffer size currently is 1000 bytes.
// 

  ortd_checkpar(sim, list('SingleValue', 'vecsize', vecsize) );
  ortd_checkpar(sim, list('Signal', 'in', in) );

  // add a postfix that identifies the type of the shared object
  ObjectIdentifyer = ObjectIdentifyer + ".I2CDevice_ShObj";

// introduce some parameters that are refered to by id's


   // pack all parameters into a structure "parlist"
   parlist = new_irparam_set();

//    parlist = new_irparam_elemet_ivec(parlist, Header, 10); // id = 10
//    parlist = new_irparam_elemet_ivec(parlist, Footer, 11); // id = 10

   p = combine_irparam(parlist); // convert to two vectors of integers and floating point values respectively

// Set-up the block parameters and I/O ports
  Uipar = [ p.ipar ];
  Urpar = [ p.rpar ];
  btype = ORTD_BLOCKIDSTART + 12; // Reference to the block's type (computational function). Use the same id you are giving via the "libdyn_compfnlist_add" C-function

  insizes=[vecsize]; // Input port sizes
  outsizes=[]; // Output port sizes
  dfeed=[1];  // for each output 0 (no df) or 1 (a direct feedthrough to one of the inputs)
  intypes=[ORTD.DATATYPE_INT32]; // datatype for each input port
  outtypes=[]; // datatype for each output port

  blocktype = 1; // 1-BLOCKTYPE_DYNAMIC (if block uses states), 2-BLOCKTYPE_STATIC (if there is only a static relationship between in- and output)

  // Create the block
  [sim, blk] = libdyn_CreateBlockAutoConfig(sim, events, btype, blocktype, Uipar, Urpar, insizes, outsizes, intypes, outtypes, dfeed, ObjectIdentifyer);
  
  // connect the inputs
 [sim,blk] = libdyn_conn_equation(sim, blk, list(in) ); // connect in1 to port 0 and in2 to port 1

  // connect the ouputs
//  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim] = ld_I2CDevice_Transmit(sim, events, ObjectIdentifyer) // PARSEDOCU_BLOCK
// 
// %PURPOSE: Transmit / Flush write buffers and send the collected data to the device.
// 
// 

  // add a postfix that identifies the type of the shared object
  ObjectIdentifyer = ObjectIdentifyer + ".I2CDevice_ShObj";

// introduce some parameters that are refered to by id's


   // pack all parameters into a structure "parlist"
   parlist = new_irparam_set();

//    parlist = new_irparam_elemet_ivec(parlist, Header, 10); // id = 10
//    parlist = new_irparam_elemet_ivec(parlist, Footer, 11); // id = 10

   p = combine_irparam(parlist); // convert to two vectors of integers and floating point values respectively

// Set-up the block parameters and I/O ports
  Uipar = [ p.ipar ];
  Urpar = [ p.rpar ];
  btype = ORTD_BLOCKIDSTART + 13; // Reference to the block's type (computational function). Use the same id you are giving via the "libdyn_compfnlist_add" C-function

  insizes=[1]; // Input port sizes
  outsizes=[]; // Output port sizes
  dfeed=[1];  // for each output 0 (no df) or 1 (a direct feedthrough to one of the inputs)
  intypes=[ORTD.DATATYPE_FLOAT]; // datatype for each input port
  outtypes=[]; // datatype for each output port

  blocktype = 1; // 1-BLOCKTYPE_DYNAMIC (if block uses states), 2-BLOCKTYPE_STATIC (if there is only a static relationship between in- and output)

  // Create the block
  [sim, blk] = libdyn_CreateBlockAutoConfig(sim, events, btype, blocktype, Uipar, Urpar, insizes, outsizes, intypes, outtypes, dfeed, ObjectIdentifyer);
  
  // connect the inputs
  [sim, dummy] = ld_const(sim, 0, 0); // dummy input FIXM. There is a function for this
 [sim,blk] = libdyn_conn_equation(sim, blk, list(dummy) ); // connect in1 to port 0 and in2 to port 1

  // connect the ouputs
//  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction



function [sim, out] = ld_I2CDevice_Read(sim, events, ObjectIdentifyer, Register) // PARSEDOCU_BLOCK
// 
// %PURPOSE: read one byte from a register of an I2C-device
// 
// 
// 

  ortd_checkpar(sim, list('SingleValue', 'Register', Register) );
//   ortd_checkpar(sim, list('Signal', 'in', in) );

  // add a postfix that identifies the type of the shared object
  ObjectIdentifyer = ObjectIdentifyer + ".I2CDevice_ShObj";

// introduce some parameters that are refered to by id's


   // pack all parameters into a structure "parlist"
   parlist = new_irparam_set();

   parlist = new_irparam_elemet_ivec(parlist, Register, 10); // id = 10

   p = combine_irparam(parlist); // convert to two vectors of integers and floating point values respectively

// Set-up the block parameters and I/O ports
  Uipar = [ p.ipar ];
  Urpar = [ p.rpar ];
  btype = ORTD_BLOCKIDSTART + 14; // Reference to the block's type (computational function). Use the same id you are giving via the "libdyn_compfnlist_add" C-function

  insizes=[]; // Input port sizes
  outsizes=[1]; // Output port sizes
  dfeed=[1];  // for each output 0 (no df) or 1 (a direct feedthrough to one of the inputs)
  intypes=[]; // datatype for each input port
  outtypes=[ORTD.DATATYPE_INT32]; // datatype for each output port

  blocktype = 1; // 1-BLOCKTYPE_DYNAMIC (if block uses states), 2-BLOCKTYPE_STATIC (if there is only a static relationship between in- and output)

  // Create the block
  [sim, blk] = libdyn_CreateBlockAutoConfig(sim, events, btype, blocktype, Uipar, Urpar, insizes, outsizes, intypes, outtypes, dfeed, ObjectIdentifyer);
  
  // connect the inputs
 [sim,blk] = libdyn_conn_equation(sim, blk, list() ); // connect in1 to port 0 and in2 to port 1

  // connect the ouputs
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction




