thispath = get_absolute_file_path('MotionStim.sce');
cd(thispath);

z = poly(0,'z');


//
// This shows how to set-up two threads: One main thread with 300Hz
// and another one that is triggered every 12th sample by the main
// thread by a frequency division.
//




function [sim, outlist] = stimulation_freq_sche(sim, inlist)
      // This superblock will run the evaluation of the experiment in a thread.
      // The superblock describes a sub-simulation, whereby only one step is simulated
      // which is enough to call scilab one signle time


  defaultevents = 0;
  ev = 0;
    
  motion_downsampled = inlist(1); // only every  300/25'th value is taken from the high frequ original signal


  [sim, zero] = ld_const(sim, ev, 0);

      [sim] = ld_printf(sim, ev, in=zero, str="This is ~300/25Hz", insize=1);


      // split up the whole motion vector
      [sim, MO] = ld_demux(sim, ev, vecsize=24, invec=motion_downsampled);  
      
      yaw1 = MO(1); pitch1 = MO(2); roll1 = MO(3); 
      yaw2 = MO(4); pitch2 = MO(5); roll2 = MO(6); 

      Thu = MO(7); phiu = MO(8); Thf = MO(9); phif = MO(10);
      Thud = MO(11); phiud = MO(12); Thfd = MO(13); phifd = MO(14);
      Thudd = MO(15); phiudd = MO(16); Thfdd = MO(17); phifdd = MO(18);

      [sim] = ld_print_angle(sim, ev, Thu, "Thu");
      [sim] = ld_print_angle(sim, ev, phiu, "phiu");
      [sim] = ld_print_angle(sim, ev, Thf, "Thf");
      [sim] = ld_print_angle(sim, ev, phif, "phif");

  



  // output of schematic
  outlist = list(zero);
endfunction




// This is the main top level schematic
function [sim, outlist] = schematic_fn(sim, inlist)
  // this is the default event
  ev = 0;


      // constants
      [sim, zero] = ld_const(sim, ev, 0);        [sim, one] = ld_const(sim, ev, 1);

      [sim] = ld_printf(sim, ev, in=zero, str="This is ~300Hz", insize=1);

      // A dummy for the vector of measurements
      [sim, motion] = ld_constvec(sim, ev, vec=zeros(24,1) );
    
  
      // include a threaded simulation that is triggered at 25Hz
      // input should be a signal vector of size 10


        //
        // A freq divider
        //

        divisor = 300/25;  // transform 300Hz into 25Hz
        [sim, count ] = ld_modcounter(sim, ev, in=one, initial_count=5, mod=divisor);
        [sim, trigger ] = ld_compare_01(sim, ev, in=count,  thr=divisor-2+0.5);
        trigger_stimulation = trigger;

//       [sim] = ld_printf(sim, ev, in=count, str="count", insize=1);
//        [sim] = ld_printf(sim, ev, in=trigger, str="trigger stimulation", insize=1);

        // a nested simulation that runns asynchronously (in a thread) to the main simulation
        [sim, outlist, computation_finished] = ld_simnest(sim, ev, ...
                              inlist=list(motion), ...
                              insizes=[ 24 ], outsizes=[ 1 ], ...
                              intypes=[ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT], ...
                              fn_list=list(stimulation_freq_sche), ...
                              dfeed=1, asynchron_simsteps=1, ...
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

