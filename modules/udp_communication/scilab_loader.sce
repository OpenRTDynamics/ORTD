

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










// 
// 
//   A packet based communication interface from ORTD using UDP datagrams to e.g.
//   nodejs. 
//   webappUDP.js is the counterpart that provides a web-interface 
// 
// Current Rev: 7
// 
// Revisions:
// 
// 27.3.14 - possibility to reservate sources
// 3.4.14  - small re-arrangements
// 4.4.14  - Bugfixes
// 7.4.14  - Bugfix
// 12.6.14 - Bugfix
// 2.11.14 - Added group finalising packet
// 


function [PacketFramework,SourceID] = ld_PF_addsource(PacketFramework, NValues_send, datatype, SourceName)
  SourceID = PacketFramework.SourceID_counter;

  Source.SourceName = SourceName;
  Source.SourceID = SourceID;
  Source.NValues_send = NValues_send;
  Source.datatype =  datatype;
  
  // Add new source to the list
  PacketFramework.Sources($+1) = Source;
  
  // inc counter
  PacketFramework.SourceID_counter = PacketFramework.SourceID_counter + 1;
endfunction

function [PacketFramework,ParameterID,MemoryOfs] = ld_PF_addparameter(PacketFramework, NValues, datatype, ParameterName)
  ParameterID = PacketFramework.Parameterid_counter;

  Parameter.ParameterName = ParameterName;
  Parameter.ParameterID = ParameterID;
  Parameter.NValues = NValues;
  Parameter.datatype =  datatype;
  Parameter.MemoryOfs = PacketFramework.ParameterMemOfs_counter;
  
  // Add new source to the list
  PacketFramework.Parameters($+1) = Parameter;
  
  // inc counters
  PacketFramework.Parameterid_counter = PacketFramework.Parameterid_counter + 1;
  PacketFramework.ParameterMemOfs_counter = PacketFramework.ParameterMemOfs_counter + NValues;

  // return values
  ParameterID = Parameter.ParameterID; 
  MemoryOfs = Parameter.MemoryOfs;
endfunction

function [sim, PacketFramework, Parameter] = ld_PF_Parameter(sim, PacketFramework, NValues, datatype, ParameterName) // PARSEDOCU_BLOCK
// 
// Define a parameter
// 
// NValues - amount of data sets
// datatype - only ORTD.DATATYPE_FLOAT for now
// ParameterName - a unique string decribing the parameter
// 
// 
// 

    [PacketFramework,ParameterID,MemoryOfs] = ld_PF_addparameter(PacketFramework, NValues, datatype, ParameterName);
   
    // read data from global memory
    [sim, readI] = ld_const(sim, 0, MemoryOfs); // start at index 1
    [sim, Parameter] = ld_read_global_memory(sim, 0, index=readI, ident_str=PacketFramework.InstanceName+"Memory", ...
						datatype, NValues);
endfunction




// Send a signal via UDP, a simple protocoll is defined, internal function
function [sim] = ld_PF_ISendUDP(sim, PacketFramework, Signal, NValues_send, datatype, SourceID)
  InstanceName = PacketFramework.InstanceName;
  [sim,one] = ld_const(sim, 0, 1);

  // Packet counter, so the order of the network packages can be determined
  [sim, Counter] = ld_modcounter(sim, 0, in=one, initial_count=0, mod=100000);
  [sim, Counter_int32] = ld_ceilInt32(sim, 0, Counter);

  // Source ID
  [sim, SourceID] = ld_const(sim, 0, SourceID);
  [sim, SourceID_int32] = ld_ceilInt32(sim, 0, SourceID);

  // Sender ID
  [sim, SenderID] = ld_const(sim, 0, PacketFramework.SenderID); // random number
  [sim, SenderID_int32] = ld_ceilInt32(sim, 0, SenderID);

  // make a binary structure
  [sim, Data, NBytes] = ld_ConcateData(sim, 0, ...
			inlist=list(SenderID_int32, Counter_int32, SourceID_int32, Signal ), insizes=[1,1,1,NValues_send], ...
			intypes=[ ORTD.DATATYPE_INT32, ORTD.DATATYPE_INT32, ORTD.DATATYPE_INT32, datatype ] );

  printf("The size of the UDP-packets will be %d bytes.\n", NBytes);

  // send to the network 
  [sim, NBytes__] = ld_constvecInt32(sim, 0, vec=NBytes); // the number of bytes that are actually send is dynamic, but must be smaller or equal to 
  [sim] = ld_UDPSocket_SendTo(sim, 0, SendSize=NBytes__, ObjectIdentifyer=InstanceName+"aSocket", ...
			      hostname=PacketFramework.Configuration.DestHost, ...
                              UDPPort=PacketFramework.Configuration.DestPort, in=Data, ...
			      insize=NBytes);

endfunction




function [sim, PacketFramework] = ld_SendPacket(sim, PacketFramework, Signal, NValues_send, datatype, SourceName) // PARSEDOCU_BLOCK // PARSEDOCU_BLOCK
// 
// Stream data - block
// 
// Signal - the signal to stream
// NValues_send - the vector length of Signal
// datatype - only ORTD.DATATYPE_FLOAT by now
// SourceName - a unique string identifier descring the stream
// 
// 
// 

  [PacketFramework,SourceID] = ld_PF_addsource(PacketFramework, NValues_send, datatype, SourceName);
  [sim]=ld_PF_ISendUDP(sim, PacketFramework, Signal, NValues_send, datatype, SourceID);
endfunction




function [sim, PacketFramework] = ld_PF_InitInstance(sim, InstanceName, Configuration) // PARSEDOCU_BLOCK
// 
// Initialise an instance of the Packet Framework
//   
// InstanceName - a unique string identifier for the instance
// Configuration must include the following properties:
// 
//   Configuration.UnderlyingProtocoll = "UDP"
//   Configuration.DestHost
//   Configuration.DestPort
//   Configuration.LocalSocketHost
//   Configuration.LocalSocketPort
// 
// 
// Example:
// 
// 
//   Configuration.UnderlyingProtocoll = "UDP";
//   Configuration.DestHost = "127.0.0.1";
//   Configuration.DestPort = 20000;
//   Configuration.LocalSocketHost = "127.0.0.1";
//   Configuration.LocalSocketPort = 20001;
//   [sim, PacketFramework] = ld_PF_InitInstance(sim, InstanceName="UDPCommunication", Configuration);
// 
// 
// 
// Also consider the file webappUDP.js as the counterpart that communicates to ORTD-simulations
// 
// 

  // initialise structure for sources
  PacketFramework.InstanceName = InstanceName;
  PacketFramework.Configuration = Configuration;

  PacketFramework.Configuration.debugmode = %F;

//   disp(Configuration.UnderlyingProtocoll)

  if Configuration.UnderlyingProtocoll == 'UDP'
    null;
  else
    error("PacketFramework: Only UDP supported up to now");
  end

  // possible packet sizes for UDP
  PacketFramework.TotalElemetsPerPacket = floor((1400-3*4)/8); // number of doubles values that fit into one UDP-packet with maximal size of 1400 bytes
  PacketFramework.PacketSize = PacketFramework.TotalElemetsPerPacket*8 + 3*4;

  // sources
  PacketFramework.SourceID_counter = 0;
  PacketFramework.Sources = list();
  
  // parameters
  PacketFramework.Parameterid_counter = 0;
  PacketFramework.ParameterMemOfs_counter = 1; // start at the first index in the memory
  PacketFramework.Parameters = list();
  
  PacketFramework.SenderID = 1295793;
endfunction


// Send a signal via UDP, a simple protocoll is defined, internal function
function [sim] = ld_PF_SendGroupFinshUDP(sim, PacketFramework, GroupID)
  InstanceName = PacketFramework.InstanceName;
  [sim,one] = ld_const(sim, 0, 1);

  // Packet counter, so the order of the network packages can be determined
  [sim, Counter] = ld_modcounter(sim, 0, in=one, initial_count=0, mod=100000);
  [sim, Counter_int32] = ld_ceilInt32(sim, 0, Counter);

  // Source ID
  [sim, SourceID] = ld_const(sim, 0, -1);                   // -1 means finish a group of sources
  [sim, SourceID_int32] = ld_ceilInt32(sim, 0, SourceID);

  // Group ID
  [sim, GroupID_] = ld_const(sim, 0, GroupID);                   // -1 means finish a group of sources
  [sim, GroupID_int32] = ld_ceilInt32(sim, 0, GroupID_);

  // Sender ID
  [sim, SenderID] = ld_const(sim, 0, PacketFramework.SenderID); // random number
  [sim, SenderID_int32] = ld_ceilInt32(sim, 0, SenderID);

  // make a binary structure
  [sim, Data, NBytes] = ld_ConcateData(sim, 0, ...
			inlist=list(SenderID_int32, Counter_int32, SourceID_int32, GroupID_int32 ), insizes=[1,1,1,1], ...
			intypes=[ ORTD.DATATYPE_INT32, ORTD.DATATYPE_INT32, ORTD.DATATYPE_INT32, ORTD.DATATYPE_INT32 ] );

//  printf("The size of the UDP-packets will be %d bytes.\n", NBytes);

  // send to the network 
  [sim, NBytes__] = ld_constvecInt32(sim, 0, vec=NBytes); // the number of bytes that are actually send is dynamic, but must be smaller or equal to 
  [sim] = ld_UDPSocket_SendTo(sim, 0, SendSize=NBytes__, ObjectIdentifyer=InstanceName+"aSocket", ...
			      hostname=PacketFramework.Configuration.DestHost, ...
                              UDPPort=PacketFramework.Configuration.DestPort, in=Data, ...
			      insize=NBytes);


  [sim] = ld_printf(sim, ev, GroupID_, "Sent finish packet ", 1);
endfunction

function [sim,PacketFramework] = ld_PF_Finalise(sim,PacketFramework) // PARSEDOCU_BLOCK
// 
// Finalise the instance.
// 
// 

      // The main real-time thread
      function [sim] = ld_PF_InitUDP(sim, InstanceName, ParameterMemory)

	  function [sim, outlist, userdata] = UDPReceiverThread(sim, inlist, userdata)
	    // This will run in a thread. Each time a UDP-packet is received 
	    // one simulation step is performed. Herein, the packet is parsed
	    // and the contained parameters are stored into a memory.

	    // Sync the simulation to incomming UDP-packets
	    [sim, Data, SrcAddr] = ld_UDPSocket_Recv(sim, 0, ObjectIdentifyer=InstanceName+"aSocket", outsize=PacketSize );

	    // disassemble packet's structure
	    [sim, DisAsm] = ld_DisassembleData(sim, 0, in=Data, ...
				  outsizes=[1,1,1,TotalElemetsPerPacket], ...
				  outtypes=[ ORTD.DATATYPE_INT32, ORTD.DATATYPE_INT32, ORTD.DATATYPE_INT32, ORTD.DATATYPE_FLOAT ] );



            DisAsm_ = list();
            DisAsm_(4) = DisAsm(4);
	    [sim, DisAsm_(1)] = ld_Int32ToFloat(sim, 0, DisAsm(1) );
	    [sim, DisAsm_(2)] = ld_Int32ToFloat(sim, 0, DisAsm(2) );
	    [sim, DisAsm_(3)] = ld_Int32ToFloat(sim, 0, DisAsm(3) );


            [sim, memofs] = ld_ArrayInt32(sim, 0, array=ParameterMemory.MemoryOfs, in=DisAsm(3) );
            [sim, Nelements] = ld_ArrayInt32(sim, 0, array=ParameterMemory.Sizes, in=DisAsm(3) );

 	    [sim, memofs_] = ld_Int32ToFloat(sim, 0, memofs );
 	    [sim, Nelements_] = ld_Int32ToFloat(sim, 0, Nelements );

            if PacketFramework.Configuration.debugmode then 
	      // print the contents of the packet
	      [sim] = ld_printf(sim, 0, DisAsm_(1), "DisAsm(1) (SenderID)       = ", 1);
	      [sim] = ld_printf(sim, 0, DisAsm_(2), "DisAsm(2) (Packet Counter) = ", 1);
	      [sim] = ld_printf(sim, 0, DisAsm_(3), "DisAsm(3) (SourceID)       = ", 1);
	      [sim] = ld_printf(sim, 0, DisAsm_(4), "DisAsm(4) (Signal)         = ", TotalElemetsPerPacket);

	      [sim] = ld_printf(sim, 0, memofs_ ,  "memofs                    = ", 1);
	      [sim] = ld_printf(sim, 0, memofs_ ,  "Nelements                 = ", 1);
            end

	    // Store the input data into a shared memory
	    [sim] = ld_WriteMemory2(sim, 0, data=DisAsm(4), index=memofs, ElementsToWrite=Nelements, ...
					  ident_str=InstanceName+"Memory", datatype=ORTD.DATATYPE_FLOAT, MaxElements=TotalElemetsPerPacket );



	    // output of schematic
	    outlist = list();
	  endfunction

	
	
	// start the node.js service from the subfolder webinterface
	//[sim, out] = ld_startproc2(sim, 0, exepath="./webappUDP.sh", chpwd="webinterface", prio=0, whentorun=0);
	
        TotalMemorySize = sum(PacketFramework.ParameterMemory.Sizes);
        TotalElemetsPerPacket = PacketFramework.TotalElemetsPerPacket; // number of doubles values that fit into one UDP-packet with maximal size of 1400 bytes
        PacketSize = PacketFramework.PacketSize;

	// Open an UDP-Port in server mode
	[sim] = ld_UDPSocket_shObj(sim, 0, ObjectIdentifyer=InstanceName+"aSocket", Visibility=0, ...
                                  hostname=PacketFramework.Configuration.LocalSocketHost, ...
                                  UDPPort=PacketFramework.Configuration.LocalSocketPort);

	// initialise a global memory for storing the input data for the computation
	[sim] = ld_global_memory(sim, 0, ident_str=InstanceName+"Memory", ... 
				datatype=ORTD.DATATYPE_FLOAT, len=TotalMemorySize, ...
				initial_data=[zeros(TotalMemorySize,1)], ... 
				visibility='global', useMutex=1);

	// Create thread for the receiver
	ThreadPrioStruct.prio1=ORTD.ORTD_RT_NORMALTASK, ThreadPrioStruct.prio2=0, ThreadPrioStruct.cpu = -1;
	[sim, startcalc] = ld_const(sim, 0, 1); // triggers your computation during each time step
	[sim, outlist, computation_finished] = ld_async_simulation(sim, 0, ...
			      inlist=list(), ...
			      insizes=[], outsizes=[], ...
			      intypes=[], outtypes=[], ...
			      nested_fn = UDPReceiverThread, ...
			      TriggerSignal=startcalc, name=InstanceName+"Thread1", ...
			      ThreadPrioStruct, userdata=list() );


      endfunction





  // calc memory
  MemoryOfs = [];
  Sizes = [];
  // go through all parameters and create memories for all
  for i=1:length(PacketFramework.Parameters)
     P = PacketFramework.Parameters(i);

     Sizes = [Sizes; P.NValues];
     MemoryOfs = [MemoryOfs; P.MemoryOfs];
  end
  
  PacketFramework.ParameterMemory.MemoryOfs = MemoryOfs;
  PacketFramework.ParameterMemory.Sizes = Sizes;

  // udp
  [sim] = ld_PF_InitUDP(sim, PacketFramework.InstanceName, PacketFramework.ParameterMemory);
  
  // Send to group update notifications for each group (currently only one possible)
  [sim] = ld_PF_SendGroupFinshUDP(sim, PacketFramework, GroupID=0);
  
endfunction

function ld_PF_Export_js(PacketFramework, fname) // PARSEDOCU_BLOCK
// 
// Export configuration of the defined protocoll (Sources, Parameters) 
// into JSON-format. This is to be used by software that shall communicate 
// to the real-time system.
// 
// fname - The file name
// 
// 


   fd = mopen(fname,'wt');
  
   mfprintf(fd,' {""SourcesConfig"" : {\n');

  for i=1:length(PacketFramework.Sources)
    
    
    SourceID = PacketFramework.Sources(i).SourceID;
    SourceName =  PacketFramework.Sources(i).SourceName;
    disp(SourceID );
    disp( SourceName );
      
      
     line=sprintf(" ""%s"" : { ""SourceName"" : ""%s"" , ""NValues_send"" : ""%s"", ""datatype"" : ""%s""  } \n", ...
               string(PacketFramework.Sources(i).SourceID), ...
               string(PacketFramework.Sources(i).SourceName), ...
               string(PacketFramework.Sources(i).NValues_send), ...
               string(PacketFramework.Sources(i).datatype) );
      
     
     if i==length(PacketFramework.Sources)
       // finalise the last entry without ","
       printf('%s \n' , line);
       mfprintf(fd,'%s', line);
     else
       printf('%s, \n' , line);
       mfprintf(fd,'%s,', line);
     end
    

  end
  
  

  
   mfprintf(fd,'} , \n ""ParametersConfig"" : {\n');
   
  // go through all parameters and create memories for all
  for i=1:length(PacketFramework.Parameters)
        
    printf("export of parameter %s \n",PacketFramework.Parameters(i).ParameterName );

     line=sprintf(" ""%s"" : { ""ParameterName"" : ""%s"" , ""NValues"" : ""%s"", ""datatype"" : ""%s""  } \n", ...
               string(PacketFramework.Parameters(i).ParameterID), ...
               string(PacketFramework.Parameters(i).ParameterName), ...
               string(PacketFramework.Parameters(i).NValues), ...
               string(PacketFramework.Parameters(i).datatype) );
      
     
     if i==length(PacketFramework.Parameters) 
       // finalise the last entry without ","
       printf('%s \n' , line);
       mfprintf(fd,'%s', line);
     else
       printf('%s, \n' , line);
       mfprintf(fd,'%s,', line);
     end
    
    
  end  
  
  mfprintf(fd,'}\n}');
  
  mclose(fd);
endfunction

// 
// Added 27.3.14
// 

function [sim, PacketFramework, SourceID]=ld_SendPacketReserve(sim, PacketFramework, NValues_send, datatype, SourceName)
  [PacketFramework,SourceID] = ld_PF_addsource(PacketFramework, NValues_send, datatype, SourceName);
endfunction

function [sim, PacketFramework]=ld_SendPacket2(sim, PacketFramework, Signal, SourceName)
  // find Sourcename
  index  = -1;
  for i=1:length(PacketFramework.Sources)
    S = PacketFramework.Sources(i);
    if S.SourceName == SourceName
      index = i;
      printf(" %s found at index %d Nvalues %d\n", SourceName, index, S.NValues_send);
      break;
    end
  end

  if index == -1
    printf("SourceName = %s\n", SourceName);
    error("SourceName not found! This source must be reservated using ld_SendPacketReserve");
  end

  [sim]=ld_PF_ISendUDP(sim, PacketFramework, Signal, S.NValues_send, S.datatype, S.SourceID);
endfunction



function [sim, PacketFramework, ParameterID]=ld_PF_ParameterReserve(sim, PacketFramework, NValues, datatype, ParameterName)
    [PacketFramework,ParameterID,MemoryOfs] = ld_PF_addparameter(PacketFramework, NValues, datatype, ParameterName);   
endfunction


function [sim, PacketFramework, Parameter]=ld_PF_Parameter2(sim, PacketFramework, ParameterName)
  // find Sourcename
  index  = -1;
  for i=1:length(PacketFramework.Parameters)
    P = PacketFramework.Parameters(i);
    if P.ParameterName == ParameterName
      index = i;
      printf(" %s found at index %d Nvalues %d\n", ParameterName, index, P.NValues);
      break;
    end
  end

  if index == -1
    printf("ParameterName = %s\n", ParameterName);
    error("ParameterName not found! This source must be reservated using ld_PF_ParameterReserve");
  end
   
  // read data from global memory
  [sim, readI] = ld_const(sim, 0, P.MemoryOfs); // start at index 1
  [sim, Parameter] = ld_read_global_memory(sim, 0, index=readI, ident_str=PacketFramework.InstanceName+"Memory", ...
                                           P.datatype, P.NValues);
endfunction

