
thispath = get_absolute_file_path('ortd_simulation.sce');
cd(thispath);






// This is the main top level schematic
function [sim, outlist] = schematic_fn(sim, inlist)
  // inputs
  r = inlist(1);
  y = inlist(2);
  
  // events
  sim.ev.defev = [0];
  
  
  [sim] = ld_printf(sim, sim.ev.defev, r, "r :", 1);
  [sim] = ld_printf(sim, sim.ev.defev, y, "y :", 1);

  
  [sim,u] = ld_const(sim, sim.ev.defev, 0.1234);

  
  // output of schematic
  outlist = list(u);
endfunction



  
//
// Set-up
//

// defile events
defaultevents = [0]; // main event

// in- and outsizes
insizes = [1,1]; outsizes=[1];  // same as in the Scicos block interface

// set-up schematic by calling the user defined function "schematic_fn"
[sim_container_irpar, sim]=libdyn_setup_schematic(schematic_fn, insizes, outsizes);



//
// Save the schematic to disk (possibly with other ones or other irpar elements)
//

parlist = new_irparam_set();

// pack simulations into irpar container with id = 900
parlist = new_irparam_container(parlist, sim_container_irpar, 900);

// irparam set is complete convert to vectors
par = combine_irparam(parlist);

// save vectors to a file
save_irparam(par, 'simulation.ipar', 'simulation.rpar');

// clear
par.ipar = [];
par.rpar = [];




// optionally execute
// unix('libdyn_generic_exec -s simulation -i 900 -l 1000');


// load results
//A = fscanfMat('result.dat');

//scf(1);clf;
//plot(A(:,1), 'k');

