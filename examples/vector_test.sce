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
// Simple example for using vector handling blocks
//
//
// Results will are stored in the variable 'messages'
//


thispath = get_absolute_file_path('vector_test.sce');
cd(thispath);


z = poly(0,'z');


//
// Set up simulation schematic
//


// This is the main top level schematic
function [sim, outlist] = schematic_fn(sim, inlist)
   u1 = inlist(1); // Simulation input #1
   u2 = inlist(2); // Simulation input #2
   
  // create a new vector
  [sim,vector] = ld_constvec(sim, defaultevents, [1,2,4,4,6,-6,-1,6] );
//  [sim,vector] = ld_constvec(sim, defaultevents, [  0.1, -0.2, 2.2, 3.5, -5.4, 3.3, 0, -10 ] );
  
  //Vector diff test
  [sim,diffvec] = ld_vector_diff(sim, defaultevents, in=vector, vecsize=8 );
  [sim] = ld_printf(sim, defaultevents, diffvec, "diffvec = ", 7);
  
  // vector find thr test
  [sim,threshold] = ld_const(sim, defaultevents, 2.0);
  [sim,index] = ld_vector_findthr(sim, defaultevents, in=vector, thr=threshold, greater=1, vecsize=8);
  [sim] = ld_printf(sim, defaultevents, index, "index = ", 1);
  
  // abs test
  [sim,out] = ld_vector_abs(sim, defaultevents, in=vector, vecsize=8);
  [sim] = ld_printf(sim, defaultevents, out, "absvector = ", 8);
  
  // vector gain test
  [sim,out] = ld_vector_gain(sim, defaultevents, in=vector, gain=2, vecsize=8);
  [sim] = ld_printf(sim, defaultevents, out, "gainedvector = ", 8);
  
  // vector extract test
  [sim,start_at] = ld_const(sim, defaultevents, 3);  
  [sim,out] = ld_vector_extract(sim, defaultevents, in=vector, from=start_at, window_len=3, vecsize=8);
  [sim] = ld_printf(sim, defaultevents, out, "extractedvector = ", 3);
  
  // vector find max / min
  [sim,index] = ld_vector_minmax(sim, defaultevents, in=vector, findmax=1, vecsize=8);
  [sim] = ld_printf(sim, defaultevents, index, "max found at index = ", 1);

  // vector add scalar
  [sim,toadd] = ld_const(sim, defaultevents, 3);  
  [sim,out] = ld_vector_addscalar(sim, defaultevents, in=vector, add=toadd, vecsize=8);
  [sim] = ld_printf(sim, defaultevents, out, "vector + toadd = ", 8);
  
  // sum up vector
  [sim,out] = ld_vector_sum(sim, defaultevents, in=vector, vecsize=8);
  [sim] = ld_printf(sim, defaultevents, out, "sum(vector) = ", 1);
  
  
  
  // save the vector to a file 
  [sim]=ld_savefile(sim, defaultevents, fname="saved_vector.dat", source=vector, vlen=8);
  
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
save_irparam(par, 'simple_demo.ipar', 'simple_demo.rpar');

// clear
par.ipar = [];
par.rpar = [];




// execute one simulation step (-l 1)
messages = unix_g('libdyn_generic_exec -s simple_demo -i 901 -l 1');

disp(messages);

//// load results
//A = fscanfMat('result.dat');
//
//scf(1);clf;
//plot(A(:,1), 'k');
//
