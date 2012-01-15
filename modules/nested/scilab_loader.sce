

// Interfacing functions are placed in this place




function [sim, outlist, computation_finished] = ld_simnest(sim, ev, inlist, insizes, outsizes, intypes, outtypes, fn_list, dfeed, asynchron_simsteps, switch_signal, reset_trigger_signal  ) // PARSEDOCU_BLOCK
// 
// ld_simnest -- create one (or multiple) nested libdyn simulation within a normal libdyn block
// 		 it is possible to switch between them by an special input signal
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

// this is not tested and ld_statemachine can be used instead for non async
function [sim, outlist, computation_finished, userdata] = ld_simnest2(sim, ev, inlist, insizes, outsizes, intypes, outtypes, nested_fn, Nsimulations, dfeed, asynchron_simsteps, switch_signal, reset_trigger_signal, userdata, simnest_name  ) 
// 
// ld_simnest2 -- create one (or multiple) nested libdyn simulation within a normal libdyn block
// 		  it is possible to switch between them by an special input signal
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
// userdata - 
// simnest_name - 
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

// this is not tested and ld_statemachine can be used instead for non async
function [par, userdata] = ld_simnest2_replacement( insizes, outsizes, intypes, outtypes, nested_fn, userdata, N  ) 
// 
// ld_simnest2_replacement -- create schematics that can be used as an online exchangeable simulation
//                            for nested simulations set-up using the ld_nested2 block
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
// N - 
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
    [sim_container_irpar, nested_sim, userdata] = libdyn_setup_sch2(nested_fn, insizes, outsizes, list(i, userdata)); 

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
// ld_statemachine -- create one (or multiple) nested libdyn simulation within a normal libdyn block
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
// Online exchange of a nested simulation via loading *[ir].par files
//
// compresult - 
// slot - 
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

