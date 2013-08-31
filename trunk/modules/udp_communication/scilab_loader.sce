

// Interfacing functions are placed in this place

function [sim, out] = ld_udp_main_receiver(sim, events, udpport, identstr, socket_fname, vecsize) // PARSEDOCU_BLOCK
// udp main receiver - block
//
// This is a simulation-synchronising Block
// 
// EXPERIMENTAL FIXME: REMOVE
// 

  datatype = ORTD.DATATYPE_FLOAT;

  btype = 39001;
  [sim,blk] = libdyn_new_block(sim, events, btype, ipar=[ udpport, vecsize, datatype, length(socket_fname), ascii(socket_fname), length(identstr), ascii(identstr) ], rpar=[  ], ...
                   insizes=[], outsizes=[vecsize], ...
                   intypes=[], outtypes=[ORTD.DATATYPE_FLOAT]  );

  //[sim,blk] = libdyn_conn_equation(sim, blk, list(in) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction




function [sim] = ld_UDPSocket_shObj(sim, events, ObjectIdentifyer, Visibility, hostname, UDPPort) // PARSEDOCU_BLOCK
// 
// Set-up an UDP-Socket
//
// hostname - Network interface to bind socket to???
// UDPPort - UDP port to bind. If -1 then no UDP server is set-up
// 
// EXPERIMENTAL
// 

  // add a postfix that identifies the type of the shared object
  ObjectIdentifyer = ObjectIdentifyer + ".UDPSocket_ShObj";



   // pack all parameters into a structure "parlist"
   parlist = new_irparam_set();

   parlist = new_irparam_elemet_ivec(parlist, UDPPort, 10); // id = 10
   parlist = new_irparam_elemet_ivec(parlist, ascii(hostname), 11); // id = 11; A string parameter

   p = combine_irparam(parlist); // convert to two vectors of integers and floating point values respectively

// Set-up the block parameters. There are no I/O ports
  Uipar = [ p.ipar ];
  Urpar = [ p.rpar ];
  btype = 39001 + 0; // Reference to the block's type (computational function). Use the same id you are giving via the "libdyn_compfnlist_add" C-function

  [sim] =  libdyn_CreateSharedObjBlk(sim, btype, ObjectIdentifyer, Visibility, Uipar, Urpar); 
endfunction

function [sim] = ld_UDPSocket_Send(sim, events, ObjectIdentifyer, in, insize, intype) // PARSEDOCU_BLOCK
// 
// UDP - Send block
//
// in *, ORTD.DATATYPE_BINARY - input
// 
// EXPERIMENTAL, About to be removed
// 

  // add a postfix that identifies the type of the shared object
  ObjectIdentifyer = ObjectIdentifyer + ".UDPSocket_ShObj";


   // pack all parameters into a structure "parlist"
   parlist = new_irparam_set();

   parlist = new_irparam_elemet_ivec(parlist, insize, 10); // id = 10
   parlist = new_irparam_elemet_ivec(parlist, intype, 11); // id = 11

   p = combine_irparam(parlist); // convert to two vectors of integers and floating point values respectively

// Set-up the block parameters and I/O ports
  Uipar = [ p.ipar ];
  Urpar = [ p.rpar ];
  btype = 39001 + 1; // Reference to the block's type (computational function). Use the same id you are giving via the "libdyn_compfnlist_add" C-function

  insizes=[insize]; // Input port sizes
  outsizes=[]; // Output port sizes
  dfeed=[1];  // for each output 0 (no df) or 1 (a direct feedthrough to one of the inputs)
  intypes=[intype]; // datatype for each input port
  outtypes=[]; // datatype for each output port

  blocktype = 1; // 1-BLOCKTYPE_DYNAMIC (if block uses states), 2-BLOCKTYPE_STATIC (if there is only a static relationship between in- and output)

  // Create the block
  [sim, blk] = libdyn_CreateBlockAutoConfig(sim, events, btype, blocktype, Uipar, Urpar, insizes, outsizes, intypes, outtypes, dfeed, ObjectIdentifyer);
  
  // connect the inputs
 [sim,blk] = libdyn_conn_equation(sim, blk, list(in) ); // connect in1 to port 0 and in2 to port 1

//   // connect the ouputs
//  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction


function [sim, out, SrcAddr] = ld_UDPSocket_Recv(sim, events, ObjectIdentifyer, outsize) // PARSEDOCU_BLOCK
// 
// UDP - receiver block
//
// out *, ORTD.DATATYPE_BINARY - output
// SrcAddr - information about where the package comes from (not implemented)
// 
// This is a simulation-synchronising Block. Everytime an UDP-Packet is received,
// the simulation that contains this blocks goes on for one step.
// 
// EXPERIMENTAL
// 

  printf("Synchronising simulation to UDP-Receiver\n");

  // add a postfix that identifies the type of the shared object
  ObjectIdentifyer = ObjectIdentifyer + ".UDPSocket_ShObj";

  //
  outtype = ORTD.DATATYPE_BINARY;

  // IPv4
  AddrSize = 4+2; // IPnumber + port

   // pack all parameters into a structure "parlist"
   parlist = new_irparam_set();

   parlist = new_irparam_elemet_ivec(parlist, outsize, 10); // id = 10
   parlist = new_irparam_elemet_ivec(parlist, outtype, 11); // id = 11

   p = combine_irparam(parlist); // convert to two vectors of integers and floating point values respectively

// Set-up the block parameters and I/O ports
  Uipar = [ p.ipar ];
  Urpar = [ p.rpar ];
  btype = 39001 + 2; // Reference to the block's type (computational function). Use the same id you are giving via the "libdyn_compfnlist_add" C-function

  insizes=[]; // Input port sizes
  outsizes=[outsize, AddrSize]; // Output port sizes
  dfeed=[1];  // for each output 0 (no df) or 1 (a direct feedthrough to one of the inputs)
  intypes=[]; // datatype for each input port
  outtypes=[outtype, ORTD.DATATYPE_BINARY]; // datatype for each output port

  blocktype = 1; // 1-BLOCKTYPE_DYNAMIC (if block uses states), 2-BLOCKTYPE_STATIC (if there is only a static relationship between in- and output)

  // Create the block
  [sim, blk] = libdyn_CreateBlockAutoConfig(sim, events, btype, blocktype, Uipar, Urpar, insizes, outsizes, intypes, outtypes, dfeed, ObjectIdentifyer);
  
//   // connect the inputs
//  [sim,blk] = libdyn_conn_equation(sim, blk, list(in) ); // connect in1 to port 0 and in2 to port 1

   // connect the ouputs
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
  [sim,SrcAddr] = libdyn_new_oport_hint(sim, blk, 1);   // 1th port
endfunction

function [sim] = ld_UDPSocket_SendTo(sim, events, SendSize, ObjectIdentifyer, hostname, UDPPort, in, insize) // PARSEDOCU_BLOCK
// 
// UDP - Send block
//
// in *, ORTD.DATATYPE_BINARY - input
// SendSize *. ORTD.DATATYPE_INT32 - Number of bytes to send
// 
// EXPERIMENTAL
// 


  // add a postfix that identifies the type of the shared object
  ObjectIdentifyer = ObjectIdentifyer + ".UDPSocket_ShObj";

  // only send binary data
  intype=ORTD.DATATYPE_BINARY;


   // pack all parameters into a structure "parlist"
   parlist = new_irparam_set();

   parlist = new_irparam_elemet_ivec(parlist, insize, 10); // id = 10
   parlist = new_irparam_elemet_ivec(parlist, intype, 11); // id = 11
   
   parlist = new_irparam_elemet_ivec(parlist, UDPPort, 12); // id = 10
   parlist = new_irparam_elemet_ivec(parlist, ascii(hostname), 13); // id = 11; A string parameter
   

   p = combine_irparam(parlist); // convert to two vectors of integers and floating point values respectively

// Set-up the block parameters and I/O ports
  Uipar = [ p.ipar ];
  Urpar = [ p.rpar ];
  btype = 39001 + 3; // Reference to the block's type (computational function). Use the same id you are giving via the "libdyn_compfnlist_add" C-function

  insizes=[insize, 1]; // Input port sizes
  outsizes=[]; // Output port sizes
  dfeed=[1];  // for each output 0 (no df) or 1 (a direct feedthrough to one of the inputs)
  intypes=[intype, ORTD.DATATYPE_INT32  ]; // datatype for each input port
  outtypes=[]; // datatype for each output port

  blocktype = 1; // 1-BLOCKTYPE_DYNAMIC (if block uses states), 2-BLOCKTYPE_STATIC (if there is only a static relationship between in- and output)

  // Create the block
  [sim, blk] = libdyn_CreateBlockAutoConfig(sim, events, btype, blocktype, Uipar, Urpar, insizes, outsizes, intypes, outtypes, dfeed, ObjectIdentifyer);
  
  // connect the inputs
 [sim,blk] = libdyn_conn_equation(sim, blk, list(in, SendSize) ); // connect in1 to port 0 and in2 to port 1

//   // connect the ouputs
//  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction






function [sim] = ld_UDPSocket_Reply(sim, events, SendSize, ObjectIdentifyer, DestAddr, in, insize) // PARSEDOCU_BLOCK
// 
// UDP - Send block
//
// in *, ORTD.DATATYPE_BINARY - input
// SendSize *. ORTD.DATATYPE_INT32 - Number of bytes to send
// DestAddr - dynamic representation for the destination address
// 
// EXPERIMENTAL not implemented by now
// 


  // add a postfix that identifies the type of the shared object
  ObjectIdentifyer = ObjectIdentifyer + ".UDPSocket_ShObj";

  // only send binary data
  intype=ORTD.DATATYPE_BINARY;

   // IPv4
  AddrSize=4+2;

   // pack all parameters into a structure "parlist"
   parlist = new_irparam_set();

   parlist = new_irparam_elemet_ivec(parlist, insize, 10); // id = 10
   parlist = new_irparam_elemet_ivec(parlist, intype, 11); // id = 11
   
//    parlist = new_irparam_elemet_ivec(parlist, UDPPort, 12); // id = 10
//    parlist = new_irparam_elemet_ivec(parlist, ascii(hostname), 13); // id = 11; A string parameter
   

   p = combine_irparam(parlist); // convert to two vectors of integers and floating point values respectively

// Set-up the block parameters and I/O ports
  Uipar = [ p.ipar ];
  Urpar = [ p.rpar ];
  btype = 39001 + 4; // Reference to the block's type (computational function). Use the same id you are giving via the "libdyn_compfnlist_add" C-function

  insizes=[insize, 1, AddrSize]; // Input port sizes
  outsizes=[]; // Output port sizes
  dfeed=[1];  // for each output 0 (no df) or 1 (a direct feedthrough to one of the inputs)
  intypes=[intype, ORTD.DATATYPE_INT32, ORTD.DATATYPE_BINARY  ]; // datatype for each input port
  outtypes=[]; // datatype for each output port

  blocktype = 1; // 1-BLOCKTYPE_DYNAMIC (if block uses states), 2-BLOCKTYPE_STATIC (if there is only a static relationship between in- and output)

  // Create the block
  [sim, blk] = libdyn_CreateBlockAutoConfig(sim, events, btype, blocktype, Uipar, Urpar, insizes, outsizes, intypes, outtypes, dfeed, ObjectIdentifyer);
  
  // connect the inputs
 [sim,blk] = libdyn_conn_equation(sim, blk, list(in, SendSize, DestAddr) ); // connect in1 to port 0 and in2 to port 1

//   // connect the ouputs
//  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction












function [sim, out, NBytes] = ld_ConcateData(sim, events, inlist, insizes, intypes) // PARSEDOCU_BLOCK
// 
// Concate Data - block
//
// concatenates the binary representation of all inputs
// 
// The output is of type ORTD.DATATYPE_BINARY
// 
// EXPERIMENTAL
// 


   // pack all parameters into a structure "parlist"
//    parlist = new_irparam_set();
// 
//    parlist = new_irparam_elemet_ivec(parlist, insize, 10); // id = 10
//    parlist = new_irparam_elemet_ivec(parlist, intype, 11); // id = 11
// 
//    p = combine_irparam(parlist); // convert to two vectors of integers and floating point values respectively

// Set-up the block parameters and I/O ports
  Uipar = [   ];
  Urpar = [   ];
  btype = 39001 + 10; // Reference to the block's type (computational function). Use the same id you are giving via the "libdyn_compfnlist_add" C-function

  // count the number of bytes
  NBytes = 0;
  for i = 1:length(inlist)
    NBytes = NBytes + insizes(i) * libdyn_datatype_len( intypes(i) );
  end



//   insizes=[insizes]; // Input port sizes
  outsizes=[ NBytes ]; // Output port sizes
  dfeed=[1];  // for each output 0 (no df) or 1 (a direct feedthrough to one of the inputs)
//   intypes=[intypes]; // datatype for each input port
  outtypes=[ ORTD.DATATYPE_BINARY  ]; // datatype for each output port

// disp(outsizes);
// disp(outtypes);

  blocktype = 2; // 1-BLOCKTYPE_DYNAMIC (if block uses states), 2-BLOCKTYPE_STATIC (if there is only a static relationship between in- and output)

  // Create the block
  [sim, blk] = libdyn_CreateBlockAutoConfig(sim, events, btype, blocktype, Uipar, Urpar, insizes, outsizes, intypes, outtypes, dfeed);
  
  // connect the inputs
//   for i = 1:length(inlist)    
    [sim,blk] = libdyn_conn_equation(sim, blk, inlist ); // connect in1 to port 0 and in2 to port 1
//   end

//   // connect the ouputs
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction


function [sim, outlist] = ld_DisassembleData(sim, events, in, outsizes, outtypes) // PARSEDOCU_BLOCK
// 
// disasseble Data - block
//
// disassemble the binary representation of the input, which is of type ORTD.DATATYPE_BINARY
// 
// EXPERIMENTAL
// 


   // pack all parameters into a structure "parlist"
//    parlist = new_irparam_set();
// 
//    parlist = new_irparam_elemet_ivec(parlist, insize, 10); // id = 10
//    parlist = new_irparam_elemet_ivec(parlist, intype, 11); // id = 11
// 
//    p = combine_irparam(parlist); // convert to two vectors of integers and floating point values respectively

// Set-up the block parameters and I/O ports
  Uipar = [   ];
  Urpar = [   ];
  btype = 39001 + 11; // Reference to the block's type (computational function). Use the same id you are giving via the "libdyn_compfnlist_add" C-function

  // count the number of bytes
  NBytes = 0;
  for i = 1:length(outsizes)
    NBytes = NBytes + outsizes(i)*libdyn_datatype_len( outtypes(i) );
  end



//   insizes=[insizes]; // Input port sizes
  insizes=[ NBytes ]; // Output port sizes
  dfeed=[1];  // for each output 0 (no df) or 1 (a direct feedthrough to one of the inputs)
//   intypes=[intypes]; // datatype for each input port
  intypes=[ ORTD.DATATYPE_BINARY  ]; // datatype for each output port

// disp(outsizes);
// disp(outtypes);

  blocktype = 2; // 1-BLOCKTYPE_DYNAMIC (if block uses states), 2-BLOCKTYPE_STATIC (if there is only a static relationship between in- and output)

  // Create the block
  [sim, blk] = libdyn_CreateBlockAutoConfig(sim, events, btype, blocktype, Uipar, Urpar, insizes, outsizes, intypes, outtypes, dfeed);
  
  // connect the inputs
//   for i = 1:length(inlist)    
    [sim,blk] = libdyn_conn_equation(sim, blk, list(in) ); // connect in1 to port 0 and in2 to port 1
//   end

//   // connect the ouputs
  outlist = list();
  for i = 1:length(outtypes)
    [sim,outlist($+1)] = libdyn_new_oport_hint(sim, blk, i-1);   // 0th port
  end
endfunction



