thispath = get_absolute_file_path('task2a_fillin.sce');
cd(thispath);

z = poly(0,'z');


//
// Set up simulation schematic
//


// This is the main top level schematic
function [sim, outlist] = schematic_fn(sim, inlist)
  // this is the default event
  ev = 0;

  // generate a disturbed sinus signal
  sinus = sin( linspace(0,%pi*6,100) );
  sinus = sinus + [ zeros(1,9), 10, zeros(1,19), -22, zeros(1,19), 5, zeros(1,9), 10, zeros(1,19), -22, zeros(1,20)     ]
  [sim, u] = ld_play_simple(sim, ev, r=sinus );
   
  /////////////////////////////////////////////////////////////////////////////
  // Please add your implementation of the filter here!
  //
  // Use the functions ld_abs, ld_ztf with H = (z-1)/z, and ld_compare_01
  // Hint: try to open the help for each command, e.g. via "help ld_abs"
  //
  // Be sure to define the signal "peak_detected"
  /////////////////////////////////////////////////////////////////////////////

  peak_detected = u; // uncomment this

  // save the signal "peak_detected" to disk
  [sim] = ld_savefile(sim, ev, fname="result.dat", source=peak_detected, vlen=1);
  
  
  
  // output of schematic
  [sim, out] = ld_const(sim, ev, 0);
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
save_irparam(par, 'controller.ipar', 'controller.rpar');

// clear
par.ipar = [];
par.rpar = [];




// optionally execute
messages=unix_g(ORTD.ortd_executable+ ' -s controller -i 901 -l 100');


// load results
A = fscanfMat('result.dat');

scf(1);clf;
plot(A(:,1), 'k');

