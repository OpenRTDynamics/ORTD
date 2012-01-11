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


thispath = get_absolute_file_path('scilab_demo.sce');
cd(thispath);


z = poly(0,'z');


//
// Set up simulation schematic
//


// This is the main top level schematic
function [sim, outlist] = schematic_fn(sim, inlist)
   u1 = inlist(1); // Simulation input #1
   
  [sim, u1] = ld_constvec(sim, defaultevents, [0.6,0.5,0,1,-1]);
  // 
//  [sim,out] = ld_scilab(sim, defaultevents, in=u1, invecsize=5, outvecsize=5, "", ...
//  "a=1;\n save(\'a.dat\', a);  \n scilab_interf.outvec1 = scilab_interf.invec1 * 2", "", "/usr/local/bin/scilab533");
  [sim,out] = ld_scilab(sim, defaultevents, in=u1, invecsize=5, outvecsize=5, "", ...
  "scilab_interf.outvec1 = scilab_interf.invec1 * 2", "", "/home/chr/scilab/scilab-5.3.3_64/bin/scilab");
  [sim] = ld_printf(sim, defaultevents, out, "resultvector = ", 5);
  
  // save result to file
//  [sim, save0] = ld_dumptoiofile(sim, defaultevents, "result.dat", out);
  
  // output of schematic
  [sim,out] = ld_const(sim, defaultevents, 0);
  outlist = list(out); // Simulation output #1
endfunction


  
//
// Set-up
//

// defile events
defaultevents = [0]; // main event

// set-up schematic by calling the user defined function "schematic_fn"
insizes = [1]; outsizes=[1];
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
save_irparam(par, 'scilab_demo.ipar', 'scilab_demo.rpar');

// clear
par.ipar = [];
par.rpar = [];




// optionally execute
messages=unix_g(ORTD.ortd_executable+ ' -s scilab_demo -i 901 -l 1');


// load results
//A = fscanfMat('result.dat');

//scf(1);clf;
//plot(A(:,1), 'k');

