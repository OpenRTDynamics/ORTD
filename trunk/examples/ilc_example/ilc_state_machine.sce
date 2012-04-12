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


thispath = get_absolute_file_path('ilc_state_machine.sce');
cd(thispath);


z = poly(0,'z');


function [sim, outlist] = ilc_run_calculation_fn(sim, inlist)
  // This will run once in a thread


  defaultevents = 0;
    
  input1 = inlist(1);

  [sim] = ld_printf(sim, defaultevents, input1, "input1 = ", 10);

  //
  // A resource demanding Scilab calculation
  //
  
  [sim, dummyin] = ld_const(sim, defaultevents, 1);

  // Scilab commands

//  init_command = " exec(" + char(39) + "online_estimation/init.sce" + char(39) + "); ";    // execute an sce-file

  init_command = "";
  exec_command = " scilab_interf.outvec1 = scilab_interf.invec1 * 2 ";

  [sim,out] = ld_scilab(sim, defaultevents, in=input1, invecsize=10, outvecsize=10, init_command, ...
                                            exec_command, "", "/localhome/arne/openrtdynamics/trunk/scilab-5.3.3/bin/scilab");

//  [sim,out__] = ld_demux(sim, defaultevents, 10, out);

  result = out;    
  [sim] = ld_printf(sim, defaultevents, out, "result = ", 10);

  //[sim, output] = ld_constvec(sim, events, vec=1:10);
 


  // output of schematic
  outlist = list(result);
endfunction

//
// Set up simulation schematic
//

function [sim, outlist, active_state, x_global_kp1, userdata] = state_mainfn(sim, inlist, x_global, state, statename, userdata)
  // This function is called multiple times: once for each state.
  // At runtime these are different nested simulation. Switching
  // between them is done, where each simulation represents a
  // certain state.
  
  printf("defining state %s (#%d) ... \n", statename, state  );
  
  ILC = userdata;
  N = ILC.N;
  
  // define names for the first event in the simulation
  events = 0;
  
  // print out some state information
//  [sim] = ld_printf(sim, events, in=inlist(1), str="state"+string(state)+": indata(1)", insize=1);
//  [sim] = ld_printf(sim, events, in=inlist(2), str="state"+string(state)+": indata(2)", insize=2);

  [sim] = ld_printf(sim, events, in=x_global, str="state"+string(state)+": x_global", insize=N);

  // demultiplex x_global
  //[sim, x_global] = ld_demux(sim, events, vecsize=10, invec=x_global);

  // sample data fot output
  [sim, outdata1] = ld_constvec(sim, events, vec=[0]);




  select state
    case 1 // state 1 10
      // wait N simulation steps and then switch to state 2
      
      [sim, counter] = ld_play_simple(sim, events, r=1:(N+1)); // +1 to get all y_vec values into x_global
      [sim, active_state] = ld_compare_01(sim, events, in=counter,  thr=N-1  +  1); // +1 to get all y_vec values into x_global
      [sim, active_state] = ld_gain(sim, events, active_state, 2);

      [sim] = ld_printf(sim, events, in=counter, str="counter=", insize=1);      
      [sim] = ld_printf(sim, events, in=active_state, str="active", insize=1);
//      [sim, save0] = ld_dumptoiofile(sim, events, "activate_state_s1.dat", active_state);

      [sim, u] = ld_extract_element(sim, events, invec=x_global, pointer=counter, vecsize=N );
      [sim] = ld_printf(sim, events, in=u, str="u=", insize=1);
      outdata1 = u;

      y = inlist(1);
      [sim, y_vec] = ld_insert_element(sim, events, in=y, pointer=counter, vecsize=N );
      [sim] = ld_printf(sim, events, in=y_vec, str="y_vec=", insize=N);
      x_global = y_vec;

      
      //[sim, x_global(1)] = ld_add_ofs(sim, events, x_global(1), 1); // increase counter 1 by 1
    case 2         // calculate new actuation trajectory

        [sim, zero] = ld_const(sim, events, 0);
//        [sim, startcalc] = ld_initimpuls(sim, events);
        [sim, startcalc] = ld_play_simple(sim, defaultevents, [1,0]); 


        // a nested simulation that runns asynchronously (in a thread) to the main simulation
        [sim, outlist, computation_finished] = ld_simnest(sim, events, ...
                              inlist=list(x_global), ...
                              insizes=[N], outsizes=[N], ...
                              intypes=[1], outtypes=[1], ...
                              fn_list=list(ilc_run_calculation_fn), ...
                              dfeed=1, asynchron_simsteps=1, ...
                              switch_signal=zero, reset_trigger_signal=startcalc         );

         output1 = outlist(1);
         [sim] = ld_printf(sim, events, in=output1, str="output1=", insize=N);
         x_global = output1;
         // computation_finished is one, when finished else zero
        //[sim, outdata1] = ld_constvec(sim, events, vec=[-2]);

       [sim] = ld_printf(sim, events, in=computation_finished, str="computation_finished", insize=1);
       
       // go on to state 3 when the computation has finished
       [sim, active_state] = ld_const(sim, events, 0);  // by default: no state switch
       [ sim, active_state ] = ld_cond_overwrite(sim, events, in=active_state, condition=computation_finished, setto=3); // go to state 3 if finished

  case 3 // state 3
      // wait for trigger
      // WHEN TO CHANGE THE STATE
      trigger_state = inlist(2);
      [sim, zero] = ld_const(sim, events, 0);
      [sim, greater_zero] = ld_compare_01(sim, events, trigger_state, 0.1);
      [sim, active_state] = ld_const(sim, events, 0);  // by default: no state switch      
      [sim, active_state] = ld_cond_overwrite(sim, events, in=active_state, condition=greater_zero, setto=1); // go to state 1 if finished

      [sim] = ld_printf(sim, events, in=active_state, str="waiting in state3 - active_state: ", insize=1);
      //[sim, outdata1] = ld_constvec(sim, events, vec=[-3]);
  end

  // multiplex the new global states
  [sim, x_global] = ld_demux(sim, events, vecsize=N, invec=x_global);
  [sim, x_global_kp1] = ld_mux(sim, events, vecsize=N, inlist=x_global);
  
  // the user defined output signals of this nested simulation
  outlist = list(outdata1);
endfunction


// This is the main top level schematic
function [sim, outlist] = schematic_fn(sim, inlist)
  // ILC parameters
  ILC.u0 = [1,2,3,4,5,6,7,8,9,10];
  ILC.N = length(ILC.u0);

  //[sim, ILC.trigger_state] = ld_const(sim, defaultevents, 0);
  [sim, trigger_state] = ld_parameter(sim, defaultevents, "trigstate", [0]);

  // detect changed in the control signal
  [sim, trigger_state] = ld_detect_step_event(sim, defaultevents, in=trigger_state, eps=0.2);
  
  [sim, y] = ld_play_simple(sim, defaultevents, [linspace(0,1000, 100000)]);

  // set-up three states represented by three nested simulations
  [sim, outlist, x_global, active_state,userdata] = ld_statemachine(sim, ev=defaultevents, ...
      inlist=list(y, trigger_state), ..
      insizes=[1,1], outsizes=[1], ... 
      intypes=[ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT], ...
      nested_fn=state_mainfn, Nstates=3, state_names_list=list("state1", "state2","state3"), ...
      inittial_state=1, x0_global=ILC.u0, userdata=ILC  );

  [sim] = ld_printf(sim, defaultevents, in=outlist(1), str="outdata1=", insize=1);
  // signal telling the currently active simulation / state
  switch_state = active_state;
  
  // outlist contains the output of the currently active nested simulation respectively


  // just some dummy proceedings
  // sum up two inputs
  //[sim,out] = ld_add(sim, defaultevents, list(u1, u2), [1, 1] );
  
  // save result to file
  //[sim, save0] = ld_dumptoiofile(sim, defaultevents, "result.dat", out);
      
  // output of schematic
  //outlist = list(out); // Simulation output #1
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
save_irparam(par, 'ilc_state_machine.ipar', 'ilc_state_machine.rpar');

// clear
par.ipar = [];
par.rpar = [];




// optionally execute
//messages=unix_g(ORTD.ortd_executable+ ' -s ilc_state_machine -i 901 -l 14');
//disp(messages);
//
//
//// load results
//A = fscanfMat('result.dat');
//
//scf(1);clf;
//plot(A(:,1), 'k');
//
