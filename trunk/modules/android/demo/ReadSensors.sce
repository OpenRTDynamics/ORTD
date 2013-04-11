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
// Example for varying sampling times
//
// This is an example for running simulations in a thread with non-constant
// sampling time. Using the "ld_synctimer" the interval time to the next
// simulation step can be specified by an input sigal to this block.
// Currently the "ld_synctimer" only works for nested simulations.
//
// Run with 
//
// $ sh ReadSensors.sh
//


thispath = get_absolute_file_path('ReadSensors.sce');
cd(thispath);


z = poly(0,'z');

T_a = 0.1;

//
// Set up simulation schematic
//


function [sim, out] = ld_AndroidSensors(sim, events, in, rate) // PARSEDOCU_BLOCK
// %PURPOSE: Read out Android Sensors and synchronise the simulation to them
//
// Special: SYNC_BLOCK (use only one block of this type in an asynchronous running sub-simulation)
//
// out - vector of size 10 containing the sensor values
// in - when in becomes one, the synchronisation loop is interrupted
// 


 btype = 15500 + 0; //
 [sim,blk] = libdyn_new_block(sim, events, btype, ipar=[ rate  ], rpar=[  ], ...
                  insizes=[1], outsizes=[10], ...
                  intypes=[ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT]  );

 [sim,blk] = libdyn_conn_equation(sim, blk, list(in) );
 [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction


function [sim, outlist] = run_thread_fn(sim, inlist)
  // This will run once in a thread


  defaultevents = 0;
    
  inputv = inlist(1);
//  [sim] = ld_printf(sim, defaultevents, inputv, "inputv = ", 10);

  //
  // Define non-constant sample times
  //

//  [sim, Tpause] = ld_const(sim, events, 1);
//  [sim,Tpause] = ld_play_simple(sim, events, r=[0.51, 0.52, 0.53, 0.54, 0.55, -1]);
  [sim,Stop] = ld_play_simple(sim, events, r=[ zeros(100, 1); 1  ]);

  // Set the time interval between the simulation steps
  [sim, out] = ld_AndroidSensors(sim, events, in=Stop, rate=100);
  [sim] = ld_printf(sim, events, out, "Sensors: ", 10);



  [sim, result] = ld_constvec(sim, defaultevents, 1:10);

  // output of schematic
  outlist = list(result);
endfunction




// This is the main top level schematic
function [sim, outlist] = schematic_fn(sim, inlist)
  [sim,u2] = ld_const(sim, defaultevents, 2);
  
//  [sim,switch] = ld_const(sim, defaultevents, 0.76);
  [sim,reset] = ld_const(sim, defaultevents, 1.23);
  
  events = defaultevents;

       // input should be a signal vector of size 10
        [sim, input] = ld_constvec(sim, events, vec=1:10)

        [sim, zero] = ld_const(sim, events, 0);

//        [sim, startcalc] = ld_initimpuls(sim, events); // triggers your computation only once
        [sim, startcalc] = ld_const(sim, events, 1); // triggers your computation during each time step

        // a nested simulation that runns asynchronously (in a thread) to the main simulation
        [sim, outlist, computation_finished] = ld_simnest(sim, events, ...
                              inlist=list(input), ...
                              insizes=[10], outsizes=[10], ...
                              intypes=[1], outtypes=[1], ...
                              fn_list=list(run_thread_fn), ...
                              dfeed=1, asynchron_simsteps=2, ...
                              switch_signal=zero, reset_trigger_signal=startcalc         );

         output1 = outlist(1);
         // computation_finished is one, when finished else zero

       [sim] = ld_printf(sim, events, in=computation_finished, str="computation_finished", insize=1);
       


  // dummy output  
  [sim,x] = ld_const(sim, defaultevents, 1.123525);
  
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

// irparam set is complete, convert to vectors
par = combine_irparam(parlist);

// save vectors to a file
save_irparam(par, 'ReadSensors.ipar', 'ReadSensors.rpar');

// clear
par.ipar = [];
par.rpar = [];





