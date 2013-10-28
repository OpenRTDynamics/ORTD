function [sim, finished, outlist] = ld_AutoExperiment(sim, ev, inlist, insizes, outsizes, intypes, outtypes, ThreadPrioStruct, experiment_fn, whileComputing_fn, evaluation_fn, whileIdle_fn)  // PARSEDOCU_BLOCK
//
// Automatically perform an experiment and the ongoing evaluation.
// The computation required for the evaluation is performed in a the background
// by means of a thread.
// 
// There are several callback functions that describe:
// 
// experiment_fn: The schematic for performing the experiment, e.g. collecting data
// whileComputing_fn: The schematic that is activated during the computation is active_state
// evaluation_fn: The schematic that performs the evaulation in a thread.
//                One simulation step is performed here
// whileIdle_fn:  The schematic that is active when the procedure finished.
// 
// 
// 
// 
// The prototypes are (Hereby outlist and inlist are lists of the signals that are forwarded 
// to the I/O of ld_AutoExperiment:
// 
// [sim, finished, outlist] = experiment_fn(sim, ev, inlist)
// [sim, outlist] = whileComputing_fn(sim, ev, inlist)
// [sim, CalibrationOk, userdata] = evaluation_fn(sim, userdata)
// [sim, outlist] = whileIdle_fn(sim, ev, inlist)
// 
// NOTE: Not everything is finished by now
// 


  function [sim, outlist, userdata] = evaluation_Thread(sim, inlist, userdata)

    [sim, CalibrationOk, userdata] = evaluation_fn(sim, userdata);

    outlist = list(CalibrationOk);
  endfunction

  function [sim, outlist, active_state, x_global_kp1, userdata] = experiment_sm(sim, inlist, x_global, state, statename, userdata)
    // This function is called multiple times: once for each state.
    // At runtime these are different nested simulations. Switching
    // between them is done, where each simulation represents a
    // certain state.
    
    ev = 0;
    printf("ld_AutoExperiment: defining state %s (#%d) ...\n", statename, state);
    
    
    // print out some state information
//     [sim] = ld_printf(sim, ev, in=x_global, str="<cntrl_state "+statename+"> x_global", insize=1);


    // define different controllers here
    select state
      case 1 // state 1
	// The experiment
	[sim, finished, outlist, userdata] = experiment_fn(sim, ev, inlist, userdata);
      
	[sim, active_state] = ld_const(sim, ev, 0);  // by default: no state switch       
	[sim, active_state ] = ld_cond_overwrite(sim, ev, in=active_state, condition=finished, setto=2); // Go to state 2 when finished

      case 2 // state 2
          // run something while the computation is running
	  [sim, outlist, userdata] = whileComputing_fn(sim, ev, inlist, userdata);
	
	  // Create a thread for performing the computation in the background
	  [sim, startcalc] = ld_initimpuls(sim, 0); // triggers the computation only once when entering this state
	  [sim, outlist, computation_finished] = ld_async_simulation(sim, 0, ...
				inlist=list(), ...
				insizes=[], outsizes=[1], ...
				intypes=[], outtypes=[ORTD.DATATYPE_INT32], ...
				nested_fn = evaluation_Thread, ...
				TriggerSignal=startcalc, name="Comp Thread", ...
				ThreadPrioStruct, userdata=list() );


          //
          CalibrationOk = outlist(1);
          [sim,CalibrationOk_] = ld_Int32ToFloat(sim, 0, CalibrationOk);

// 	  [sim] = ld_printf(sim, ev, in=computation_finished, str="computation_finished", insize=1);

          [sim, FinshedOk] = ld_and(sim, 0, list( CalibrationOk_, computation_finished ));

	  // WHEN TO CHANGE THE STATE
	  [sim, active_state] = ld_const(sim, ev, 0);  // by default: no state switch
	  [sim, active_state ] = ld_cond_overwrite(sim, ev, in=active_state, condition=computation_finished, setto=3); // go to state 3 if        
	
      case 3 // state 3
	  [sim, outlist, userdata] = whileIdle_fn(sim, ev, inlist, userdata);

	  [sim, active_state] = ld_const(sim, ev, 0);  // by default: no state switch       
    end

    x_global_kp1 = x_global;    
  endfunction




  // set-up three states represented by three nested simulations
  [sim, outlist, x_global, active_state,userdata] = ld_statemachine(sim, ev=ev, ...
      inlist, ..
      insizes, outsizes, ... 
      intypes, outtypes, ...
      nested_fn=experiment_sm, Nstates=3, state_names_list=list("experiment", "evaluation", "finished"), ...
      inittial_state=1, x0_global=[1], userdata=list()  );

  
  finished = active_state;
endfunction


