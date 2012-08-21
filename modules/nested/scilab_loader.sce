

// Interfacing functions are placed in this place




function [sim, outlist, computation_finished] = ld_simnest(sim, ev, inlist, insizes, outsizes, intypes, outtypes, fn_list, dfeed, asynchron_simsteps, switch_signal, reset_trigger_signal  ) // PARSEDOCU_BLOCK
// 
// %PURPOSE: create one (or multiple) nested libdyn simulation within a normal libdyn block it is possible to switch between them by an special input signal
//
// INPUTS: 
//
// switch_signal: signal used to switch between different nested simulations
// reset_trigger_signal: when 1 the current simulation is reset (sync)
//                       OR when 1 the current simulation is triggered (async)
// inlist - list( ) of input signals to the block, that will be forwarded to the nested simulation(s)
//
// PARAMETERS:
// 
// ev - events to be forwarded to the nested simulation
// insizes - input ports configuration
// outsizes - output ports configuration
// intypes - ignored for now, put ORTD.DATATYPE_FLOAT for each port
// outtypes - ignored for now, put ORTD.DATATYPE_FLOAT for each port
// fn_list - list( ) of scilab functions defining sub-schematics
// dfeed - the block containing all sub-schematics is configured with dfeed
// asynchron_simsteps - if > 0 asynchron_simsteps steps will be simulated in a thread
//                     when finished the result becomes available to the blocks outports
//                     if == 0 the nested simulation runns synchronous to the upper level simulation.
// 
// OUTPUTS:
// 
// outlist - list( ) of output signals
// computation_finished - optional and only meanful if asynchron_simsteps > 0 (means async computation)
// 


  parlist = new_irparam_set();

  N1 = length(insizes);
  N2 = length(outsizes);
  N3 = length(intypes);
  N4 = length(outtypes);

  // check input signals for correctness
  try 
    libdyn_check_object(sim, switch_signal);
  catch
    error("Input signal switch_signal is not correct");
  end

  try
    libdyn_check_object(sim, reset_trigger_signal);
  catch
    error("Input signal reset_trigger_signal is not correct");
  end
    
  
  // check for sizes
  // ...
  if (length(inlist) ~= N1) then
    error("length inlist invalid or length of insizes invalid\n");
  end

  if N4 ~= N2 then
    error("length of outsizes invalid\n");
  end

  if N1 ~= N3 then
    error("length of intypes invalid\n");
  end


   parlist = new_irparam_elemet_ivec(parlist, insizes, 10); 
   parlist = new_irparam_elemet_ivec(parlist, outsizes, 11); 
   parlist = new_irparam_elemet_ivec(parlist, intypes, 12); 
   parlist = new_irparam_elemet_ivec(parlist, outtypes, 13); 


  if (length(dfeed) ~= 1) then
    error("dfeed should be of size 1\n");
  end

  if (length(asynchron_simsteps) ~= 1) then
    error("asynchron_simsteps should be of size 1\n");
  end


  Nsimulations = length(fn_list);
   parlist = new_irparam_elemet_ivec(parlist, [Nsimulations, dfeed, asynchron_simsteps], 20); 



  // Go through all schematics
  // and set them up
  // finially they are stored within an irpar structure under different irpar ids
  irpar_sim_idcounter = 900;

  for i = 1:Nsimulations

    fn = fn_list(i);
    //    [sim_container_irpar, sim] = libdyn_setup_schematic(fn, insizes, outsizes, intypes, outtypes); // outtypes and intypes are not handled at the moment
    [sim_container_irpar, nested_sim] = libdyn_setup_schematic(fn, insizes, outsizes); 

    // pack simulations into irpar container with id = 901
    parlist = new_irparam_container(parlist, sim_container_irpar, irpar_sim_idcounter);

    // increase irpar_sim_idcounter so the next simulation gets another id
    irpar_sim_idcounter = irpar_sim_idcounter + 1;
  end



//   // combine ir parameters
   blockparam = combine_irparam(parlist);


  // blockparam.ipar and blockparam.rpar now contain the blocks parameters

  // set-up the nested block

  btype = 15001;
//   [sim,blk] = libdyn_new_blk_generic(sim, ev, btype, [blockparam.ipar], [blockparam.rpar] );
  [sim,blk] = libdyn_new_block(sim, ev, btype, ipar=[blockparam.ipar], rpar=[blockparam.rpar], ...
                       insizes=[insizes(:)' ,1,1], outsizes=[outsizes(:)', 1], ...
                       intypes=[ones(insizes(:)') * ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT], ...
                       outtypes=[ones(outsizes(:)') * ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT]  );


// 
  // add switch and reset input signals
  blocks_inlist = inlist;
  blocks_inlist($+1) = switch_signal;

  if asynchron_simsteps == 0 then
    // connect reset input
    blocks_inlist($+1) = reset_trigger_signal; //reset_signal;
  else
    // connect trigger input
    blocks_inlist($+1) = reset_trigger_signal; //trigger_signal;
  end

  // connect all inputs
  printf("ld_simnest: connection of inputs\n");
  try
    [sim,blk] = libdyn_conn_equation(sim, blk, blocks_inlist );
  catch
    error("ld_simnest: One of the input signals in inlist could not be connected");
  end
 
  // connect all outputs
  Noutp = length(outsizes);

  outlist = list();
  for i = 0:(Noutp-1)
    [sim,out] = libdyn_new_oport_hint(sim, blk, i);   // ith port
    outlist(i+1) = out;
  end

  if (asynchron_simsteps > 0) then
    [sim,computation_finished] = libdyn_new_oport_hint(sim, blk, Noutp);   // the last port
  else
    computation_finished = 0; // dummy value
  end

endfunction

function [sim, outlist, computation_finished, userdata] = ld_simnest2(sim, ev, inlist, insizes, outsizes, intypes, outtypes, nested_fn, Nsimulations, dfeed, asynchron_simsteps, switch_signal, reset_trigger_signal, userdata, simnest_name  )  // PARSEDOCU_BLOCK
// 
// %PURPOSE: create one (or multiple) nested libdyn simulation within a normal libdyn block
//                It is possible to switch between them by an special input signal
//                Replacement schematics can be generated by the "ld_simnest2_replacement" function.
//                The "ld_nested_exchffile"-block can be used to load the schematic into the controller
//
// INPUTS: 
//
// switch_signal: signal used to switch between different nested simulations
// reset_trigger_signal: when 1 the current simulation is reset (sync)
//                       OR when 1 the current simulation is triggered (async)
// inlist - list( ) of input signals to the block, that will be forwarded to the nested simulation(s)
//
// PARAMETERS:
// 
// ev - events to be forwarded to the nested simulation
// insizes - input ports configuration
// outsizes - output ports configuration
// intypes - ignored for now, put ORTD.DATATYPE_FLOAT for each port
// outtypes - ignored for now, put ORTD.DATATYPE_FLOAT for each port
// nested_fn - scilab function defining sub-schematics
// Nsimulations - number of simulations to switch inbetween
// dfeed - the block containing all sub-schematics is configured with dfeed
// asynchron_simsteps - if > 0 asynchron_simsteps steps will be simulated in a thread
//                     when finished the result becomes available to the blocks outports
//                     if == 0 the nested simulation runns synchronous to the upper level simulation.
// switch_signal 
// reset_trigger_signal
// userdata - arbitrary user date
// simnest_name - a string name with which the nested is refered.
// 
// OUTPUTS:
// 
// outlist - list( ) of output signals
// computation_finished - optional and only meanful if asynchron_simsteps > 0 (means async computation)
// 


  parlist = new_irparam_set();

  N1 = length(insizes);
  N2 = length(outsizes);
  N3 = length(intypes);
  N4 = length(outtypes);

  // check for sizes
  // ...
  if (length(inlist) ~= N1) then
    error("length inlist invalid or length of insizes invalid\n");
  end

  if N4 ~= N2 then
    error("length of outsizes invalid\n");
  end

  if N1 ~= N3 then
    error("length of intypes invalid\n");
  end


   parlist = new_irparam_elemet_ivec(parlist, insizes, 10); 
   parlist = new_irparam_elemet_ivec(parlist, outsizes, 11); 
   parlist = new_irparam_elemet_ivec(parlist, intypes, 12); 
   parlist = new_irparam_elemet_ivec(parlist, outtypes, 13); 


  if (length(dfeed) ~= 1) then
    error("dfeed should be of size 1\n");
  end

  if (length(asynchron_simsteps) ~= 1) then
    error("asynchron_simsteps should be of size 1\n");
  end



   parlist = new_irparam_elemet_ivec(parlist, [Nsimulations, dfeed, asynchron_simsteps], 20); 
   parlist = new_irparam_elemet_ivec(parlist, ascii(simnest_name), 21); 



  // Go through all schematics
  // and set them up
  // finially they are stored within an irpar structure under different irpar ids
  irpar_sim_idcounter = 900;

  for i = 1:Nsimulations

    //    [sim_container_irpar, sim] = libdyn_setup_schematic(fn, insizes, outsizes, intypes, outtypes); // outtypes and intypes are not handled at the moment
    //[sim_container_irpar, nested_sim] = libdyn_setup_schematic(fn, insizes, outsizes); 
    [sim_container_irpar, nested_sim, userdata] = libdyn_setup_sch2(nested_fn, insizes, outsizes,  intypes, outtypes, list(i, userdata)); 

    // pack simulations into irpar container with id = 901
    parlist = new_irparam_container(parlist, sim_container_irpar, irpar_sim_idcounter);

    // increase irpar_sim_idcounter so the next simulation gets another id
    irpar_sim_idcounter = irpar_sim_idcounter + 1;
  end



//   // combine ir parameters
   blockparam = combine_irparam(parlist);


  // blockparam.ipar and blockparam.rpar now contain the blocks parameters

  // set-up the nested block

  btype = 15001;
//   [sim,blk] = libdyn_new_blk_generic(sim, ev, btype, [blockparam.ipar], [blockparam.rpar] );
  [sim,blk] = libdyn_new_block(sim, ev, btype, ipar=[blockparam.ipar], rpar=[blockparam.rpar], ...
                       insizes=[insizes(:)' ,1,1], outsizes=[outsizes(:)', 1], ...
                       intypes=[ones(insizes(:)') * ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT], ...
                       outtypes=[ones(outsizes(:)') * ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT]  );


// 
  // add switch and reset input signals
  blocks_inlist = inlist;
  blocks_inlist($+1) = switch_signal;

  if asynchron_simsteps == 0 then
    // connect reset input
    blocks_inlist($+1) = reset_trigger_signal; //reset_signal;
  else
    // connect trigger input
    blocks_inlist($+1) = reset_trigger_signal; //trigger_signal;
  end

  // connect all inputs
  [sim,blk] = libdyn_conn_equation(sim, blk, blocks_inlist );
 
  // connect all outputs
  Noutp = length(outsizes);

  outlist = list();
  for i = 0:(Noutp-1)
    [sim,out] = libdyn_new_oport_hint(sim, blk, i);   // ith port
    outlist(i+1) = out;
  end

  if (asynchron_simsteps > 0) then
    [sim,computation_finished] = libdyn_new_oport_hint(sim, blk, Noutp);   // the last port
  else
    computation_finished = 0; // dummy value
  end

endfunction


//   // FIXME: change PARSEDOCU_BLOCK to   // PARSEDOCU_FUNCTION
function [par, userdata] = ld_simnest2_replacement( insizes, outsizes, intypes, outtypes, nested_fn, userdata, N  )   // PARSEDOCU_BLOCK
// 
// %PURPOSE: create schematics that can be used as an online exchangeable simulation for nested simulations set-up using the ld_nested2 block
//
// The "ld_nested_exchffile"-block can be used to load the schematic into the controller
//
// INPUTS: 
//
// switch_signal: signal used to switch between different nested simulations
// reset_trigger_signal: when 1 the current simulation is reset (sync)
//                       OR when 1 the current simulation is triggered (async)
// inlist - list( ) of input signals to the block, that will be forwarded to the nested simulation(s)
//
// PARAMETERS:
// 
// insizes - input ports configuration
// outsizes - output ports configuration
// intypes - ignored for now, put ORTD.DATATYPE_FLOAT for each port
// outtypes - ignored for now, put ORTD.DATATYPE_FLOAT for each port
// nested_fn - scilab function defining sub-schematics
// N - FIXME ???
// 
// OUTPUTS:
//
// par - irpar data set. par.ipar and par.rpar contain the integer and double parameters list
// 


  parlist = new_irparam_set();

  N1 = length(insizes);
  N2 = length(outsizes);
  N3 = length(intypes);
  N4 = length(outtypes);

  // check for sizes
  // ...

  if N4 ~= N2 then
    error("length of outsizes invalid\n");
  end

  if N1 ~= N3 then
    error("length of intypes invalid\n");
  end

  if length(N) ~= 1 then
    error("N must be scalar\n");
  end

   parlist = new_irparam_elemet_ivec(parlist, insizes, 10); 
   parlist = new_irparam_elemet_ivec(parlist, outsizes, 11); 
   parlist = new_irparam_elemet_ivec(parlist, intypes, 12); 
   parlist = new_irparam_elemet_ivec(parlist, outtypes, 13); 



   parlist = new_irparam_elemet_ivec(parlist, [ -1, -1, -1, getdate("s"), -1, -1, getdate() ], 21); 
   parlist = new_irparam_elemet_ivec(parlist, [ ascii("ORTD-replacement-schematic") ], 22); 



  // Go through all schematics
  // and set them up
  // finially they are stored within an irpar structure under different irpar ids
  irpar_sim_idcounter = 100;

  for i = N

    //    [sim_container_irpar, sim] = libdyn_setup_schematic(fn, insizes, outsizes, intypes, outtypes); // outtypes and intypes are not handled at the moment
    //[sim_container_irpar, nested_sim] = libdyn_setup_schematic(fn, insizes, outsizes); 
//  printf("***\n");
//  pause;
    [sim_container_irpar, nested_sim, userdata] = libdyn_setup_sch2(nested_fn, insizes, outsizes, intypes, outtypes, list(i, userdata) ); 

    // pack simulations into irpar container with id = 901
    parlist = new_irparam_container(parlist, sim_container_irpar, irpar_sim_idcounter);

    // increase irpar_sim_idcounter so the next simulation gets another id
    irpar_sim_idcounter = irpar_sim_idcounter + 1;
  end



//   // combine ir parameters
   par = combine_irparam(parlist);
  // blockparam.ipar and blockparam.rpar now contain the blocks parameters

//   ipar = blockparam.ipar;
//   rpar = blockparam.rpar;

endfunction



function [sim, outlist, userdata ] = LD_STATEMACHINE_MAIN(sim, inlist, userdata)
  state = userdata(1);
  userdata_nested = userdata(2);
  fn = userdata(3);
  statename = userdata(4);  
  Nin_userdata = userdata(5);  
  Nout_userdata = userdata(6); 
  Nevents = userdata(7);
//  Nevents = 1;
  
  //pause;
    
  printf("Nested wrapper function: state=%s (#%d) Nin_userdata=%d Nout_userdata=%d, %d events are forwarded\n", statename, state, Nin_userdata, Nout_userdata, Nevents);
    
  x_global = inlist(Nin_userdata+1); // the global states
  
  // call the actual function
  [sim, outlist_inner, active_state, x_global_kp1, userdata] = fn(sim, inlist, x_global, state, statename, userdata_nested);


  if length(outlist_inner) ~= Nout_userdata then
      printf("Nested wrapper function: your provided function returns more or less outputs in outlist. Expecting %d but there are %d\n", Nout_userdata, length(outlist_inner));
      error(".");
  end

  outlist = list();

  N = length(outlist_inner);
  for i = 1:N
      outlist($+1) = outlist_inner(i);
  end
  outlist($+1) = active_state;
  outlist($+1) = x_global_kp1

//  outlist = list(, switch_event, global_states);
endfunction


function [sim, outlist, x_global, active_state, userdata] = ld_statemachine(sim, ev, inlist, insizes, outsizes, intypes, outtypes, nested_fn, Nstates, state_names_list, inittial_state, x0_global, userdata  ) // PARSEDOCU_BLOCK
// 
// %PURPOSE: A statemachine

//  create one (or multiple) nested libdyn simulation within a normal libdyn block
//                    it is possible to switch between them by an special signal. By this the
//                    nested simulations can be interpreted as a state machine, whereby each
//                    simulation describes a state. Additionally, global states (a vector of doubles)
//                    can be shared accross the different nested simulations.
//
// EXAMPLE: examples/state_machine.sce (probpably tells more then this reference)
//
// INPUTS: 
//
// inlist - list( ) of input signals to the block, that will be forwarded to the nested simulation(s)
//
// PARAMETERS:
// 
// ev - events to be forwarded to the nested simulation. The first one is also used by this nesting block as activation event
// insizes - input ports configuration
// outsizes - output ports configuration
// intypes - ignored for now, put ORTD.DATATYPE_FLOAT for each port
// outtypes - ignored for now, put ORTD.DATATYPE_FLOAT for each port
// nested_fn - list( ) of one! scilab functions defining sub-schematics
// Nstates - number of simulations to switch inbetween
// state_names_list - list() of strings - one for each state
// inittial_state - number of the inittial state. counting starts at 1
// x0_global - inittial state of x_global
// userdata - anythig - feed to the schematic_fn
// 
// OUTPUTS:
// 
// outlist - list( ) of output signals
// x_global - signal of the states x_global
// active_state - signal with the number of the currently active state
// 
// PROTOTYPE FOR nested_fn:
// 
// [sim, outlist, active_state, x_global_kp1, userdata] = state_mainfn(sim, inlist, x_global, state, statename, userdata)
//
// inlist - the same signals as feed to ld_statemachine
// x_global - vector signal of the global states
// state - constant: number of the state to define
// statename - string: name of the state to define
// userdata - a custom variable as feed to ld_statemachine
//
// RETURS:
//
// outlist - 
// active_state - signal describing the active state. If this is a singal containing -1 no state swicth occurs
// x_global_kp1 - vectorial signal of the new global states 
//



  parlist = new_irparam_set();

  N1 = length(insizes);
  N2 = length(outsizes);
  N3 = length(intypes);
  N4 = length(outtypes);

  NGlobStates = length(x0_global);
  N_datainports = length(insizes);
  N_dataoutports = length(outsizes);


  // check for sizes
  // ...
  if (length(inlist) ~= N1) then
    error("length inlist invalid or length of insizes invalid\n");
  end

  if N4 ~= N2 then
    error("length of outsizes invalid\n");
  end

  if N1 ~= N3 then
    error("length of intypes invalid\n");
  end


  // io for the nested simulation (this is not eq to the outer blocks io)

   nested_insizes = [ insizes(:)', NGlobStates ];
   nested_outsizes = [ outsizes(:)', 1, NGlobStates];
   nested_intypes = [ intypes(:)', ORTD.DATATYPE_FLOAT];
   nested_outtypes = [ outtypes(:)', ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT ];

   parlist = new_irparam_elemet_ivec(parlist, nested_insizes, 10); 
   parlist = new_irparam_elemet_ivec(parlist, nested_outsizes, 11);  // datasigs, active_state, x_global
   parlist = new_irparam_elemet_ivec(parlist, nested_intypes, 12); 
   parlist = new_irparam_elemet_ivec(parlist, nested_outtypes, 13); 



   // parameters
   parlist = new_irparam_elemet_ivec(parlist, [Nstates, NGlobStates, inittial_state], 20); 
   parlist = new_irparam_elemet_rvec(parlist, [x0_global], 21);  // inittial global state



  // Go through all schematics
  // and set them up
  // finially they are stored within an irpar structure under different irpar ids
  irpar_sim_idcounter = 900;

  for i = 1:Nstates

    // the parameters for the wrapper scilab function
    wrapper_fn_arg = list(i, userdata, nested_fn, state_names_list(i), N_datainports, N_dataoutports, length(ev) );

    // create a nested simulation
    [sim_container_irpar, nested_sim, userdata] = libdyn_setup_sch2(LD_STATEMACHINE_MAIN, insizes=nested_insizes, ...
                           outsizes=nested_outsizes, nested_intypes, nested_outtypes, wrapper_fn_arg); 

    // pack simulations into irpar container with id = 901
    parlist = new_irparam_container(parlist, sim_container_irpar, irpar_sim_idcounter);

    // increase irpar_sim_idcounter so the next simulation gets another id
    irpar_sim_idcounter = irpar_sim_idcounter + 1;
  end



//   // combine ir parameters
   blockparam = combine_irparam(parlist);


  // blockparam.ipar and blockparam.rpar now contain the blocks parameters

  // set-up the nested block



  btype = 15002;
//   [sim,blk] = libdyn_new_blk_generic(sim, ev, btype, [blockparam.ipar], [blockparam.rpar] );
  [sim,blk] = libdyn_new_block(sim, ev, btype, ipar=[blockparam.ipar], rpar=[blockparam.rpar], ...
                       insizes=[insizes(:)'], outsizes=[outsizes(:)', 1, NGlobStates], ...
                       intypes=[intypes(:)'], ...
                       outtypes=[outtypes(:)', ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT]  );


// 
  // add switch and reset input signals
  blocks_inlist = inlist;

  // connect all inputs
  [sim,blk] = libdyn_conn_equation(sim, blk, blocks_inlist );
 
  // connect all outputs
  Noutp = length(outsizes);

  outlist = list();
  for i = 0:(Noutp-1)
    [sim,out] = libdyn_new_oport_hint(sim, blk, i);   // ith port
    outlist(i+1) = out;
  end

  [sim,active_state] = libdyn_new_oport_hint(sim, blk, Noutp);   // the second last port
  [sim,x_global] = libdyn_new_oport_hint(sim, blk, Noutp+1);   // the last port

endfunction



function [sim, out] = ld_nested_exchffile(sim, events, compresult, slot, fname, simnest_name) // PARSEDOCU_BLOCK
//
// %PURPOSE: Online exchange of a nested simulation via loading *[ir].par files
//
// NOTE: May delay realtime behaviour - use during a asynchronous nested simulation
//
// compresult - signal
// slot - signal
// fname - the filename without ".[ir]par"-ending
// simnest_name - the string which referes to the nested simulation (as given to "ld_simnest2")
// 
//

  ifname = fname + ".ipar";
  rfname = fname + ".rpar";

  btype = 15003;
  [sim,blk] = libdyn_new_block(sim, events, btype, [0,0,0, length(ifname), length(rfname), length(simnest_name), ascii(ifname), ascii(rfname), ascii(simnest_name) ], [ ], ...
                   insizes=[1, 1], outsizes=[1], ...
                   intypes=[ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT], ...
                   outtypes=[ORTD.DATATYPE_FLOAT]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(compresult, slot) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction


function [sim, outvec] = ld_survivereset(sim, events, invec, initvec) // PARSEDOCU_BLOCK
// %PURPOSE: Keep stored data through a simulation reset (EXPERIMENTAL FOR NOW)
//
// in *+(vecsize) - input
// out *+(vecsize) - output
// 
// Prior to a simulation reset the input is stored into memory.
// After the reset the data is available again via the output
// Initially the output is set to initvec
// 
//    

  if (length(initvec) < 1) then
    error("length(initvec) < 1 !");
  end

  btype = 15004;	
  ipar = [length(initvec); 0]; rpar = [initvec];

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                                     insizes=[length(initvec)], outsizes=[length(initvec)], ...
                                     intypes=[ ORTD.DATATYPE_FLOAT ], outtypes=[ORTD.DATATYPE_FLOAT] );
 
  // libdyn_conn_equation connects multiple input signals to blocks
  [sim,blk] = libdyn_conn_equation(sim, blk, list( in ) );

  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction







// 
// A higher level for defining online replaceable schematics using callback functions
// 


function [sim, outlist, userdata, replaced] = ld_ReplaceableNest(sim, ev, inlist, trigger_reload, fnlist, insizes, outsizes, intypes, outtypes, simnest_name, irpar_fname, dfeed, userdata) // PARSEDOCU_BLOCK
// %PURPOSE: A higher level for defining online replaceable schematics using callback functions
//
// inlist - input list()
// outlist - output list()
// 
// For an example see modules/nested/demo/online_replacement
//    


function [sim, outlist] = exch_helper_thread(sim, inlist)
      // This superblock will run the evaluation of the experiment in a thread.
      // The superblock describes a sub-simulation, whereby only one step is simulated
      // which is enough to call scilab one signle time


  defaultevents = 0;
    
  inputv = inlist(1);

//   [sim] = ld_printf(sim, defaultevents, inputv, "inputv = ", 10);

  //
  // A resource demanding Scilab calculation
  //
  
  [sim, dummyin] = ld_const(sim, defaultevents, 1);

      [sim, compready]  = ld_const(sim, defaultevents, 1);
//      [sim, result]  = ld_const_vec(sim, defaultevents, 123);
      [sim, result] = ld_constvec(sim, defaultevents, vec=1:10)

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
  
  useruserdata = userdata(1);
  define_fn = userdata(2);
  
//  input1 = inlist(1);
  
  
//   // a zero
//   [sim, zero] = ld_const(sim, ev, 0);

 // printf("Defining a replaceable controller\n");

  if (cntrlN == 1) then  // is this the schematic 2) ?
    [sim, outlist, useruserdata] = define_fn( sim, inlist, schematic_type='spare', useruserdata );

  end
  
  if (cntrlN == 2) then  // is this the schematic 2) ?

    [sim, outlist, useruserdata] = define_fn( sim, inlist, schematic_type='default', useruserdata );

//          // Here the  default controller can be defined
//          [sim, output] = ld_gain(sim, ev, input1, 5); // 
//          [sim] = ld_printf(sim, ev, in=zero, str="This is just a dummy simulation, which can be replaced.", insize=1);
  end

  userdata(1) = useruserdata;



//   outlist = list(zero);
endfunction

  [sim,zero] = ld_const(sim, ev, 0);
  [sim,active_sim] = ld_const(sim, ev, 1);

  // get the callback functions
  define_fn = fnlist(1);


  //
  // Here the controller is nested, which can be replaced online
  //

        [sim, outlist_42342, computation_finished, userdata] = ld_simnest2(sim, ev=[ ev ] , ...
                       inlist, ...
                       insizes, outsizes, ...
                       intypes, ...
                       outtypes, ...
                       nested_fn=replaceable_cntrl_main, Nsimulations=2, dfeed, ...
                       asynchron_simsteps=0, switch_signal=active_sim, ...
                       reset_trigger_signal=zero, userdata=list(userdata, define_fn), ...
                       simnest_name );

//        [sim] = ld_printf(sim, ev, in=outlist(1), str="The nested, replaceable sim returns", insize=1);


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
         
         // The output list() of the nested schematic
         outlist = outlist_42342;
         
         
endfunction








function [sim] = ld_global_memory(sim, events, ident_str, datatype, len, initial_data, visibility, useMutex)  // PARSEDOCU_BLOCK
// 
// %PURPOSE: inittialise a persistent globally shared memory
// 
// ident_str (string) - name of the memory
// datatype - ORTD datatype of the memory (for now only ORTD.DATATYPE_FLOAT)
// len (integer) - number of elements
// initial_data - initial data of the memory
// visibility (string) - 'global', ... (more are following)
// useMutex (integer) - 0 or 1. Use a mutex if you access the memory from different threads
// 
// 

  ident_str = ident_str + '.memory';

  if (visibility == 'global') then
  
  else
    error("Visibility has to be one of global, ... (more are following)");
  end

  btype = 15001 + 4   ;	
  ipar = [0, datatype, len, useMutex, 0,0,0,0, 0,0, length(ident_str), ascii(ident_str) ]; 
  
  if datatype == ORTD.DATATYPE_FLOAT then
    rpar = [ initial_data ];
    if (length(initial_data) ~= len) then
      error("length(initial_data) ~= len");
    end
  else
    rpar = [ ];
    error("datatype is not one of ORTD.DATATYPE_FLOAT, ...");
  end

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                   insizes=[], outsizes=[], ...
                   intypes=[], outtypes=[]  );
 
 // ensure the block is included in the simulation even without any I/O ports
 sim = libdyn_include_block(sim, blk);
 
//   [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim] = ld_write_global_memory(sim, events, data, index, ident_str, datatype, ElementsToWrite)   // PARSEDOCU_BLOCK
// 
// %PURPOSE: Write a portion to a persistent globally shared memory
// 
// Initialises a memory structure which can be refered by an
// identifier. Data is available for read and write access
// accross different state machines as well as accross
// different threads.
// 
// Make sure to only use the memory created by this function in 
// lower level simulations such as nested state machines, etc.
// Access from higher level simulations is possible but should
// be avoided, as the memory can not be freed on destruction.
// 
// data *+(ElementsToWrite) - data
// index * - index to store the data. Starts at 1
// ident_str (string) - name of the memory
// datatype - ORTD datatype of the memory (for now only ORTD.DATATYPE_FLOAT)
// ElementsToWrite (integer) - number of elements to write to the memory
// 
// 

  ident_str = ident_str + '.memory';

  btype = 15001 + 5   ;	
  ipar = [0, datatype, ElementsToWrite, 0, 0,0,0,0, 0,0, length(ident_str), ascii(ident_str) ]; 
  rpar = [];

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                   insizes=[ElementsToWrite , 1], outsizes=[], ...
                   intypes=[ datatype, ORTD.DATATYPE_FLOAT  ], outtypes=[]  );
 
 // ensure the block is included in the simulation even without any I/O ports
 [sim,blk] = libdyn_conn_equation(sim, blk, list(data, index) );
 
 
 
//   [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim, data] = ld_read_global_memory(sim, events, index, ident_str, datatype, ElementsToRead)   // PARSEDOCU_BLOCK
// 
// %PURPOSE: Read a portion from a persistent globally shared memory
// 
// data *+(ElementsToRead) - data
// index * - index to store the data. Starts at 1
// ident_str (string) - name of the memory
// datatype - ORTD datatype of the memory (for now only ORTD.DATATYPE_FLOAT)
// ElementsToRead (integer) - number of elements to read from the memory
// 

  ident_str = ident_str + '.memory';

  btype = 15001 + 6;
  ipar = [0, datatype, ElementsToRead, 0, 0,0,0,0, 0,0, length(ident_str), ascii(ident_str) ]; 
  rpar = [];

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                   insizes=[1], outsizes=[ElementsToRead], ...
                   intypes=[ ORTD.DATATYPE_FLOAT ], outtypes=[datatype*[1]]  );
 
 // ensure the block is included in the simulation even without any I/O ports
 [sim,blk] = libdyn_conn_equation(sim, blk, list(index) );
 
   [sim,data] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction





