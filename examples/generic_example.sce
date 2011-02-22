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
// execute within scilab and run "libdyn_generic_exec" within 
// the directory of this file
//
// It will write output data to *dat files
//


thispath = get_absolute_file_path('generic_example.sce');
cd(thispath);


T_as =1/50;
T_am = 1/1;

noplot = 0;
z = poly(0,'z');




u1 = 1:100;
u2 = 201:300;



//
// Set up simulation schematic
//

// A superblock

function [sim, out] = calc(sim, in1, in2)
  [sim,out] = ld_sum(sim, defaultevents, list(in1, in2), 1, 1);
endfunction

//
// Main
//

  sim = libdyn_new_simulation([], []);
  defaultevents = [0];

   // get input signals of container
  [sim,simulation_inputs] = libdyn_get_external_ins(sim);
  [sim,key0] = libdyn_new_oport_hint(sim, simulation_inputs, 0);
  [sim,key1] = libdyn_new_oport_hint(sim, simulation_inputs, 1);
  
  // save to a file
  [sim, save0] = libdyn_dumptoiofile(sim, defaultevents, "key.dat", list(key0));


  
  // sum up
  [sim,wave] = ld_sum(sim, defaultevents, list(key0, key1), 1,1);
  
  [sim, out] = calc(sim, key0, key1)

  // save to a file
  [sim, save0] = libdyn_dumptoiofile(sim, defaultevents, "result.dat", list(out));
  
  // connect output
  sim = libdyn_connect_outport(sim, wave, 0);
  
  
  
  // Collect and encode as irparem-set
  sim = libdyn_build_cl(sim); 
  sim_container_irpar = combine_irparam(sim.parlist);







//
//
//


parlist = new_irparam_set();

parlist = new_irparam_elemet_rvec(parlist, u1,  920);
parlist = new_irparam_elemet_rvec(parlist, u2,  921);

// pack simulations into irpar container
parlist = new_irparam_elemet_box(parlist, sim_container_irpar.ipar, sim_container_irpar.rpar, 901);


par = combine_irparam(parlist);
save_irparam(par, 'generic.ipar', 'generic.rpar');
par.ipar = [];
par.rpar = [];


//unix('./dynlib_test4');



//A = fscanfMat('ldtest_out.dat');

//scf(1);clf;
//plot(A(:,1), 'k');
//plot(A(:,2), 'g');
//plot(A(:,3), 'r');

