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


thispath = get_absolute_file_path('testbed.sce');
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
    [sim,a] = ld_sum(sim, defaultevents, list(u, x_feedback), 1, -1);
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
    [sim,a] = ld_sum(sim, defaultevents, list(u, x_feedback), 1, -1);
    [sim,a] = ld_sum(sim, defaultevents, list(a, v_feedback), 1, -1);
    
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




//function [sim,y] = ld_alternate( sim, ev, start_with_zero )
//    //
//    // generate an alternating sequence     
//    //
//    // [0, 1, 0, 1, 0, ... ], if start_with_zero == %T
//    // [1, 0, 1, 0, 1, ... ], if start_with_zero == %F
//    //
//    
//  [sim,one] = ld_const(sim, ev, 1);
//
//  [sim, fb] = libdyn_new_feedback();
//  
//  [sim, su ] = ld_add(sim, ev, list(fb, one), [-1,1] );
//  [sim, del] = ld_ztf(sim, ev, su, 1/z);
//  
//  [sim] = libdyn_close_loop(sim, del, fb);
//  
//  y = del; 
//  
//  if (start_with_zero == %F) then
//    [sim, y] = ld_not(sim, ev, y);
//  end
//    
//endfunction
//


function [sim, y] = mundus_leftrighth_help2c(sim, ev, u, init, min__, max__, Ta) 
// Implements a time discrete integrator with saturation of the output between min__ and max__
// 
// u * - input
// y * - output
// init * - should be a const signal
// 

    z = poly(0, 'z');

    //
    [sim] = ld_printf(sim, ev, init, termcode.red + "init  " + termcode.reset, 1);

    // extract only the first sample of init
    [sim, initimp] = ld_initimpuls(sim, ev);
    [sim, notinitimp ] = ld_not(sim, ev, initimp);
    [sim, init] = ld_cond_overwrite(sim, ev, in=init, condition=notinitimp, setto=0);

//      a = 0.99;
//      [sim, init] = ld_ztf(sim, ev, init, (1-a)/(z-a) );




    [sim, u__] = ld_gain(sim, ev, u, Ta);
    
    [sim,z_fb] = libdyn_new_feedback(sim);
    
    [sim, i1] = ld_sum(sim, ev, list(u__, z_fb), 1, 1);
//     [sim, i2] = ld_ztf(sim, ev, i1, 1/z);
    [sim, i2] = ld_delay(sim, ev, i1, 1);
    [sim, i3] = ld_sum(sim, ev, list(i2, init), 1, 1); // add initial value
    [sim, y] = ld_sat(sim, ev, i3, min__, max__);
    
    [sim] = libdyn_close_loop(sim, y, z_fb);


    [sim] = ld_printf(sim, ev, y, termcode.red + "output  " + termcode.reset, 1);


   // save 
   [sim, save_] = ld_mux(sim, ev, 8, list( i1, i2, i3, u, initimp, notinitimp, init, y ) );
   [sim] = ld_savefile(sim, ev, fname="debug_mundus_leftrighth_help2c.dat", source=save_, vlen=8);


    // delay the output
    a = 0.94;
    [sim, y] = ld_ztf(sim, ev, y, (1-a)/(z-a) );


endfunction

// This is the main top level schematic
function [sim, outlist] = schematic_fn(sim, inlist)
  

 [sim, u] = ld_play_simple(sim, defaultevents, [1,0.5,1,2,3,4,0,1,0]);
 [sim, u] = ld_delay(sim, defaultevents, u, 1);

ev = defaultevents;

//  [sim,u] = ld_alternate( sim, ev, start_with_zero=%F );



  [sim, c1] = ld_const(sim, ev, 0.01);
  [sim, c2] = ld_const(sim, ev, 0.1);


  [sim, y] = mundus_leftrighth_help2c(sim, ev, u=c1, init=c2, min__=0, max__=0.3, Ta=1);
  
  [sim, save0] = ld_dumptoiofile(sim, defaultevents, "result2.dat", y);

  
  [sim] = ld_printf(sim, defaultevents, u, "mytest = ", 1);

  x = y;
  
  
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
save_irparam(par, 'testbed.ipar', 'testbed.rpar');

// clear
par.ipar = [];
par.rpar = [];




// optionally execute
messages=unix_g(ORTD.ortd_executable+ ' -s testbed -i 901 -l 20');


// load results
A = fscanfMat('result.dat');

scf(1);clf;
plot(A(:,1), 'k');

