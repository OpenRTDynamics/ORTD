// 
// 
// This a template for writing real-time applications using OpenRTDynamics
// (openrtdynamics.sf.net)
// 
//
// 


// The name of the program
ProgramName = 'template'; // must be the filename without .sce

//
// To run the generated controller stored in template.[i,r]par, call from a terminal the 
//
// ortd --baserate=1000 --rtmode 1 -s template -i 901 -l 0
// 
// If you want to use harder real-time capabilities, run as root: 
// 
// sudo ortd --baserate=1000 --rtmode 1 -s template -i 901 -l 0
// 






// The main real-time thread
function [sim, outlist, userdata] = Thread_MainRT(sim, inlist, userdata)
  // This will run in a thread
  [sim, Tpause] = ld_const(sim, ev, 1/27);  // The sampling time that is constant at 27 Hz in this example
  [sim, out] = ld_ClockSync(sim, ev, in=Tpause); // synchronise this simulation

  // print the time interval
  [sim] = ld_printf(sim, ev, Tpause, "Time interval [s]", 1);

  // save the absolute time into a file
  [sim, time] = ld_clock(sim, ev);
  [sim] = ld_savefile(sim, ev, fname="AbsoluteTime.dat", source=time, vlen=1);

  //
  // Add you own control system here
  //
  
      // Embedded Scilab. Run the function scilab_comp_fn defined above for one time step to perform the calibration
      // that is implemented in Scilab. The console output of the scilab process will be forwarded
      // to the output of the ortd-interpreter.

      function [block]=EMGFilterRegress( block, flag )
	// This scilab function is called during run-time by starting a
        // a Scilab instance in a separated process.
        //
	// NOTE: Please note that the variables defined outside this
	//       function are typically not available at run-time.
	//       This also holds true for self defined Scilab functions!
        //       For execptions see below.


	function [outvec,counter]=calc(EMG, counter)
//          printf("EMG=\n"); disp(EMG);

          counter = counter + 1; 

          // pack
          outvec(1) = counter;
          outvec(2) = 2;
          outvec(3) = 3;
          outvec(4) = 4;
          outvec(5) = 5;
	endfunction

	select flag
	  case 1 // output calculation
            EMG = block.inptr(1);            // input signals
            counter = block.states.counter;
 
            // calculate something
 	    [outvec, counter] = calc(EMG, counter); 

            block.states.counter = counter; // update states
	    block.outptr(1) = outvec; // output signal

	  case 4 // init
	    printf("init\n");

            block.states.counter = 0;  // init states

	  case 5 // terminate
	    printf("terminate\n");

	  case 10 // configure I/O
	    printf("configure\n");
	    block.invecsize = MaxEMGLen;
	    block.outvecsize = 5;
	end
      endfunction

      // The variable MaxEMGLen will be automagically transfered to and defined in the on-line Scilab instance,
      // because it is used inside the function EMGFilterRegress (automatically detected).
      // However this *only* works for scalar variables by now!
      MaxEMGLen = 100;

      [sim, EMG] = ld_constvec(sim, 0, vec=1:100 );

      [sim, out] = ld_scilab3(sim, 0, in=EMG, comp_fn=EMGFilterRegress, include_scilab_fns=list(), ...
                                      InitStr="", scilab_path="BUILDIN_PATH");
  
  
      [sim] = ld_printf(sim, 0, out, "Scilab out=",  5);
  

  outlist = list();
endfunction




// This is the main top level schematic
function [sim, outlist] = schematic_fn(sim, inlist)  

// 
// Create a thread that runs the control system
// 
   
        ThreadPrioStruct.prio1=ORTD.ORTD_RT_NORMALTASK; // or  ORTD.ORTD_RT_NORMALTASK
        ThreadPrioStruct.prio2=0; // for ORTD.ORTD_RT_REALTIMETASK: 1-99 as described in   man sched_setscheduler
                                  // for ORTD.ORTD_RT_NORMALTASK this is the nice-value (higher value means less priority)
        ThreadPrioStruct.cpu = -1; // The CPU on which the thread will run; -1 dynamically assigns to a CPU, 
                                   // counting of the CPUs starts at 0

        [sim, StartThread] = ld_initimpuls(sim, ev); // triggers your computation only once
        [sim, outlist, computation_finished] = ld_async_simulation(sim, ev, ...
                              inlist=list(), ...
                              insizes=[], outsizes=[], ...
                              intypes=[], outtypes=[], ...
                              nested_fn = Thread_MainRT, ...
                              TriggerSignal=StartThread, name="MainRealtimeThread", ...
                              ThreadPrioStruct, userdata=list() );
       

//    NOTE: for rt_preempt real-time you can use e.g. the following parameters:
// 
//         // Create a RT thread on CPU 0:
//         ThreadPrioStruct.prio1=ORTD.ORTD_RT_REALTIMETASK; // rt_preempt FIFO scheduler
//         ThreadPrioStruct.prio2=50; // Highest priority
//         ThreadPrioStruct.cpu = 0; // CPU 0


   // output of schematic (empty)
   outlist = list();
endfunction

  








//
// Set-up (no detailed understanding necessary)
//

thispath = get_absolute_file_path(ProgramName+'.sce');
cd(thispath);
z = poly(0,'z');

// defile ev
ev = [0]; // main event

// set-up schematic by calling the user defined function "schematic_fn"
insizes = []; outsizes=[];
[sim_container_irpar, sim]=libdyn_setup_schematic(schematic_fn, insizes, outsizes);

// pack the simulation into a irpar container
parlist = new_irparam_set();
parlist = new_irparam_container(parlist, sim_container_irpar, 901); // pack simulations into irpar container with id = 901
par = combine_irparam(parlist); // complete irparam set
save_irparam(par, ProgramName+'.ipar', ProgramName+'.rpar'); // Save the schematic to disk

// clear
par.ipar = []; par.rpar = [];


