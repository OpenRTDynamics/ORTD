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


thispath = get_absolute_file_path('nested.sce');
cd(thispath);


z = poly(0,'z');

T_a = 0.1;

//
// Set up simulation schematic
//

function [sim, x,v] = oscillator(sim, u)
    // create a feedback signal
    [sim,x_feedback] = libdyn_new_feedback(sim);

    // use this as a normal signal
    [sim,a] = ld_add(sim, sim.ev.defev, list(u, x_feedback), [1, -1]);
    [sim,v] = ld_ztf(sim, sim.ev.defev, a, 1/(z-1) * T_a ); // Integrator approximation
    [sim,x] = ld_ztf(sim, sim.ev.defev, v, 1/(z-1) * T_a ); // Integrator approximation  
    
    // feedback gain
    [sim,x_gain] = ld_gain(sim, sim.ev.defev, x, 0.6);
    
    // close loop x_gain = x_feedback
    [sim] = libdyn_close_loop(sim, x_gain, x_feedback);

//pause;
    
//    [sim] = ld_printf(sim, sim.ev.defev, x_gain, "fb = ", 1);
//    [sim] = ld_printf(sim, sim.ev.defev, a, "a = ", 1);
endfunction

function [sim, x,v] = damped_oscillator(sim, u)
    // create a feedback signal
    [sim,x_feedback] = libdyn_new_feedback(sim);
    [sim,v_feedback] = libdyn_new_feedback(sim);

    // use this as a normal signal
    [sim,a] = ld_add(sim, sim.ev.defev, list(u, x_feedback), [1, -1]);
    [sim,a] = ld_add(sim, sim.ev.defev, list(a, v_feedback), [1, -1]);
    
    [sim,v] = ld_ztf(sim, sim.ev.defev, a, 1/(z-1) * T_a ); // Integrator approximation
    
    // feedback gain
    [sim,v_gain] = ld_gain(sim, sim.ev.defev, v, 0.1);
    
    // close loop v_gain = v_feedback
    [sim] = libdyn_close_loop(sim, v_gain, v_feedback);
    
    
    [sim,x] = ld_ztf(sim, sim.ev.defev, v, 1/(z-1) * T_a ); // Integrator approximation  
    
    // feedback gain
    [sim,x_gain] = ld_gain(sim, sim.ev.defev, x, 0.6);
    
    // close loop x_gain = x_feedback
    [sim] = libdyn_close_loop(sim, x_gain, x_feedback);

//pause;
    
//    [sim] = ld_printf(sim, sim.ev.defev, x_gain, "fb = ", 1);
//    [sim] = ld_printf(sim, sim.ev.defev, a, "a = ", 1);
endfunction


function [sim, outlist] = nested1_schematic_fn(sim, inlist)
  sim.ev.defev = 0; // define the events
  u = inlist(1); // the inputs to the nested simulation

  [sim] = ld_printf(sim, sim.ev.defev, u, "nested1: u = ", 1);

//
//  // WARNING: THE FOLLOWING DOESN'T WORK, because you cannot 
//  // connect sim inputs directly to sim outputs
//  x = u;
//  y = u;
// 
// use [sim, x] = ld_gain(sim, sim.ev.defev, u, 1);
//
// as a work around
//


  // just put out something
  [sim,y] = ld_gain(sim, sim.ev.defev, u, 1);
  [sim, x] = ld_play_simple(sim, sim.ev.defev, [ linspace(0,5,90) ]);
  
  [sim] = ld_printf(sim, sim.ev.defev, x, "nested1: x = ", 1);

  // output of schematic
  outlist = list(x,y);
endfunction

function [sim, outlist] = nested2_schematic_fn(sim, inlist)
  sim.ev.defev = 0; // define the events
  u = inlist(1); // the inputs to the nested simulation
  
  [sim] = ld_printf(sim, sim.ev.defev, u, "nested2: u = ", 1);
  
  // simulate some model
  [sim, x,y] = damped_oscillator(sim, u);  


  [sim] = ld_printf(sim, sim.ev.defev, x, "nested2: x = ", 1);


  // output of schematic
  outlist = list(x,y);
endfunction

// This is the main top level schematic
function [sim, outlist] = schematic_fn(sim, inlist)
  sim.ev.defev = 0;
    
  [sim,u1] = ld_const(sim, sim.ev.defev, 1.123525);
  [sim,u2] = ld_const(sim, sim.ev.defev, 2);
  
  
  // A signal that switches between simulations
  slice = ones(100,1);
  [sim, switch] = ld_play_simple(sim, sim.ev.defev, [ slice; slice*1;  slice; slice*0; slice; slice*0 ]);
  
  // generate a reset impuls for every switch between the nested simulations
  // Thus simulations are reset after they are deactivated when switching
  // (The computational functions inside are called with flag=COMPF_FLAG_RESETSTATES)
  [sim, reset] = ld_ztf(sim, sim.ev.defev, switch, (z-1)/z );
  [sim, reset] = ld_abs(sim, sim.ev.defev, reset);

  // The block, which contains multiple nested simulations
  [sim, outlist] = ld_simnest(sim, sim.ev.defev, ...
                              inlist=list(u1, u2), ...
                              insizes=[1, 1], outsizes=[1, 1], ...
                              intypes=[1, 1], outtypes=[1, 1], ...
                              fn_list=list(nested1_schematic_fn, nested2_schematic_fn), ...
                              dfeed=1, asynchron_simsteps=0, ...
                              switch_signal=switch, reset_trigger_signal=reset           );
                              
  // get the outputs
  x = outlist(1);
  y = outlist(2);
  
  [sim] = ld_printf(sim, sim.ev.defev, x, "x = ", 1);
  [sim] = ld_printf(sim, sim.ev.defev, y, "y = ", 1);
  
  // save result to file
  [sim, save0] = ld_dumptoiofile(sim, sim.ev.defev, "result.dat", x);
  
  // output of schematic
  outlist = list(x);
endfunction



  
//
// Set-up
//

// defile events
sim.ev.defev = [0]; // main event

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
messages = unix_g(ORTD.ortd_executable+' -s oscillator -i 901 -l 1000');


// load results
A = fscanfMat('result.dat');

scf(1);clf;
plot(A(:,1), 'k');

