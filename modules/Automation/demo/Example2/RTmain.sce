// 
// This file is part of OpenRTDynamics.sf.net, the Real-Time Dynamics Framework
// 
// A demonstration for automating experiments using the macro ld_AutoOnlineExch_dev2
// --------------------------------------------------------------------------------
// 
// Rev 2. 10.8.15
// 
// 
// This example shows how to automate sequences of experiments including
// their evaluation and/or the ongoing design of control systems. Normal Scilab
// code can be used to automatically perform calibration / controller design
// procedures, while the overall control system is running. Concepts like
// embedded Scilab and on-line replaceable schematics are involved.
// 
// 
// Definitions of terms:
// 
// This example involves the execution of Scilab code to define schematics as
// well as to perform calculations, while the control
// system is running by using Scilab embedded into the ORTD interpreter
// (ld_scilab4). If comments in the code state that a certain part of Scilab
// commands are executed "On-line" or "at runtime" it means these commands will be
// run in the embedded Scilab instance, when the ortd interpreter is running.
// Parts marked by the term "Off-line" are executed when this Scilab-Script
// (RTmain.sce) is executed prior to running the ortd-interpreter.
// 
// The schematic that is responsible for performing e.g. calibration
// or implementing control systems is called "experiment controller".
// Such schematics are defined using the function "ExperimentCntrl" in
// this example. This function may run Off-line (to define an initial 
// experiment controller) as well as On-line to define experiment controllers 
// during runtime. The execution of this function is triggered each time
// an experiment controller decides to finish its execution, which is indicated
// by setting the return-signal "finished" to one. In a next step, typically,
// a replacement for the current experiment controller is defined that is automatically
// loaded.
// 
// By implementing "ExperimentCntrl" using a state machine, complex experiment control
// logic can be implemented. Herein, each state may be used to define new experiment
// controllers potentially based on results (e.g. collected data) of previously 
// active experiment controllers.
// 
// 
// In this example the following steps are performed
// 
// 1) An initial, dummy schematic for the experiment controller is
//    active that finishes executing after 10 execution steps.
// 2) The function "ExperimentCntrl" is called using the embedded 
//    Scilab instance. The state "init" is active and used to define
//    an experiment controller that is supposed to collect measured
//    data.
// 3) The experiment controller is loaded and executed. Collection of data
//    is progressing until NcalibSamples=20 samples are collected.
// 4) Again, he function "ExperimentCntrl" is called using the embedded 
//    Scilab instance to define a next experiment controller. Now, the state "control"
//    is active. The previously collected data is used obtain some parameters 
//    (e.g. a mean value) using normal Scilab commands to design a control system.
// 5) The control system is loaded and executed.
// 6) after some time, the control system decides to perform a re-calibration;
//    Step 2) and the ongoing logic is performed again.
// 
// 
// Please note: All error messages occurring when embedded Scilab code is executed
//              go to stderr of the ortd interpreter. E.g. append something like
//              ortd <...> 2> Errors.log
// 


// The name of the program
ProgramName = 'RTmain'; // must be the filename without .sce
thispath = get_absolute_file_path(ProgramName+'.sce');
cd(thispath);



function [sim] = AutoCalibration(sim, Signal)

  function [sim, finished, outlist, userdata] = ExperimentCntrl(sim, ev, inlist, userdata, CalledOnline)

    // Define parameters. They must be defined once again at this place, because this will also be called at
    // runtime.
    Data_vecsize = 2; NcalibSamples=20;

    if CalledOnline == %t then
      // The contents of this part will be compiled on-line, while the control
      // system is running. The aim is to generate a new compiled schematic for
      // the experiment.
      // Please note: Since this code is only executed on-line, most potential errors 
      // occuring in this part become only visible during runtime.

      printf("Compiling a new control system\n");

      if userdata.isInitialised == %f then
        //
        // State variables can be initialise at this place
        //
        userdata.Acounter = 0;
        userdata.State = "calibration";
    
        userdata.isInitialised = %t; // prevent from initialising the variables once again
      end

      // 
      // Example for a state update: increase the counter
      // 
      userdata.Acounter = userdata.Acounter + 1;

      // Build an info-string
      SchematicInfo = "On-line compiled in iteration #" + string(userdata.Acounter);

      // 
      // Define a new experiment controller schematic depending on the currently active state
      // 

      [sim, zero] = ld_const(sim, ev, 0);
      [sim, one] = ld_const(sim, 0, 1);

      // default output (dummy)
      outlist=list(zero);

      //
      // Here a state-machine is implemented that may be used to implement some automation
      // logic that is executed during runtime using the embedded Scilab interpreter.
      // In this example, a calibration run succeeded by the design/compilation/execution 
      // of a control-system is implemented. The schematics defined in each state are loaded
      // at runtime.
      // 
      select userdata.State
        case "calibration"  // define a controller to perform a calibration experiment

          Signal = inlist(1);
          [sim] = ld_printf(sim, 0, Signal, "Calibration active; collecting data: ", 2);

          // Store the sensor data into a shared memory.
          [sim, Data_vecsize__] = ld_const(sim, ev, Data_vecsize);
          [sim, zero] = ld_const(sim, ev, 0);
          [sim, writeI] = ld_counter(sim, 0, count=Data_vecsize__, reset=zero, resetto=zero, initial=1);

          [sim] = ld_write_global_memory(sim, 0, data=Signal, index=writeI, ...
                                         ident_str="CalibrationData", datatype=ORTD.DATATYPE_FLOAT, ...
                                         ElementsToWrite=Data_vecsize);

          // Wait until a number of time steps has passed, then notify that
          // the experiment has finished by setting "finished" to 1.
          [sim, finished] = ld_steps2(sim, ev, activation_simsteps=NcalibSamples, values=[0,1] );

          [sim, out] = ld_const(sim, ev, 0);
          outlist=list(out);

          // chose the next state to enter when the calibration experiment controller has finished
          userdata.State = "control";

        case "control" // design a controller based on the parameters obtained during the calibration

          // Use the data collected during the experiment that was defined by state "calibration".
          // Data comming out of userdata.InputData is the output ToScilab of "PreScilabRun".
          data = userdata.InputData;
          A=matrix( userdata.InputData , Data_vecsize, NcalibSamples )';
          printf("Got the following data for iteration %d:\n", userdata.Acounter);  disp(A);

          // Perform some calibration tasks...
          Signal1 = A(:,1); mean_S1 = mean(Signal1); stdev_S1 = stdev(Signal1);

          // No real control system at this place -- just one printf to show the current parameters.
          [sim] = ld_printf(sim, 0, zero, "The control system was compiled and is active now. Parameters are: mean_S1="+string(mean_S1)+" stdev_S1="+string(stdev_S1), 1);

          // Do something useful in this demo: subtract to obtained mean values from the Signal
          // S1
          Signal = inlist(1);  [sim, S1] = ld_demux(sim, 0, 2, Signal);
          [sim, S1_minus_ofs] = ld_add_ofs(sim, 0, S1(1), -mean_S1);
          [sim] = ld_printf(sim, 0, S1_minus_ofs, "The offset compensated input signal:" , 1);
          
          // Wait until a number of time steps has passed, then notify that
          // the experiment (control system in this case) has finished.
          [sim, finished] = ld_steps2(sim, 0, activation_simsteps=50-1, values=[0,1] );

          [sim, MaxCount] = ld_const(sim, 0, 50);
          [sim, Counter] = ld_modcounter(sim, 0, in=one, initial_count=0, mod=50+1);
          [sim, CountDown] = ld_add(sim, 0, list(Counter, MaxCount), [ -1, 1 ] );
          [sim] = ld_printf(sim, 0, CountDown, "Recalibration after time-steps:" , 1);

          // next state: recalibrate by going to "init" again after "finished" is set to 1
          userdata.State = "calibration";

      end
    end // CalledOnline == %t

    // When RTmain.sce is executed, this part will be run. It may be used to define an initial experiment in advance to
    // the execution of the whole control system.
    if CalledOnline == %f then
      SchematicInfo = "Off-line compiled";

      // default output (dummy)
      [sim, out] = ld_const(sim, 0, 0);
      outlist=list(out);
      [sim, finished] = ld_steps2(sim, 0, activation_simsteps=10, values=[0,1] );
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

  // Start the experiment
  ThreadPrioStruct.prio1=ORTD.ORTD_RT_NORMALTASK;
  ThreadPrioStruct.prio2=0, ThreadPrioStruct.cpu = -1;

  insizes=[2]; outsizes=[1];
  intypes=[ORTD.DATATYPE_FLOAT]; outtypes=[ORTD.DATATYPE_FLOAT];


  CallbackFns.experiment = ExperimentCntrl;
  CallbackFns.whileComputing = whileComputing_example;
  CallbackFns.PreScilabRun = PreScilabRun;

  // Please note ident_str must be unique.
  userdata = [];
  param.scilab_path = "BUILDIN_PATH";
  [sim, finished, outlist, userdata] = ld_AutoOnlineExch_dev2(sim, 0, inlist=list(Signal), ...
                                                              insizes, outsizes, intypes, outtypes, ... 
                                                              ThreadPrioStruct, CallbackFns, ident_str="AutoCalibDemo", userdata, param);

//   [sim] = ld_printf(sim, 0, finished, "State ", 1);
  
endfunction












// The main real-time thread
function [sim, outlist, userdata] = Thread_MainRT(sim, inlist, userdata)
  // This will become a main-loop running in a thread
  [sim, Tpause] = ld_const(sim, ev, 1/4);  // The sampling time that is constant at 4 Hz in this example
  [sim, out] = ld_ClockSync(sim, ev, in=Tpause); // synchronise this simulation

  // print the time interval
  // [sim] = ld_printf(sim, ev, Tpause, "Time interval [s]", 1);

  // save the absolute time into a file
  [sim, time] = ld_clock(sim, ev);
  // [sim] = ld_savefile(sim, ev, fname="AbsoluteTime.dat", source=time, vlen=1);

  //
  // Add your own control system here
  //

  rand('seed',0);

  [sim,S1] = ld_play_simple(sim, 0, r=rand(1000,1) ); // some noise signal
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


