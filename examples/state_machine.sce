//
//    Copyright (C) 2011, 2012  Christian Klauer
//
//    This file is part of OpenRTDynamics, the Real Time Dynamics Toolbox
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
// This is an example for setting up a statemachine / a nested simulation
// using the nested module
//
//


thispath = get_absolute_file_path('state_machine.sce');
cd(thispath);


z = poly(0,'z');



//
// Set up simulation schematic
//

function [sim, outlist, active_state, x_global_kp1, userdata] = state_mainfn(sim, inlist, x_global, state, statename, userdata)
  // This function is called multiple times -- once for each state.
  // At runtime, these are three different nested simulations. Switching
  // between them represents state changing, thus each simulation 
  // represents a certain state.
  
  printf("defining state %s (#%d) ... userdata(1)=%s\n", statename, state, userdata(1) );
  
  // define names for the first event in the simulation
  events = 0;
  
  // print out some state information
  [sim] = ld_printf(sim, events, in=inlist(1), str="state"+string(state)+": indata(1)", insize=1);
  [sim] = ld_printf(sim, events, in=inlist(2), str="state"+string(state)+": indata(2)", insize=2);

  [sim] = ld_printf(sim, events, in=x_global, str="state"+string(state)+": x_global", insize=4);

  // demultiplex x_global
  [sim, x_global] = ld_demux(sim, events, vecsize=4, invec=x_global);

  // sample data fot output
  [sim, outdata1] = ld_constvec(sim, events, vec=[1200]);

  // The signals "active_state" is used to indicate state switching: A value > 0 means the 
  // the state enumed by "active_state" shall be activated in the next time step.
  // A value less or equal to zero causes the statemachine to stay in its currently active
  // state

  select state
    case 1 // state 1
      // wait 10 simulation steps and then switch to state 2
      [sim, active_state] = ld_steps(sim, events, activation_simsteps=[10], values=[-1,2]); // -1 means stay within this state. 2 means go to state # 2
      [sim, x_global(1)] = ld_add_ofs(sim, events, x_global(1), 1); // increase counter 1 by 1
    case 2 // state 2
      // wait 10 simulation steps and then switch to state 3
      [sim, active_state] = ld_steps(sim, events, activation_simsteps=[10], values=[-1,3]);
      [sim, x_global(2)] = ld_add_ofs(sim, events, x_global(2), 1); // increase counter 2 by 1
    case 3 // state 3
      // wait 10 simulation steps and then switch to state 1
      [sim, active_state] = ld_steps(sim, events, activation_simsteps=[10], values=[-1,1]);
      [sim, x_global(3)] = ld_add_ofs(sim, events, x_global(3), 1); // increase counter 3 by 1
  end

  // multiplex the new global states
  [sim, x_global_kp1] = ld_mux(sim, events, vecsize=4, inlist=x_global);
  
  // the user defined output signals of this nested simulation
  outlist = list(outdata1);
endfunction


// This is the main top level schematic
function [sim, outlist] = schematic_fn(sim, inlist)
   u1 = inlist(1); // Simulation input #1
   u2 = inlist(2); // Simulation input #2
 
  // some dummy input
  [sim,data1] = ld_const(sim, defaultevents, 1.234);   
  [sim, data2] = ld_constvec(sim, defaultevents, vec=[12,13]);

  // set-up three states represented by three nested simulations
  [sim, outlist, x_global, active_state,userdata] = ld_statemachine(sim, ev=defaultevents, ...
      inlist=list(data1, data2), ..
      insizes=[1,2], outsizes=[1], ... 
      intypes=[ORTD.DATATYPE_FLOAT,ORTD.DATATYPE_FLOAT  ], outtypes=[ORTD.DATATYPE_FLOAT], ...
      nested_fn=state_mainfn, Nstates=3, state_names_list=list("state1", "state2", "state3"), ...
      inittial_state=3, x0_global=[1,0,2,0], userdata=list("hallo")  );

  // signal telling the currently active simulation / state
  switch_state = active_state;
  
  // the list of signals "outlist" contains the output of the currently active 
  // nested simulation respectively
  // Grab each output signal via outlist(1), outlist(2), ...



  // just some dummy proceedings
  // sum up two inputs
  [sim,out] = ld_add(sim, defaultevents, list(u1, u2), [1, 1] );
  
  // save result to file
  [sim, save0] = ld_dumptoiofile(sim, defaultevents, "result.dat", out);
      
  // output of schematic
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
save_irparam(par, 'state_machine.ipar', 'state_machine.rpar');

// clear
par.ipar = [];
par.rpar = [];




// optionally execute
messages=unix_g(ORTD.ortd_executable+ ' -s state_machine -i 901 -l 100');
disp(messages);
//
//
//// load results
//A = fscanfMat('result.dat');
//
//scf(1);clf;
//plot(A(:,1), 'k');
//
