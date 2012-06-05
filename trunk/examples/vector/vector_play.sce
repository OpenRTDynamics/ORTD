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


thispath = get_absolute_file_path('vector_play.sce');
cd(thispath);


z = poly(0,'z');


//
// Set up simulation schematic
//



//function [sim, outvec, Nvecplay] = ld_vector_play(sim, ev, A, special)
//  [Nvecplay,Nsamples] = size(A); // m is the number of samples in time
//
//  data = A(:); Ndata = length(data);
//
//  // create a new vector
//  [sim,vector] = ld_constvec(sim, ev, data );
//  
//  // vector extract test
////  [sim,index] = ld_const(sim, defaultevents, 2);  // max is Nsamples
//
//  if special == "repeate" then
//    [sim,one] = ld_const(sim, ev, 1);
//    [sim, index] = ld_modcounter(sim, ev, in=one, initial_count=0, mod=Nsamples);
//  else
//    error("wrong special string. Should be one of ""repeate"", ...");
//  end
//
////  [sim] = ld_printf(sim, ev, index, "index = ", 1);
//
//  [sim, start_at] = ld_gain(sim, ev, index, Nvecplay);
//  [sim, start_at] = ld_add_ofs(sim, ev, start_at, 1);
//
//  [sim,outvec] = ld_vector_extract(sim, ev, in=vector, from=start_at, window_len=Nvecplay, vecsize=Ndata );
//endfunction



// This is the main top level schematic
function [sim, outlist] = schematic_fn(sim, inlist)
   u1 = inlist(1); // Simulation input #1
   u2 = inlist(2); // Simulation input #2
   
  A = [ 1,       2,    3,   4;
        1.1,   2.2,  3.3,  4.4;
        1.11, 2.22, 3.33 , 4.44 ];


  [sim, outvec, Nvecplay] = ld_vector_play(sim, defaultevents, A, special="repeate");

  [sim] = ld_printf(sim, defaultevents, outvec, "vector = ", Nvecplay);





  
  
//  // save the vector to a file 
//  [sim]=ld_savefile(sim, defaultevents, fname="saved_vector.dat", source=vector, vlen=8);
//  
  // output of schematic
  [sim,out] = ld_const(sim, defaultevents, 0);
  outlist = list(out); // Simulation output #1
endfunction


  
//
// Set-up
//

// defile events
defaultevents = [0]; // main event
ev = [0]; // main event

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
messages = unix_g(ORTD.ortd_executable+' -s simple_demo -i 901 -l 4');

disp(messages);

//// load results
//A = fscanfMat('result.dat');
//
//scf(1);clf;
//plot(A(:,1), 'k');
//
