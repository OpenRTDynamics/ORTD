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


thispath = get_absolute_file_path('oscillator.sce');
cd(thispath);


z = poly(0,'z');

T_a = 0.1;

//
// Set up simulation schematic
//

function [sim, x,v] = oscillator(u)
    // create a feedback signal
    [sim,x_feedback] = libdyn_new_feedback(sim);

    // use this as a normal signal
    [sim,a] = ld_add(sim, defaultevents, list(u, x_feedback), [1, -1]);
    [sim,v] = ld_ztf(sim, defaultevents, a, 1/(z-1) * T_a ); // Integrator approximation
    [sim,x] = ld_ztf(sim, defaultevents, v, 1/(z-1) * T_a ); // Integrator approximation  
    
    // feedback gain
    [sim,x_gain] = ld_gain(sim, defaultevents, x, 0.6);
    
    // close loop x_gain = x_feedback
    [sim] = libdyn_close_loop(sim, x_gain, x_feedback);

//pause;
    
//    [sim] = ld_printf(sim, defaultevents, x_gain, "fb = ", 1);
//    [sim] = ld_printf(sim, defaultevents, a, "a = ", 1);
endfunction

function [sim, x,v] = damped_oscillator(u)
    // create a feedback signal
    [sim,x_feedback] = libdyn_new_feedback(sim);
    [sim,v_feedback] = libdyn_new_feedback(sim);

    // use this as a normal signal
    [sim,a] = ld_add(sim, defaultevents, list(u, x_feedback), [1, -1]);
    [sim,a] = ld_add(sim, defaultevents, list(a, v_feedback), [1, -1]);
    
    [sim,v] = ld_ztf(sim, defaultevents, a, 1/(z-1) * T_a ); // Integrator approximation
    
    // feedback gain
    [sim,v_gain] = ld_gain(sim, defaultevents, v, 0.1);
    
    // close loop v_gain = v_feedback
    [sim] = libdyn_close_loop(sim, v_gain, v_feedback);
    
    
    [sim,x] = ld_ztf(sim, defaultevents, v, 1/(z-1) * T_a ); // Integrator approximation  
    
    // feedback gain
    [sim,x_gain] = ld_gain(sim, defaultevents, x, 0.6);
    
    // close loop x_gain = x_feedback
    [sim] = libdyn_close_loop(sim, x_gain, x_feedback);

//pause;
    
//    [sim] = ld_printf(sim, defaultevents, x_gain, "fb = ", 1);
//    [sim] = ld_printf(sim, defaultevents, a, "a = ", 1);
endfunction


// This is the main top level schematic
function [sim, outlist] = schematic_fn(sim, inlist)
  [sim,u] = ld_const(sim, defaultevents, 1);
  
  // example of conditional schmeatic generation
  damped = 1; // please choose 1 or 0
  
  if (damped == 1) then
    [sim, x,y] = damped_oscillator(u);
  else
    [sim, x,y] = oscillator(u);  
  end
  
  
  
  [sim] = ld_printf(sim, defaultevents, x, "x = ", 1);
  
  // save result to file
  [sim, save0] = ld_dumptoiofile(sim, defaultevents, "result.dat", x);
  
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
save_irparam(par, 'oscillator.ipar', 'oscillator.rpar');

// clear
par.ipar = [];
par.rpar = [];




// optionally execute
unix('libdyn_generic_exec -s oscillator -i 901 -l 1000');


// load results
A = fscanfMat('result.dat');

scf(1);clf;
plot(A(:,1), 'k');

