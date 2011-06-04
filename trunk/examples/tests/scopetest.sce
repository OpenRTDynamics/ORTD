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
// execute within scilab and run "libdyn_generic_exec -s scopetest -i 901 -l 100" within 
// the directory of this file
//
// It will write output data to *dat files
//


thispath = get_absolute_file_path('scopetest.sce');
cd(thispath);


z = poly(0,'z');


//
// Set up simulation schematic
//


// This is the main top level schematic
function [sim, outlist] = schematic_fn(sim, inlist)
  // sum up two inputs
  [sim,out] = ld_sum(sim, defaultevents, list(inlist(1), inlist(2)), 1, 1);
  
  sim=ld_printf(sim, defaultevents, out, "testscope", 1);
  sim=ld_scope(sim, defaultevents, out, "testscope", 1);
  
  // save result to file
  [sim, save0] = libdyn_dumptoiofile(sim, defaultevents, "result.dat", list(out));
  
  
  // output of schematic
  outlist = list(out);
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
save_irparam(par, 'scopetest.ipar', 'scopetest.rpar');

// clear
par.ipar = [];
par.rpar = [];




// optionally execute
//unix('libdyn_generic_exec -s scopetest -i 901 -l 100');


// load results
//A = fscanfMat('result.dat');

//scf(1);clf;
//plot(A(:,1), 'k');

