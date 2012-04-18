//
//    Copyright (C) 2010, 2011  Christian Klauer
//
//    This file is part of OpenRTDynamics, the Real Time Dynamic Toolbox
//
//    OpenRTDynamics is free software: you can redistribute it and/or modify
//    it under the terms of the GNU Lesser General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    OpenRTDynamics is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public License
//    along with OpenRTDynamics.  If not, see <http://www.gnu.org/licenses/>.
//





//
// Simple example for the UDP interface to ortd
//
// execute within scilab and run "libdyn_generic_exec -s udp_recv -i 901 -l 0" within 
// the directory of this file
//
// It will write output data to *dat files
//


thispath = get_absolute_file_path('startproc.sce');
cd(thispath);


z = poly(0,'z');


//
// Set up simulation schematic
//


// This is the main top level schematic
function [sim, outlist] = schematic_fn(sim, inlist)
   u1 = inlist(1); // Simulation input #1
   u2 = inlist(2); // Simulation input #2
   
   
  // sum up two inputs
  [sim,dummy] = ld_add(sim, defaultevents, list(u1, u2), [1, 1] );


  [sim, out] = ld_startproc(sim, defaultevents, exepath="./sub_process.sh", whentorun=0);

  // save result to file
  [sim] = ld_savefile(sim, defaultevents, fname="startproc.dat", source=out, vlen=1);
  
  // output of schematic
  outlist = list(dummy); // Simulation output #1
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
save_irparam(par, 'startproc.ipar', 'startproc.rpar');

// clear
par.ipar = [];
par.rpar = [];




// optionally execute
//messages=unix_g(ORTD.ortd_executable+ ' -s startproc -i 901 -l 100');

