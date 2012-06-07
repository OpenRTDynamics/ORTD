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








//
// Set up simulation schematic
//


function [sim, outlist, userdata, replaced] = ReplaceableController(sim, ev, inlist, trigger_reload, insizes, outsizes, intypes, outtypes, simnest_name, irpar_fname, dfeed, userdata)


function [sim, outlist] = exch_helper_thread(sim, inlist)
      // This superblock will run the evaluation of the experiment in a thread.
      // The superblock describes a sub-simulation, whereby only one step is simulated
      // which is enough to call scilab one signle time


  defaultevents = 0;
    
  inputv = inlist(1);

  [sim] = ld_printf(sim, defaultevents, inputv, "inputv = ", 10);

  //
  // A resource demanding Scilab calculation
  //
  
  [sim, dummyin] = ld_const(sim, defaultevents, 1);

  if 1==0 then
      // Scilab commands

      //  init_command = " exec(" + char(39) + "online_estimation/init.sce" + char(39) + "); ";    // execute an sce-file

      init_command = "";
      exec_command = " scilab_interf.outvec1 = 1:10  ";

      [sim,out] = ld_scilab(sim, defaultevents, in=inputv, invecsize=10, outvecsize=10, init_command, ...
      exec_command, "", "/home/chr/scilab/scilab-5.3.3_64/bin/scilab");

      [sim,out__] = ld_demux(sim, defaultevents, 10, out);

      result = out;    
      [sim] = ld_printf(sim, defaultevents, out, "result: = ", 10);



      // is scilab ready?
      compready = out__(1); //   
  else
      [sim, compready]  = ld_const(sim, defaultevents, 1);
//      [sim, result]  = ld_const_vec(sim, defaultevents, 123);
      [sim, result] = ld_constvec(sim, defaultevents, vec=1:10)
  end

  // replace schematic RST_ident
  [sim, exchslot] = ld_const(sim, defaultevents, 2);                            
  [sim, out] = ld_nested_exchffile(sim, defaultevents, compresult=compready, slot=exchslot, ... 
                                   fname=irpar_fname, simnest_name);


  // output of schematic
  outlist = list(result);
endfunction



function [sim, outlist, userdata ] = replaceable_cntrl_main(sim, inlist, par)
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

  ev = 0;

  cntrlN = par(1); // the number of the nested schematics (one of two) "1" means the 
                   // dummy schematic which is activated while the 2nd "2" is exchanged during runtime
  userdata = par(2);
  
  string1 = userdata(1);
  
  input1 = inlist(1);
  
  
  // a zero
  [sim, zero] = ld_const(sim, ev, 0);

  printf("Defining a replaceable controller\n");
  
  if (cntrlN == 2) then  // is this the schematic 2) ?
          // Here the  default controller can be defined
          [sim, output] = ld_gain(sim, ev, input1, 2); // 
          [sim] = ld_printf(sim, ev, in=zero, str="This is just a dummy simulation, which can be replaced.", insize=1);
  end

  
  outlist = list(zero);
endfunction

  [sim,zero] = ld_const(sim, ev, 0);
  [sim,active_sim] = ld_const(sim, ev, 1);

  //
  // Here the controller is nested, which can be replaced online
  //

        [sim, outlist, computation_finished, userdata] = ld_simnest2(sim, ev=[ ev ] , ...
                       inlist, ...
                       insizes, outsizes, ...
                       intypes, ...
                       outtypes, ...
                       nested_fn=replaceable_cntrl_main, Nsimulations=2, dfeed, ...
                       asynchron_simsteps=0, switch_signal=active_sim, ...
                       reset_trigger_signal=zero, userdata=list(userdata, "any", "data"), ...
                       simnest_name );

       [sim] = ld_printf(sim, ev, in=outlist(1), str="The nested, replaceable sim returns", insize=1);


  //
  // The exchange helper, which consits of a threaded sub-simulation
  // for loading the schematic from files.
  // The replacement is triggered by setting "startcalc" to 1 for one sample
  //

        // input to the calculation
        [sim, input1] = ld_constvec(sim, ev, vec=1:10);

        [sim, zero] = ld_const(sim, ev, 0);
//        [sim, startcalc] = ld_initimpuls(sim, ev);
        startcalc = trigger_reload;


        // a nested simulation that runns asynchronously (in a thread) to the main simulation
        [sim, outlist__, computation_finished] = ld_simnest(sim, ev, ...
                              inlist=list(input1), ...
                              insizes=[10], outsizes=[10], ...
                              intypes=[ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT], ...
                              fn_list=list(exch_helper_thread), ...
                              dfeed=1, asynchron_simsteps=1, ...
                              switch_signal=zero, reset_trigger_signal=startcalc         );

         output1 = outlist__(1);
         // computation_finished is one, when finished else zero

         replaced = computation_finished; // FIXME and successful
endfunction


               

function [sim] = main(sim, ev)

  [sim,aninput] = ld_const(sim, ev, 1.1234);

  //[sim, trigger_reload] = ld_play_simple(sim, ev, [0,0,0,0,0,0,1,0]);

  [sim, gui_trigger_reload] = ld_parameter(sim, ev, str="gui_trigger_reload", initial_param=0);
  [sim, trigger_reload] = ld_detect_step_event2(sim, ev, in=gui_trigger_reload, eps=0.1);

  [sim, outlist, userdata, replaced] = ReplaceableController(sim, ev, ...
                        inlist=list(aninput), trigger_reload, ...
                        insizes=[1], outsizes=[1], ...
                        intypes=ORTD.DATATYPE_FLOAT*[1], ...
                        outtypes=ORTD.DATATYPE_FLOAT*[1], ...
                        simnest_name="DemoReplaceableController", irpar_fname="replacement", dfeed=1, ...
                        userdata=list() );

  [sim] = ld_printf(sim, ev, in=outlist(1), str="The nested, replaceable sim returns", insize=1);
                  
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

