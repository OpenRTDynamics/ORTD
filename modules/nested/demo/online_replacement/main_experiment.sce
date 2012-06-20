//
//    Copyright (C) 2012  Christian Klauer
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
// Simple example for using online exchangeable schematics
//
// execute within scilab and run "libdyn_generic_exec -s main -i 901 -l 100" within 
// the directory of this file
//
//


thispath = get_absolute_file_path('main_experiment.sce');
cd(thispath);


z = poly(0,'z');




               
function [sim, outlist, userdata] = define_schematics( sim, inlist, schematic_type, userdata )
//    
//    The nested simulation contains two sub-simulations:
//    
//    1) A schematic, which commonly contains nothing and is switched to
//       when the replacement is in progress (which may take some time)
//
//    2) The schematic, which actually contains the algorithm to execute
//
//    Here, the initial simulations are defined, which can then be 
//   replaced during runtime
// 

  printf("Defining a replaceable controller. My userdata string is %s\n", userdata(1) );

  // an input
  input1 = inlist(1);

  // default event
  ev = 0;

  // a zero
  [sim, zero] = ld_const(sim, ev, 0);

  select schematic_type  

    case 'spare'  // Here the controller can be defined, which is active while the replacement process is ongoing
      outlist = list(zero);

    case 'default' // Here the default controller can be defined
      outlist = list(zero);
      [sim] = ld_printf(sim, ev, in=input1, str="This is just a dummy simulation, which can be replaced.", insize=1);

  end
endfunction


function [sim] = main(sim, ev)

  [sim,aninput] = ld_const(sim, ev, 1.1234);

  //[sim, trigger_reload] = ld_play_simple(sim, ev, [0,0,0,0,0,0,1,0]);

  // set-up the inteface to the GUI
  [sim, gui_trigger_reload] = ld_parameter(sim, ev, str="gui_trigger_reload", initial_param=0);
  [sim, trigger_reload] = ld_detect_step_event2(sim, ev, in=gui_trigger_reload, eps=0.1);

  [sim, outlist, userdata, replaced] = ld_ReplaceableNest(sim, ev, ...
                        inlist=list(aninput), trigger_reload, list(define_schematics), ...
                        insizes=[1], outsizes=[1], ...
                        intypes=ORTD.DATATYPE_FLOAT*[1], ...
                        outtypes=ORTD.DATATYPE_FLOAT*[1], ...
                        simnest_name="DemoReplaceableController", irpar_fname="replacement", dfeed=1, ...
                        userdata=list("Hallo") );

  [sim] = ld_printf(sim, ev, in=outlist(1), str="The nested, replaceable sim returns", insize=1);

  // set-up the inteface to the GUI
  [sim] = ld_stream(sim, ev, in=outlist(1), str="ControlOutput", insize=1);
                  
endfunction







// This is the main top level schematic
function [sim, outlist] = schematic_fn(sim, inlist)
   u1 = inlist(1); // Simulation input #1
   u2 = inlist(2); // Simulation input #2
   

  [sim] = main(sim, ev=defaultevents );   

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
save_irparam(par, 'main.ipar', 'main.rpar');

// clear
par.ipar = [];
par.rpar = [];

//
//
//
//// optionally execute
//messages=unix_g(ORTD.ortd_executable+ ' -s main -i 901 -l 100');
//
//
//// load results
//A = fscanfMat('result.dat');
//
//scf(1);clf;
//plot(A(:,1), 'k');

