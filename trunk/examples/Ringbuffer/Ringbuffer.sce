thispath = get_absolute_file_path('Ringbuffer.sce');
cd(thispath);

z = poly(0,'z');


//
// This demonstrates how to set-up two threads, which are completely asynch to
// each other. One thread has a variable sampling time.
//
// Currently there is one Bug: To terminate the application it has to be killed,
// because the 2nd thread does not finish his execution.
//




function [sim, outlist] = stimulation_freq_sche(sim, inlist)
      // This superblock will periodically run at 15Hz.
      //
      // This runns at 15Hz, while the main schematic is only at 2Hz
      // This could be e.g. a fast measurement thread, while data processing
      // is performed block-wise in the main schematic at a lower frequency
      //

  defaultevents = 0;
  ev = 0;
    
  NestedSimInput = inlist(1); // 

  [sim, Time] = ld_clock(sim, ev);

  [sim, zero] = ld_const(sim, ev, 0);

//      [sim] = ld_printf(sim, ev, in=zero, str="This runns at a variable samping time", insize=1);


  [sim] = ld_write_ringbuf(sim, ev, data=Time, ident_str="Ringbuffer1", datatype=ORTD.DATATYPE_FLOAT, ElementsToWrite=1);
  [sim] = ld_printf(sim, ev, Time, "Writing to ringbuf: ", 1);

  // The synchronisation
  [sim, Tpause] = ld_const(sim, ev, 1/15); // 15Hz

  // Set the time interval between the simulation steps
  [sim, out] = ld_synctimer(sim, ev, in=Tpause);


  // output of schematic
  outlist = list();
endfunction




// This is the main top level schematic
function [sim, outlist] = schematic_fn(sim, inlist)
  // this is the default event
  ev = 0;


      // constants
      [sim, zero] = ld_const(sim, ev, 0);        [sim, one] = ld_const(sim, ev, 1);

//      [sim] = ld_printf(sim, ev, in=zero, str="This is ~2Hz", insize=1);

      // A dummy for the vector of measurements
      [sim, motion] = ld_constvec(sim, ev, vec=zeros(1,1) );
    
      // Create the Ringbuffer
      [sim] = ld_ringbuf(sim, ev, ident_str="Ringbuffer1", datatype=ORTD.DATATYPE_FLOAT, len=100, visibility='global');
  

      // include a threaded simulation that is triggered at 25Hz
      // input should be a signal vector of size 10
       [sim, trigger_stimulation] = ld_initimpuls(sim, ev); // triggers your computation only once

        // a nested simulation that runns asynchronously (in a thread) to the main simulation
        [sim, outlist, computation_finished] = ld_simnest(sim, ev, ...
                              inlist=list(motion), ...
                              insizes=[ 1 ], outsizes=[  ], ...
                              intypes=[ORTD.DATATYPE_FLOAT], outtypes=[], ...
                              fn_list=list(stimulation_freq_sche), ...
                              dfeed=1, asynchron_simsteps=2, ...
                              switch_signal=zero, reset_trigger_signal=trigger_stimulation   );


  //
  // Block-wise read out the ringbuffer
  // The number of read values is not fixed and returned by NumRead
  // 
  [sim, DataRead, NumRead] = ld_read_ringbuf(sim, ev, ident_str="Ringbuffer1", datatype=ORTD.DATATYPE_FLOAT, ElementsToRead=10); 

  [sim] = ld_printf(sim, ev, in=NumRead, str="NumRead", insize=1);
  [sim] = ld_printf(sim, ev, in=DataRead, str="DataRead", insize=10);

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
save_irparam(par, 'Ringbuffer.ipar', 'Ringbuffer.rpar');

// clear
par.ipar = [];
par.rpar = [];




//// optionally execute
//messages=unix_g(ORTD.ortd_executable+ ' -s controller -i 901 -l 100');

//
//// load results
//A = fscanfMat('result.dat');
//
//scf(1);clf;
//plot(A(:,1), 'k');

