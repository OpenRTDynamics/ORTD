

// ADAPT: Point to your Scilab5 binary
ORTD.ONLINE_SCILAB_BIN = "./dependencies/scilab/scilab-5.3.3/bin/scilab";
// To be modified funcions:

function [sim, outlist, active_state, x_global_kp1, userdata] = experiment_sm(sim, inlist, x_global, state, statename, userdata)
  // This function is called multiple times: once for each state.
  // At runtime these are different nested simulations. Switching
  // between them is done, where each simulation represents a
  // certain state.
  
  
  function [sim, finished, out] = experiment(sim, ev, in1, in2)
      // Do the experiment
      
      [sim, out] = ld_const(sim, ev, 0);
      
      [sim, finished] = ld_steps2(sim, ev, activation_simsteps=4*50, values=[0,1] );  // FIXME ld_steps2 funktioniert hier nicht, wenn 2 Schichten eine ld_statemachine verwendet werden. Es schein, als ob der Ausgang nicht zurück gesetzt wird. ld_play funktioniert
//      [sim, finished] = ld_play_simple(sim, ev, [ zeros(4*50, 1); 1 ]);
  endfunction

  function [sim, outlist] = evaluation(sim, inlist)
      // This superblock will run the evaluation of the experiment in a thread.
      // The superblock describes a sub-simulation, whereby only one step is simulated
      // which is enough to call scilab one signle time
      
      ev = 0;

      // Get the inputs to the computation
      inputv = inlist(1);
      [sim] = ld_printf(sim, ev, inputv, "inputv = ", 10);

      //
      // A resource demanding Scilab calculation
      //

      [sim, dummyin] = ld_const(sim, ev, 1);

      // Scilab commands
      //  init_command = " exec(" + char(39) + "online_estimation/init.sce" + char(39) + "); ";    // execute a sce-file on startup
      init_command = "";
      exec_command = " scilab_interf.outvec1 = 1:10  ";

      [sim,out] = ld_scilab(sim, ev, in=inputv, invecsize=10, outvecsize=10, init_command, ...
      exec_command, "", ORTD.ONLINE_SCILAB_BIN);

      result = out; [sim] = ld_printf(sim, ev, out, "result: = ", 10);

      // is scilab ready?
      [sim,out__] = ld_demux(sim, ev, 10, out);
      compready = out__(1); //   

      // [sim, result] = ld_constvec(sim, ev, 1:10);

      // output of schematic
      outlist = list(result);
  endfunction

  
  printf("defining state %s (#%d) ... userdata(1)=%s\n", statename, state, userdata(1) );
  
  // define names for the first event in the simulation
  ev = 0;
  
  // print out some state information
//  [sim] = ld_printf(sim, ev, in=inlist(1), str="state"+string(state)+": indata(1)", insize=1);
//  [sim] = ld_printf(sim, ev, in=inlist(2), str="state"+string(state)+": indata(2)", insize=2);

  [sim] = ld_printf(sim, ev, in=x_global, str="<cntrl_state "+statename+"> x_global", insize=1);

  //
  in1 = inlist(1);
  in2 = inlist(2);

  // demultiplex x_global
  [sim, x_global] = ld_demux(sim, ev, vecsize=1, invec=x_global);

  // sample data for output
  [sim, out] = ld_const(sim, ev, 0);

  // define different controllers here
  select state
     case 1 // state 1
       // The experiment
       [sim, finished, out] = experiment(sim, ev, in1, in2);
    
       [sim, active_state] = ld_const(sim, ev, 0);  // by default: no state switch       
       [sim, active_state ] = ld_cond_overwrite(sim, ev, in=active_state, condition=finished, setto=2); // Go to state 2 when finished

     case 2 // state 2
    
        // input should be a signal vector of size 10
        [sim, in] = ld_constvec(sim, ev, vec=1:10); // some dummy input to the computation

        [sim, zero] = ld_const(sim, ev, 0);
        [sim, startcalc] = ld_initimpuls(sim, ev); // start computation immediately when state 2 is entered

        // a nested simulation that runns asynchronously (in a thread) to the main simulation
        [sim, outlist, computation_finished] = ld_simnest(sim, ev, ...
                              inlist=list(in), ...
                              insizes=[10], outsizes=[10], ...
                              intypes=[ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT], ...
                              fn_list=list(evaluation), ...
                              dfeed=1, asynchron_simsteps=1, ...
                              switch_signal=zero, reset_trigger_signal=startcalc  );

         output1 = outlist(1);
         // computation_finished is one, when finished else zero

         [sim] = ld_printf(sim, ev, in=computation_finished, str="computation_finished", insize=1);
       
         // WHEN TO CHANGE THE STATE
         [sim, active_state] = ld_const(sim, ev, 0);  // by default: no state switch
         [sim, active_state ] = ld_cond_overwrite(sim, ev, in=active_state, condition=computation_finished, setto=3); // go to state 3 if        
       
    case 3 // state 3
      [sim, out] = ld_gain(sim, ev, in1, 10);


      [sim, active_state] = ld_const(sim, ev, 0);  // by default: no state switch       
  end

  // multiplex the new global states
  [sim, x_global_kp1] = ld_mux(sim, ev, vecsize=1, inlist=x_global);
  
  // the user defined output signals of this nested simulation
  outlist = list(out);
endfunction


function [sim, out] = experiment_main(sim, ev, in1, in2)
//
// in1 and in2 are some arbitrary inputs
//


  // set-up three states represented by three nested simulations
  [sim, outlist, x_global, active_state,userdata] = ld_statemachine(sim, ev=ev, ...
      inlist=list(in1, in2), ..
      insizes=[1,1], outsizes=[1], ... 
      intypes=[ORTD.DATATYPE_FLOAT*ones(2,1) ], outtypes=[ORTD.DATATYPE_FLOAT], ...
      nested_fn=experiment_sm, Nstates=3, state_names_list=list("experiment", "evaluation", "finished"), ...
      inittial_state=1, x0_global=[1], userdata=list("Place", "any", "data")  );

  out = outlist(1);
endfunction




