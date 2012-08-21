thispath = get_absolute_file_path('global_memory_demo.sce');
cd(thispath);

z = poly(0,'z');


//
// Set up simulation schematic
//


// This is the main top level schematic
function [sim, outlist] = schematic_fn(sim, inlist)
  // this is the default event
  ev = 0;

  // generate two signals   
  [sim, u1] = ld_play_simple(sim, ev, r=-1:12 );
  [sim, u2] = ld_play_simple(sim, ev, r=sin( linspace(0,%pi*6,100) ) );
   
  //  calculate us = - u1 + 2*u2
  [sim,us] = ld_add(sim, ev, list(u1, u2), [-1, 2] );
  
  // read the memory
  [sim, readI] = ld_const(sim, ev, 1);
  [sim, data] = ld_read_global_memory(sim, ev, index=readI, ident_str="memory", datatype=ORTD.DATATYPE_FLOAT, ElementsToRead=1);
  [sim]  = ld_printf(sim, ev, data, "memory: ", 1);

  // initialise a global memory
  [sim] = ld_global_memory(sim, ev, ident_str="memory", datatype=ORTD.DATATYPE_FLOAT, len=1, initial_data=[2], visibility='global', useMutex=0);

  // write to a global memory
  [sim] = ld_write_global_memory(sim, ev, data=u2, index=u1, ident_str="memory", datatype=ORTD.DATATYPE_FLOAT, ElementsToWrite=1);
  
  // output of schematic
  [sim, out] = ld_const(sim, ev, 0);
  
 // pause;
//   outlist = list(inlist(1)); // Simulation output #1
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
save_irparam(par, 'global_memory.ipar', 'global_memory.rpar');

// clear
par.ipar = [];
par.rpar = [];




