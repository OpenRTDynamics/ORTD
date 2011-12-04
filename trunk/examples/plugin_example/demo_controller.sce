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
// Simple example for using scilab interface to libdyn
//
// execute within scilab and run "libdyn_generic_exec -s simple_demo -i 901 -l 100" within 
// the directory of this file
//
// It will write output data to *dat files
//


thispath = get_absolute_file_path('demo_controller.sce');
cd(thispath);


z = poly(0,'z');


function [sim,out] = sensor_actor_block(sim, events, in)
// 

  btype = 100001 + 0;
  [sim,blk] = libdyn_new_block(sim, events, btype, [  ], [  ], ...
                   insizes=[1], outsizes=[1], ...
                   intypes=[ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(in) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction



//
// Set up simulation schematic
//


// This is the main top level schematic
function [sim, outlist] = schematic_fn(sim, inlist)
      
  [sim, actuator] = ld_const(sim, defaultevents, 10);

  [sim,measure] = sensor_actor_block(sim, defaultevents, in=actuator);  

  // save result to file
  [sim, save0] = ld_dumptoiofile(sim, defaultevents, "result.dat", measure);
  
  // output of schematic (dummy)
  outlist = list(measure); 
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
save_irparam(par, 'demo_controller.ipar', 'demo_controller.rpar');

// clear
par.ipar = [];
par.rpar = [];

// build the C-Code for the plugin
unix_g("./build.sh");


// optionally execute
//messages=unix_g(ORTD.ortd_executable+ ' -s demo_controller -i 901 -l 100');


// load results
A = fscanfMat('result.dat');

scf(1);clf;
plot(A(:,1), 'k');

