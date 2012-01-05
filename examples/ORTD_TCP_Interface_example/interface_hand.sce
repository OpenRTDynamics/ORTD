



//
// ORTD - Schematic definition file
//


thispath = get_absolute_file_path('interface_hand.sce');
cd(thispath);


z = poly(0,'z');

T_a = 0.1;

//
// Set up simulation schematic
//




// This is the main top level schematic
function [sim, outlist] = schematic_fn(sim, inlist)
  output = inlist(1);
    
  //[sim,u] = ld_const(sim, defaultevents, 1);
  
  
  [sim] = ld_stream(sim, defaultevents, output, "output", 10 );  // *** EDIT HERE TO CHANGE SIZES ***
  
  
  
  [sim, control_signal] = ld_parameter(sim, defaultevents, "control_signal", [ 1:10 ]);     // *** EDIT HERE TO CHANGE SIZES ***
  
  


  // output of schematic
  outlist = list(control_signal);
endfunction



  
//
// Set-up
//

// defile events
defaultevents = [0]; // main event

// set-up schematic by calling the user defined function "schematic_fn"
insizes = [10]; outsizes=[10]; // *** EDIT HERE TO CHANGE SIZES ***
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
save_irparam(par, 'interface_hand.ipar', 'interface_hand.rpar');

// clear
par.ipar = [];
par.rpar = [];




// optionally execute
//unix('libdyn_generic_exec -s oscillator -i 901 -l 1000');


// load results
//A = fscanfMat('result.dat');

//scf(1);clf;
//plot(A(:,1), 'k');

