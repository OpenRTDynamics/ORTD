// 
//     Copyright (C) 2010, 2011, 2012, 2013  Christian Klauer
// 
//     This file is part of OpenRTDynamics, the Real-Time Dynamics Framework
// 
//     OpenRTDynamics is free software: you can redistribute it and/or modify
//     it under the terms of the GNU Lesser General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
// 
//     OpenRTDynamics is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU Lesser General Public License for more details.
// 
//     You should have received a copy of the GNU Lesser General Public License
//     along with OpenRTDynamics.  If not, see <http://www.gnu.org/licenses/>.
// 

SchematicName = 'send';
thispath = get_absolute_file_path(SchematicName+'.sce');
cd(thispath);






z = poly(0,'z');

// 
// Load the interfacing function "ld_TemplateWrite"
// 

exec('../scilab_loader.sce');


//
// Set up simulation schematic
//


// This is the main top level schematic
function [sim, outlist] = schematic_fn(sim, inlist)
  // this is the default event
  ev = 0;

  [sim,one] = ld_const(sim, ev, 1);

  // Open an UDP-Port
  [sim] = ld_UDPSocket_shObj(sim, ev, ObjectIdentifyer="aSocket", Visibility=0, hostname="127.0.0.1", UDPPort=10000);

  // generate a signal
  [sim, Signal ] = ld_play_simple(sim, ev, r=5*sin( linspace(0,%pi*6*10,1000) ) );

  // Packet counter, so the order of the network packages can be determined
  [sim, Counter] = ld_modcounter(sim, ev, in=one, initial_count=0, mod=10000)
  [sim, Counter_int32] = ld_ceilInt32(sim, ev, Counter);

  // Source ID
  [sim, SourceID] = ld_const(sim, ev, 4);
  [sim, SourceID_int32] = ld_ceilInt32(sim, ev, SourceID);

  // Sender ID
  [sim, SenderID] = ld_const(sim, ev, 1295793); // random number
  [sim, SenderID_int32] = ld_ceilInt32(sim, ev, SenderID);


  // print data
  [sim] = ld_printf(sim, ev, Signal, "Signal = ", 1);

  // make a binary structure
  [sim, Data, NBytes] = ld_ConcateData(sim, ev, ...
                         inlist=list(SenderID_int32, Counter_int32, SourceID_int32, Signal ), insizes=[1,1,1,1], ...
                         intypes=[ ORTD.DATATYPE_INT32, ORTD.DATATYPE_INT32, ORTD.DATATYPE_INT32, ORTD.DATATYPE_FLOAT ] );

  printf("Size of the UDP-packets will be %d bytes.\n", NBytes);

  // send to the network
  [sim] = ld_UDPSocket_Send(sim, ev, ObjectIdentifyer="aSocket", ...
                 in=Data, insize=NBytes, intype=ORTD.DATATYPE_BINARY);

  // demo for disassembling this structure -- just to show how it works
  [sim, DisAsm] = ld_DisassembleData(sim, ev, in=Data, ...
                         outsizes=[1,1,1,1], ...
                         outtypes=[ ORTD.DATATYPE_INT32, ORTD.DATATYPE_INT32, ORTD.DATATYPE_INT32, ORTD.DATATYPE_FLOAT ] );

  [sim, DisAsm(1)] = ld_Int32ToFloat(sim, ev, DisAsm(1) );
  [sim, DisAsm(2)] = ld_Int32ToFloat(sim, ev, DisAsm(2) );
  [sim, DisAsm(3)] = ld_Int32ToFloat(sim, ev, DisAsm(3) );

  [sim] = ld_printf(sim, ev, DisAsm(1), "DisAsm(1) (SenderID)       = ", 1);
  [sim] = ld_printf(sim, ev, DisAsm(2), "DisAsm(2) (Packet Counter) = ", 1);
  [sim] = ld_printf(sim, ev, DisAsm(3), "DisAsm(3) (SourceID)       = ", 1);
  [sim] = ld_printf(sim, ev, DisAsm(4), "DisAsm(4) (Signal)         = ", 1);




  // output of schematic
  outlist = list();
endfunction


  
//
// Set-up
//

// defile events
defaultevents = [0]; // main event

// set-up schematic by calling the user defined function "schematic_fn"
insizes = []; outsizes=[];
[sim_container_irpar, sim]=libdyn_setup_schematic(schematic_fn, insizes, outsizes);



//
// Save the schematic to disk (possibly with other ones or other irpar elements)
//

parlist = new_irparam_set();

// pack simulations into irpar container with id = 901
parlist = new_irparam_container(parlist, sim_container_irpar, 901);

// irparam set is complete convert to vectors
par = combine_irparam(parlist);

// save vectors to a file
save_irparam(par, SchematicName+'.ipar', SchematicName+'.rpar');

// clear
par.ipar = [];
par.rpar = [];



// 
// // optionally execute 
// // Hereby, ORTD.ortd_executable must be the filename of the "libdyn_generic_exec_scilab"-executable
// messages=unix_g(ORTD.ortd_executable+ ' -s '+SchematicName+' -i 901 -l 100'); // simulate 100 samples
// 
// 
// // load results
// A = fscanfMat('result.dat');
// 
// // and show them
// scf(1);clf;
// plot(A(:,1), 'k');
// 
