// A template for switching between multiple controllers
// calc_reemg is the main function


// Example for usage:

  [sim, zero] = ld_const(sim, sim.ev.defev, 0); // 
  
  [sim, switch] = ld_parameter(sim, sim.ev.defev, str="controller_switch", initial_param=0); // A TCP-line interface for choosing the current controller
  [sim, out] = controller_switch(sim, sim.ev.defev, switch, in1=zero, in2=zero);




// To be modified funcions:

function [sim, outlist, active_state, x_global_kp1, userdata] = controller_switch_sm(sim, inlist, x_global, state, statename, userdata)
  // This function is called multiple times: once for each state.
  // At runtime these are different nested simulation. Switching
  // between them is done, where each simulation represents a
  // certain state.
  
  printf("defining state %s (#%d) ... userdata(1)=%s\n", statename, state, userdata(1) );
  
  // define names for the first event in the simulation
  ev = 0;
  
  // print out some state information
//  [sim] = ld_printf(sim, ev, in=inlist(1), str="state"+string(state)+": indata(1)", insize=1);
//  [sim] = ld_printf(sim, ev, in=inlist(2), str="state"+string(state)+": indata(2)", insize=2);

  [sim] = ld_printf(sim, ev, in=x_global, str="<cntrl_state "+statename+"> x_global", insize=1);

  //
  switch = inlist(1);  [sim, switch] = ld_gain(sim, ev, switch, 1);
  in1 = inlist(2);
  in2 = inlist(3);

  // demultiplex x_global
  [sim, x_global] = ld_demux(sim, ev, vecsize=1, invec=x_global);

  // sample data for output
  [sim, out] = ld_const(sim, ev, 0);

  // define different controllers here
  select state
    case 1 // state 1
      [sim, out] = ld_gain(sim, ev, in1, 10);

      active_state = switch;
    case 2 // state 2
      [sim, out] = ld_gain(sim, ev, in2, 10);

      active_state = switch;
    case 3 // state 3
      [sim, out] = ld_gain(sim, ev, in1, 10);

      active_state = switch;
  end

  // multiplex the new global states
  [sim, x_global_kp1] = ld_mux(sim, ev, vecsize=1, inlist=x_global);
  
  // the user defined output signals of this nested simulation
  outlist = list(out);
endfunction

function [sim, out] = controller_switch(sim, ev, switch, in1, in2)
// switch is an integer value above or equal to 1,
// which refers to the currently active state
// If switch changes, the state also changes


  [sim, switch] = ld_detect_step_event(sim, ev, in=switch, eps=0.2);



  // set-up three states represented by three nested simulations
  [sim, outlist, x_global, active_state,userdata] = ld_statemachine(sim, ev=ev, ...
      inlist=list(switch, in1, in2), ..
      insizes=[1,1,1], outsizes=[1], ... 
      intypes=[ORTD.DATATYPE_FLOAT*ones(3,1) ], outtypes=[ORTD.DATATYPE_FLOAT], ...
      nested_fn=controller_switch_sm, Nstates=3, state_names_list=list("cntlr1", "cntrl2", "cntrl3"), ...
      inittial_state=1, x0_global=[1], userdata=list("Place", "any", "data")  );

  out = outlist(1);
endfunction



















