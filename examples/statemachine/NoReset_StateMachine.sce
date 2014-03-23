//
//    Copyright (C) 2011, 2012, 2013  Christian Klauer
//
//    This file is part of OpenRTDynamics, the Real-Time Dynamics Framework
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
// After running this file, have a look into the variable "messages" to
// observed what happened.
//


SchematicName = 'NoReset_StateMachine'; // must be the filename without .sce
thispath = get_absolute_file_path(SchematicName+'.sce');
cd(thispath);

z = poly(0,'z');



//
// Set up simulation schematic
//




function [sim, count] = Counter(sim, Inc)
// 
// This implements a counter that will not be reset e.g. on state-machine
// state switches
// 

	  function [sim, outlist, userdata] = NoResetNestedSim(sim, inlist, userdata)
	    // This will run in a thread
	    defaultevents = 0;
	      
	    // input to the calculation
	    Inc = inlist(1);
	    [sim] = ld_printf(sim, 0, Inc, "Inc counter by", 1);


// 	    [sim, result] = ld_constvec(sim, 0, 2); // dummmy calculation

            [sim, zero] = ld_const(sim, 0, 0);
            [sim, result] = ld_counter(sim, 0, count=Inc, reset=zero, resetto=zero, initial=0);

	    // output of schematic
	    outlist = list(result);
	  endfunction


	  [sim, outlist ] = ld_NoResetNest(sim, 0, ...
				inlist=list(Inc), ...
				insizes=[1], outsizes=[1], ...
				intypes=[ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT], ...
				nested_fn = NoResetNestedSim, ...
				ResetLevel=-1, SimnestName="Counter_NoResetNestedSim", ...
				userdata=list() );

           count = outlist(1);
endfunction


function [sim, outlist, active_state, x_global_kp1, userdata] = state_mainfn(sim, inlist, x_global, state, statename, userdata)
  // This function is called multiple times -- once to define each state.
  // At runtime, all states will become different nested simulations of 
  // which only one is active a a time. Switching
  // between them represents state changing, thus each simulation 
  // represents a certain state.
  
  printf("Defining state %s (#%d) ...\n", statename, state);
  
  // define names for the first event in the simulation
  events = 0;
  
  // print out some state information
  [sim] = ld_printf(sim, events, in=inlist(1), str="state"+string(state)+": indata(1)", insize=1);
  [sim] = ld_printf(sim, events, in=inlist(2), str="state"+string(state)+": indata(2)", insize=2);
  [sim] = ld_printf(sim, events, in=x_global, str="state"+string(state)+": x_global", insize=4);

  // demultiplex x_global that is a state variable shared among the different states
  [sim, x_global] = ld_demux(sim, events, vecsize=4, invec=x_global);

  // sample data for the output
  [sim, outdata1] = ld_constvec(sim, events, vec=[1200]);

  // The signals "active_state" is used to indicate state switching: A value > 0 means the 
  // the state enumed by "active_state" shall be activated in the next time step.
  // A value less or equal to zero causes the statemachine to stay in its currently active
  // state

  select state
    case 1 // state 1

      // This counter will not be reset by this state machine on a state switch
      [sim, one] = ld_const(sim, 0, 1);
      [sim, count] = Counter(sim, one); 
      [sim] = ld_printf(sim, 0, count, "Total Counter: ", 1);

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

  // some dummy input to the state machine
  [sim,data1] = ld_const(sim, defaultevents, 1.234);   
  [sim, data2] = ld_constvec(sim, defaultevents, vec=[12,13]);

  // set-up three states represented by three nested simulations
  [sim, outlist, x_global, active_state,userdata] = ld_statemachine(sim, ev=defaultevents, ...
      inlist=list(data1, data2), ..
      insizes=[1,2], outsizes=[1], ... 
      intypes=[ORTD.DATATYPE_FLOAT,ORTD.DATATYPE_FLOAT  ], outtypes=[ORTD.DATATYPE_FLOAT], ...
      nested_fn=state_mainfn, Nstates=3, state_names_list=list("state1", "state2", "state3"), ...
      inittial_state=3, x0_global=[1,0,2,0], userdata=list("some", "data")  );

  // signal telling the currently active simulation / state
  switch_state = active_state;
  
  // the list of signals "outlist" contains the output of the currently active 
  // nested simulation respectively
  // Grab each output signal via outlist(1), outlist(2), ...



      
  // output of schematic
  outlist = list(); 
endfunction


  







//
// Set-up
//

// defile events
defaultevents = [0]; // main event

// set-up schematic by calling the user defined function "schematic_fn"
insizes = []; outsizes=[];
[sim_container_irpar, sim]=libdyn_setup_schematic(schematic_fn, insizes, outsizes);



//
// Save the schematic to disk (possibly with other ones or other irpar elements)
//

parlist = new_irparam_set();

// pack simulations into irpar container with id = 901
parlist = new_irparam_container(parlist, sim_container_irpar, 901);

// irparam set is complete convert
par = combine_irparam(parlist);

// save vectors to a file
save_irparam(par, SchematicName+'.ipar', SchematicName+'.rpar');

// clear
par.ipar = [];
par.rpar = [];




// optionally execute
messages=unix_g(ORTD.ortd_executable+ ' -s '+SchematicName+' -i 901 -l 100 --master_tcpport 10000');

//
//// load results
//A = fscanfMat('result.dat');
//
//scf(1);clf;
//plot(A(:,1), 'k');

