// 
// 
// This a template for writing real-time applications using OpenRTDynamics
// (openrtdynamics.sf.net)
// 
//
// 


// The name of the program
ProgramName = 'RTmain'; // must be the filename without .sce
thispath = get_absolute_file_path(ProgramName+'.sce');
cd(thispath);

//
// To run the generated controller stored in template.[i,r]par, call from a terminal the 
//
// ortd --baserate=1000 --rtmode 1 -s template -i 901 -l 0
// 
// If you want to use harder real-time capabilities, run as root: 
// 
// sudo ortd --baserate=1000 --rtmode 1 -s template -i 901 -l 0
// 





function [sim] = AutoCalibration(sim, Signal)

  Data_vecsize = 2;
  NcalibSamples = 60;

// AccGyro, Ts are measurements


// Automatically perform an experiment and the ongoing evaluation.
// The computation required for the evaluation is performed in a the background
// by means of a thread.
// 
// There are several callback functions that describe:
// 
// experiment_fn: The schematic for performing the experiment, e.g. collecting data
// whileComputing_fn: The schematic that is activated during the computation is active_state
// evaluation_fn: The schematic that performs the evaulation in a thread.
//                One simulation step is performed here
// whileIdle_fn:  The schematic that is active when the procedure finished.

  function [sim, finished, outlist, userdata] = experiment(sim, ev, inlist, userdata)
      // Do the experiment
      
      Signal = inlist(1);
      [sim] = ld_printf(sim, 0, Signal, "Collecting data ... ", 2);

      // Store the sensor data into a shared memory
      [sim, Data_vecsize__] = ld_const(sim, ev, Data_vecsize);
      [sim, zero] = ld_const(sim, ev, 0);
      [sim, writeI] = ld_counter(sim, 0, count=Data_vecsize__, reset=zero, resetto=zero, initial=1);

      [sim] = ld_write_global_memory(sim, 0, data=Signal, index=writeI, ...
                                     ident_str="CalibrationData", datatype=ORTD.DATATYPE_FLOAT, ...
                                     ElementsToWrite=Data_vecsize);

 
      // wait until a number of time steps to be passed, then tell ld_AutoExperiment that
      // the experiment has finished.
      [sim, finished] = ld_steps2(sim, ev, activation_simsteps=NcalibSamples, values=[0,1] );
//       [sim, finished] = ld_play_simple(sim, ev, [ zeros(NcalibSamples, 1); 1 ]);

      [sim, out] = ld_const(sim, ev, 0);
      outlist=list(out);
  endfunction

  function [sim, outlist, userdata] = whileComputing(sim, ev, inlist, userdata)
      // While the computation is running this is called regularly
      [sim, out] = ld_const(sim, ev, 0);
      outlist=list(out);
  endfunction

  function [sim, outlist, userdata] = whileIdle(sim, ev, inlist, userdata)
      AccGyro = inlist(1);

      // Do wait
      [sim, readI] = ld_const(sim, ev, 1); // start at index 1
      [sim, Calibration] = ld_read_global_memory(sim, ev, index=readI, ident_str="CalibrationResult", ...
                                                  datatype=ORTD.DATATYPE_FLOAT, ...
                                                  ElementsToRead=20);

//       [sim] = ld_printf(sim, 0, Calibration, "The calibration result is ", 20);
      
      [sim, out] = ld_const(sim, ev, 0);
      outlist=list(out);
  endfunction

  function [sim, CalibrationOk, userdata] = evaluation(sim, userdata)
      // This superblock will run the evaluation of the experiment in a thread.
      // The superblock describes a sub-simulation, whereby only one step is simulated
      // which is enough to call scilab one signle time
      
      ev = 0;

      // define a Scilab function that performs the calibration
      function [block]=scilab_comp_fn( block, flag )
	// This scilab function is called during run-time
	// NOTE: Please note that the variables defined outside this
	//       function are typically not available at run-time.
	//       This also holds true for self defined Scilab functions!

        Data_vecsize = 2;  
        NcalibSamples = 60;

	function outvec=PerformCalibration(Signal1, Signal2)
          printf("Calibration data:\n");
          disp(Signal1);
          disp(Signal2);

          // pack
          outvec = zeros(20,1);
          outvec(1) = 1;
          outvec(2) = 0;
//           outvec(8:11) = q(:);
	endfunction

	select flag

	  case 1 // output
            // split the sensor data
            data = block.inptr(1);

            Signal1 = zeros(NcalibSamples,1);
            Signal2 = zeros(NcalibSamples,1);

            for i=1:NcalibSamples
              Signal1(i) = data( (i-1)*Data_vecsize + 1 );
              Signal2(i) = data( (i-1)*Data_vecsize + 2 );
            end

 	    outvec = PerformCalibration(Signal1, Signal2);

	    block.outptr(1) = outvec;

	  case 4 // init
	    printf("init\n");

	  case 5 // terminate
	    printf("terminate\n");

	  case 10 // configure
	    printf("configure\n");
	    block.invecsize = NcalibSamples*Data_vecsize;
	    block.outvecsize = 20;

	end
      endfunction
      

      // get the stored sensor data
      [sim, readI] = ld_const(sim, ev, 1); // start at index 1
      [sim, CombinedData] = ld_read_global_memory(sim, ev, index=readI, ident_str="CalibrationData", ...
                                                  datatype=ORTD.DATATYPE_FLOAT, ...
                                                  ElementsToRead=NcalibSamples*Data_vecsize);

      // Embedded Scilab. Run the function scilab_comp_fn defined above for one time step to perform the calibration
      // that is implemented in Scilab.
      [sim, Calibration] = ld_scilab2(sim, 0, in=CombinedData, comp_fn=scilab_comp_fn, include_scilab_fns=list(), scilab_path="BUILDIN_PATH");

      // Print the results
      [sim] = ld_printf(sim, 0, Calibration, "The from Scilab returned values are ", 20);

      // Store the calibration into a shared memory
      [sim, one] = ld_const(sim, ev, 1);
      [sim] = ld_write_global_memory(sim, 0, data=Calibration, index=one, ...
                                     ident_str="CalibrationResult", datatype=ORTD.DATATYPE_FLOAT, ...
                                     ElementsToWrite=20);


      // Tell ld_AutoExperiment that the calibration was successful
      [sim, oneint32] = ld_constvecInt32(sim, 0, vec=1)
      CalibrationOk = oneint32;
  endfunction


  // initialise a global memory for storing the sensor-data for the calibration 
  [sim] = ld_global_memory(sim, ev, ident_str="CalibrationData", ... 
                           datatype=ORTD.DATATYPE_FLOAT, len=NcalibSamples*Data_vecsize, ...
                           initial_data=[zeros(NcalibSamples*Data_vecsize,1)], ... 
                           visibility='global', useMutex=1);

  // initialise a global memory for storing the calibration result
  [sim] = ld_global_memory(sim, ev, ident_str="CalibrationResult", ... 
                           datatype=ORTD.DATATYPE_FLOAT, len=20, ...
                           initial_data=[zeros(20,1)], ... 
                           visibility='global', useMutex=1);

  // Start the experiment
  ThreadPrioStruct.prio1=ORTD.ORTD_RT_NORMALTASK;
  ThreadPrioStruct.prio2=0, ThreadPrioStruct.cpu = -1;

  [sim, finished, outlist] = ld_AutoExperiment(sim, ev, inlist=list(Signal), insizes=[2], outsizes=[1], ...
                                     intypes=[ORTD.DATATYPE_FLOAT] , outtypes=[ORTD.DATATYPE_FLOAT], ...
                                     ThreadPrioStruct, experiment, whileComputing, evaluation, whileIdle);


  [sim] = ld_printf(sim, 0, finished, "State ", 1);

  q = 0; R = 0; g_s = 0;

endfunction












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


  [sim,S1] = ld_play_simple(sim, 0, r=sin(1:0.1:100) );
  [sim,S2] = ld_play_simple(sim, 0, r=1+sin(1:0.1:100) );
  [sim, Signal] = ld_mux(sim, 0, 2, list(S1, S2) );
  [sim] = AutoCalibration(sim, Signal);

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


