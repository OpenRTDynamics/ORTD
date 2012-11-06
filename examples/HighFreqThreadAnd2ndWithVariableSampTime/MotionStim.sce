thispath = get_absolute_file_path('MotionStim.sce');
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
      // This superblock will run the evaluation of the experiment in a thread.
      // The superblock describes a sub-simulation, whereby only one step is simulated
      // which is enough to call scilab one signle time


  defaultevents = 0;
  ev = 0;
    
  motion_downsampled = inlist(1); // 


  [sim, zero] = ld_const(sim, ev, 0);

      [sim] = ld_printf(sim, ev, in=zero, str="This runns at a variable samping time", insize=1);


      // split up the whole motion vector
      [sim, MO] = ld_demux(sim, ev, vecsize=24, invec=motion_downsampled);  
      


  // The synchronisation
  [sim,Tpause] = ld_play_simple(sim, ev, r= [exp( linspace( -4, -0.5, 40) ) , 0.6]);

  // Set the time interval between the simulation steps
  [sim, out] = ld_synctimer(sim, ev, in=Tpause);
  [sim] = ld_printf(sim, ev, Tpause, "Time interval [s]", 1);


  // output of schematic
  outlist = list(zero);
endfunction




// This is the main top level schematic
function [sim, outlist] = schematic_fn(sim, inlist)
  // this is the default event
  ev = 0;


      // constants
      [sim, zero] = ld_const(sim, ev, 0);        [sim, one] = ld_const(sim, ev, 1);

      [sim] = ld_printf(sim, ev, in=zero, str="This is ~10Hz", insize=1);

      // A dummy for the vector of measurements
      [sim, motion] = ld_constvec(sim, ev, vec=zeros(24,1) );
    
  
      // include a threaded simulation that is triggered at 25Hz
      // input should be a signal vector of size 10



       [sim, trigger_stimulation] = ld_initimpuls(sim, ev); // triggers your computation only once

        // a nested simulation that runns asynchronously (in a thread) to the main simulation
        [sim, outlist, computation_finished] = ld_simnest(sim, ev, ...
                              inlist=list(motion), ...
                              insizes=[ 24 ], outsizes=[ 1 ], ...
                              intypes=[ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT], ...
                              fn_list=list(stimulation_freq_sche), ...
                              dfeed=1, asynchron_simsteps=2, ...
                              switch_signal=zero, reset_trigger_signal=trigger_stimulation   );

         output1 = outlist(1);
         // computation_finished is one, when finished else zero


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
save_irparam(par, 'read_xs2_ArmMotion.ipar', 'read_xs2_ArmMotion.rpar');

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

