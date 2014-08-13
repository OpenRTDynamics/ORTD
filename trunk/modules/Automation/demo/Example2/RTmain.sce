// 
// A demonstration for automating experiments.
// 
// 
// 
// 
// embedded 
// 


// The name of the program
ProgramName = 'RTmain'; // must be the filename without .sce
thispath = get_absolute_file_path(ProgramName+'.sce');
cd(thispath);



function [sim] = AutoCalibration(sim, Signal)

  function [sim, finished, outlist, userdata] = experiment_example(sim, ev, inlist, userdata, CalledOnline)

    // Define parameters. They must be defined once again at this place, because this will also be called at
    // runtime.
    Data_vecsize = 2; NcalibSamples=20;

    if CalledOnline == %t then
      // The contents of this part will be compiled on-line, while the control
      // system is running. The aim is to generate a new compiled schematic for
      // the experiment.
      // Please note: Since this code is only executed on-line, most potential errors 
      // occuring in this part become only visible during runtime.

      if userdata.isInitialised == %f then
        //
        // State variables can be initialise at this place
        //
	userdata.Acounter = 0;
        userdata.State = "init";
    
	userdata.isInitialised = %t; // prevent from initialising the variables once again
      end

      // 
      // Example for a state update: increase the counter
      // 
      userdata.Acounter = userdata.Acounter + 1;

      // Build an info-string
      SchematicInfo = "On-line compiled in iteration #" + string(userdata.Acounter);

      // 
      // Define a new schematic that is used to replace experiment-controller
      // 

      // default output (dummy)
      [sim, out] = ld_const(sim, ev, 0);
      outlist=list(out);

      //
      // Here a state-machine is implemented that may be used to implement some automation
      // logic, e.g. a calibration run in a first step and a successive compilation of a control-
      // system that is loaded at runtime.
      // 
      select userdata.State
	case "init"  // define a controller to perform a calibration experiment

	  Signal = inlist(1);
	  [sim] = ld_printf(sim, 0, Signal, "Collecting data ... " + SchematicInfo, 2);

	  // Store the sensor data into a shared memory
	  [sim, Data_vecsize__] = ld_const(sim, ev, Data_vecsize);
	  [sim, zero] = ld_const(sim, ev, 0);
	  [sim, writeI] = ld_counter(sim, 0, count=Data_vecsize__, reset=zero, resetto=zero, initial=1);

	  [sim] = ld_write_global_memory(sim, 0, data=Signal, index=writeI, ...
					ident_str="CalibrationData", datatype=ORTD.DATATYPE_FLOAT, ...
					ElementsToWrite=Data_vecsize);

	  // wait until a number of time steps to be passed, then tell that
	  // the experiment has finished by setting "finished" to 1.
	  [sim, finished] = ld_steps2(sim, ev, activation_simsteps=NcalibSamples, values=[0,1] );


	  [sim] = ld_printf(sim, 0, finished, "Collecting data ..., finished? " , 1);

	  [sim, out] = ld_const(sim, ev, 0);
	  outlist=list(out);

          // chose the next state, which would define a control system
	  userdata.State = "control";

	case "control" // define a new schematic based on the parameters obtained during the calibration

          // use the data collected during the experiment that was defined by state "init"
	  data = userdata.InputData;
	  A=matrix( userdata.InputData , Data_vecsize, NcalibSamples )';
	  printf("Got the following data for iteration %d:\n", userdata.Acounter);  disp(A);

          // Perform some calibration tasks...
          Signal1 = A(:,1); mean_S1 = mean(Signal1);

	  // wait until a number of time steps to be passed, then tell that
	  // the experiment (control system in this case) has finished.
	  [sim, finished] = ld_steps2(sim, ev, activation_simsteps=50, values=[0,1] );

          // No real control system at this place -- just one printf to show the current parameters.
	  [sim] = ld_printf(sim, 0, finished, "Some calibrated control system is active. The previously obtained parameters are: mean_S1="+string(mean_S1) , 1);

          // next state: recalibrate by going to "init" again after "finished" is set to 1
	  userdata.State = "init";
      end
    end // CalledOnline == %t

    // When RTmain.sce is executed, this part will be run. It may be used to define an initial experiment in advance to
    // the execution of the whole control system.
    if CalledOnline == %f then
      SchematicInfo = "Off-line compiled";

      // default output (dummy)
      [sim, out] = ld_const(sim, ev, 0);
      outlist=list(out);
      [sim, finished] = ld_steps2(sim, ev, activation_simsteps=10, values=[0,1] );
    end
  endfunction




  function [sim, outlist, HoldState, userdata] = whileComputing_example(sim, ev, inlist, CalibrationReturnVal, computation_finished, par);

	[sim, HoldState] = ld_const(sim, 0, 0);

	[sim] = ld_printf(sim, 0, HoldState, "calculating ... " , 1);

	// While the computation is running this is called regularly
	[sim, out] = ld_const(sim, ev, 0);
	outlist=list(out);
  endfunction


  function [sim, ToScilab, userdata] = PreScilabRun(sim, ev, par)
	userdata = par.userdata;

	// get the stored sensor data
	[sim, readI] = ld_const(sim, 0, 1); // start at index 1
	[sim, ToScilab] = ld_read_global_memory(sim, 0, index=readI, ident_str="CalibrationData", ...
						    datatype=ORTD.DATATYPE_FLOAT, ...
						    ElementsToRead=NcalibSamples*Data_vecsize);
  endfunction


  Data_vecsize = 2;
  NcalibSamples = 20;


  // initialise a global memory for storing the sensor-data for the calibration 
  [sim] = ld_global_memory(sim, ev, ident_str="CalibrationData", ... 
                           datatype=ORTD.DATATYPE_FLOAT, len=NcalibSamples*Data_vecsize, ...
                           initial_data=[zeros(NcalibSamples*Data_vecsize,1)], ... 
                           visibility='global', useMutex=1);

//   // initialise a global memory for storing the calibration result
//   [sim] = ld_global_memory(sim, ev, ident_str="CalibrationResult", ... 
//                            datatype=ORTD.DATATYPE_FLOAT, len=20, ...
//                            initial_data=[zeros(20,1)], ... 
//                            visibility='global', useMutex=1);



  // Start the experiment
  ThreadPrioStruct.prio1=ORTD.ORTD_RT_NORMALTASK;
  ThreadPrioStruct.prio2=0, ThreadPrioStruct.cpu = -1;

  insizes=[2]; outsizes=[1];
  intypes=[ORTD.DATATYPE_FLOAT]; outtypes=[ORTD.DATATYPE_FLOAT];


  CallbackFns.experiment = experiment_example;
  CallbackFns.whileComputing = whileComputing_example;
  CallbackFns.PreScilabRun = PreScilabRun;

//
// 
// 
// 
  userdata = [];
  [sim, finished, outlist, userdata] = ld_AutoOnlineExch_dev(sim, 0, inlist=list(Signal), ...
                                                             insizes, outsizes, intypes, outtypes, ... 
                                                             ThreadPrioStruct, CallbackFns, ident_str="AutoCalibDemo", userdata);

//   [sim] = ld_printf(sim, 0, finished, "State ", 1);
  
endfunction












// The main real-time thread
function [sim, outlist, userdata] = Thread_MainRT(sim, inlist, userdata)
  // This will run in a thread
  [sim, Tpause] = ld_const(sim, ev, 1/7);  // The sampling time that is constant at 7 Hz in this example
  [sim, out] = ld_ClockSync(sim, ev, in=Tpause); // synchronise this simulation

  // print the time interval
//   [sim] = ld_printf(sim, ev, Tpause, "Time interval [s]", 1);

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
   
        ThreadPrioStruct.prio1=ORTD.ORTD_RT_NORMALTASK; // or  ORTD.ORTD_RT_REALTIMETASK
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


