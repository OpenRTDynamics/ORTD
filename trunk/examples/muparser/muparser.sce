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
// An oscillator example for using the Scilab interface to libdyn
// NOTE: The "ld_toolbox" is needed to run
//
// Execute within scilab. The shell command "libdyn_generic_exec -s oscillator -i 901 -l 1000"
// will be executed to simulate this example. It will write output data to *dat files.
// Finally, the results are plotted
//
//


thispath = get_absolute_file_path('muparser.sce');
cd(thispath);


z = poly(0,'z');

T_a = 0.1;






// This is the main top level schematic
function [sim, outlist] = schematic_fn(sim, inlist)
  [sim,u] = ld_const(sim, defaultevents, 1);
  [sim,x] = ld_const(sim, defaultevents, 2);
  [sim,y] = ld_const(sim, defaultevents, 3);
  
  

  expr = 'sin(u1+1)*c2+u2';
  float_param=[3.1, 21];
  [sim,out] = ld_muparser(sim, defaultevents, list(u,x), expr, float_param);

  [sim] = ld_printf(sim, defaultevents, out, "muout = ", 1);

  
  
  parNames = ['par1', 'par2']; par = [ 0.2, 0.4 ];
  inNames = ['x', 'y' ]; in = list(x,y);
  expr = 'sin(x)*par1 + par2*y';
  [sim,mytest] = ld_muparser_subst(sim, defaultevents, in, expr, par, inNames, parNames);
  
  [sim] = ld_printf(sim, defaultevents, mytest, "mytest__ = ", 1);
  
  // save result to file
  [sim, save0] = ld_dumptoiofile(sim, defaultevents, "result.dat", mytest);
  
  // output of schematic
  outlist = list(x);
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
save_irparam(par, 'muparser.ipar', 'muparser.rpar');

// clear
par.ipar = [];
par.rpar = [];




// optionally execute
messages = unix_g(ORTD.ortd_executable+' -s muparser -i 901 -l 10');


// load results
A = fscanfMat('result.dat');

scf(1);clf;
plot(A(:,1), 'k');

