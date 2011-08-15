

// Interfacing functions are placed in this place


//function [sim,bid] = libdyn_new_scope(sim, events, str, insize)
//  btype = 10001;
//  str = ascii(str);
//  [sim,bid] = libdyn_new_blk_generic(sim, events, btype, [insize, length(str), str(:)'], []);
//endfunction
//
//function [sim] = ld_scope(sim, events, in, str, insize)
//  [sim,blk] = libdyn_new_scope(sim, events, str, insize);
//  [sim,blk] = libdyn_conn_equation(sim, blk, list(in,0) );
//endfunction



function [sim, outlist, computation_finished] = ld_simnest(sim, ev, switch_signal,reset_signal, inlist, insizes, outsizes, intypes, outtypes, fn_list, dfeed, synchron_simsteps)
// 
// ld_simnest -- create one (or multiple) nested libdyn simulation within a normal libdyn block
// 		 it is possible to switch between them by an special input signal
//
// INPUTS: 
//
// switch_signal: signal used to switch between different nested simulations
// inlist - list( ) of input signals to the block, that will be forwarded to the nested simulation(s)
//
// PARAMETERS:
// 
// ev - events to be forwarded to the nested simulation
// insizes - input ports configuration
// outsizes - output ports configuration
// intypes - 
// outtypes -
// fn_list - list( ) of scilab functions defining sub-schematics
// dfeed - the block containing all sub-schematics is configured with dfeed
// synchron_simsteps - if > 0 synchron_simsteps steps will be simulated in a thread
//                     when finished the result becomes available to the blocks outports
//                     if == 0 the nested simulation runns synchronous to the upper level simulation.
// 
// OUTPUTS:
// 
// outlist - list( ) of output signals
// computation_finished - optional and only meanful if synchron_simsteps > 0 (means async computation)
// 


   parlist = new_irparam_set();

  N1 = length(insizes);
  N2 = length(outsizes);
  N3 = length(intypes);
  N4 = length(outtypes);

  // check for sizes
  // ...
  if (length(inlist) ~= N1) then
    error("length inlist invalid\n");
  end

   parlist = new_irparam_elemet_ivec(parlist, insizes, 10); 
   parlist = new_irparam_elemet_ivec(parlist, outsizes, 11); 
   parlist = new_irparam_elemet_ivec(parlist, intypes, 12); 
   parlist = new_irparam_elemet_ivec(parlist, outtypes, 13); 


  if (length(dfeed) ~= 1) then
    error("dfeed should be of size 1\n");
  end

  if (length(synchron_simsteps) ~= 1) then
    error("synchron_simsteps should be of size 1\n");
  end


  Nsimulations = length(fn_list);
   parlist = new_irparam_elemet_ivec(parlist, [Nsimulations, dfeed, synchron_simsteps], 20); 



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
  [sim,blk] = libdyn_new_blk_generic(sim, ev, btype, [blockparam.ipar], [blockparam.rpar] );
// 
  // add switch and reset input signals
  blocks_inlist = inlist;
  blocks_inlist($+1) = switch_signal;
  blocks_inlist($+1) = reset_signal;

  // connect all inputs
  [sim,blk] = libdyn_conn_equation(sim, blk, blocks_inlist );
 
  // connect all outputs
  Noutp = length(outsizes);

  outlist = list();
  for i = 0:(Noutp-1)
    [sim,out] = libdyn_new_oport_hint(sim, blk, i);   // ith port
    outlist(i+1) = out;
  end

  if (synchron_simsteps > 0) then
    [sim,computation_finished] = libdyn_new_oport_hint(sim, blk, Noutp);   // the last port
  else
    computation_finished = 0; // dummy value
  end

endfunction


