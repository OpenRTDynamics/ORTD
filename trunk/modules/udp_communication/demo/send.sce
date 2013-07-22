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

  // Open an UDP-Port
  [sim] = ld_UDPSocket_shObj(sim, ev, ObjectIdentifyer="aSocket", Visibility=0, hostname="127.0.0.1", UDPPort=10000);

  // generate two signals   
  [sim, u1] = ld_play_simple(sim, ev, r=linspace(0,1,100) );
  [sim, u2] = ld_play_simple(sim, ev, r=sin( linspace(0,%pi*6,100) ) );

  out = u1;

  // print data
  [sim] = ld_printf(sim, ev, out, "output = ", 1);
  
  // send
  [sim, out] = ld_UDPSocket_Send(sim, ev, ObjectIdentifyer="aSocket", in=out, insize=1, intype=ORTD.DATATYPE_FLOAT);

//   // save the signal us
//   [sim] = ld_savefile(sim, ev, fname="result.dat", source=out, vlen=1);
  
  // output of schematic
  [sim, out] = ld_const(sim, ev, 0);
  outlist = list(out); // Simulation output #1
endfunction


  
//
// Set-up
//

// defile events
defaultevents = [0]; // main event

// set-up schematic by calling the user defined function "schematic_fn"
insizes = [1,1]; outsizes=[1];
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
