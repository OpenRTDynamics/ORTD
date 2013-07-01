//
//    Copyright (C) 2010, 2011, 2012, 2013  Christian Klauer
//
//    This file is part of OpenRTDynamics, the Real Time Dynamics Framework
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
// Execute within scilab. The shell command "ortd -s oscillator -i 901 -l 1000"
// will be executed to simulate this example. It will write output data to *dat files.
// Finally, the results are plotted
//
//


thispath = get_absolute_file_path('oscillator.sce');
cd(thispath);


SchematicName = 'oscillator'; // must be the filename without .sce
thispath = get_absolute_file_path(SchematicName+'.sce');
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
        [sim,a] = ld_add(sim, ev, list(u, x_feedback), [1, -1]);
        [sim,v] = ld_ztf(sim, ev, a, 1/(z-1) * T_a ); // Integrator approximation
        [sim,x] = ld_ztf(sim, ev, v, 1/(z-1) * T_a ); // Integrator approximation  
    
        // feedback gain
        [sim,x_gain] = ld_gain(sim, ev, x, 0.6);
    
    // close loop x_gain = x_feedback
    [sim] = libdyn_close_loop(sim, x_gain, x_feedback);
endfunction

function [sim, x,v] = damped_oscillator(sim, u)
    // create feedback signals
    [sim,x_feedback] = libdyn_new_feedback(sim);

        [sim,v_feedback] = libdyn_new_feedback(sim);

            // use this as a normal signal
            [sim,a] = ld_add(sim, ev, list(u, x_feedback), [1, -1]);
            [sim,a] = ld_add(sim, ev, list(a, v_feedback), [1, -1]);
    
            [sim,v] = ld_ztf(sim, ev, a, 1/(z-1) * T_a ); // Integrator approximation
    
            // feedback gain
            [sim,v_gain] = ld_gain(sim, ev, v, 0.25);
    
            // close loop v_gain = v_feedback
        [sim] = libdyn_close_loop(sim, v_gain, v_feedback);
    
    
        [sim,x] = ld_ztf(sim, ev, v, 1/(z-1) * T_a ); // Integrator approximation  
    
        // feedback gain
        [sim,x_gain] = ld_gain(sim, ev, x, 0.6);
    
    // close loop x_gain = x_feedback
    [sim] = libdyn_close_loop(sim, x_gain, x_feedback);
endfunction


// This is the main top level schematic
function [sim, outlist] = schematic_fn(sim, inlist)
  [sim,u] = ld_const(sim, ev, 1);
  
  // example of conditional schmeatic generation
  damped = 1; // please choose 1 or 0
  
  if (damped == 1) then
    [sim, x,y] = damped_oscillator(sim, u);
  else
    [sim, x,y] = oscillator(sim, u);  
  end
  
  // print the current value of x
//   [sim] = ld_printf(sim, ev, x, "x = ", 1);

  // print a barchart
  [sim, x__] = ld_gain(sim, ev, x, 15);
  [sim] = ld_printfbar(sim, ev, in=x__, str="x ");
  
  // save result to file
//   [sim, save0] = ld_dumptoiofile(sim, ev, "result.dat", x);
  
  // output of schematic
  outlist = list(x);
endfunction



  
//
// Set-up
//

// default events
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
save_irparam(par, 'oscillator.ipar', 'oscillator.rpar');

// clear
par.ipar = [];
par.rpar = [];




// run the executable for 1000 simulation steps
messages=unix_g(ORTD.ortd_executable+ ' -s '+SchematicName+' -i 901 -l 1000');


// load results
A = fscanfMat('result.dat');

scf(1);clf;
plot(A(:,1), 'k');

