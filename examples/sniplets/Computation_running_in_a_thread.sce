





function [sim, outlist] = ilc_run_calculation_fn(sim, inlist)
      // This superblock will run the evaluation of the experiment in a thread.
      // The superblock describes a sub-simulation, whereby only one step is simulated
      // which is enough to call scilab one signle time


  defaultevents = 0;
    
  inputv = inlist(1);

  [sim, zero] = ld_const(sim, events, 0);


  [sim] = ld_printf(sim, defaultevents, inputv, "inputv = ", 10);

  //
  // A resource demanding Scilab calculation
  //
  
  [sim, dummyin] = ld_const(sim, defaultevents, 1);


  init_command = "";
  exec_command = " scilab_interf.outvec1 = 1:10  ";

  [sim,out] = ld_scilab(sim, defaultevents, in=inputv, invecsize=10, outvecsize=10, init_command, ...
                                            exec_command, "", "scilab5");

  [sim,out__] = ld_demux(sim, defaultevents, 10, out);

  result = out;    
  [sim] = ld_printf(sim, defaultevents, out, "result: = ", 10);


 
  // is scilab ready?
  compready = out__(1); //   


  [sim, result] = ld_constvec(sim, defaultevents, 1:10);



  // output of schematic
  outlist = list(result);
endfunction



//
// To be included into an state machine
//


        // input should be a signal vector of size 10
        [sim, input] = ld_constvec(sim, events, vec=1:10)

        [sim, zero] = ld_const(sim, events, 0);
        [sim, startcalc] = ld_initimpuls(sim, events);


        // a nested simulation that runns asynchronously (in a thread) to the main simulation
        [sim, outlist, computation_finished] = ld_simnest(sim, events, ...
                              inlist=list(input), ...
                              insizes=[10], outsizes=[10], ...
                              intypes=[ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT], ...
                              fn_list=list(ilc_run_calculation_fn), ...
                              dfeed=1, asynchron_simsteps=1, ...
                              switch_signal=zero, reset_trigger_signal=startcalc         );

         output1 = outlist(1);
         // computation_finished is one, when finished else zero

       [sim] = ld_printf(sim, events, in=computation_finished, str="computation_finished", insize=1);
       
       // WHEN TO CHANGE THE STATE
       [sim, active_state] = ld_const(sim, events, 0);  // by default: no state switch
       [ sim, active_state ] = ld_cond_overwrite(sim, events, in=active_state, condition=computation_finished, setto=3); // go to state 3 if finished







