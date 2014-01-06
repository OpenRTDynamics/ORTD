// 
// 
// Very basic example to communicate to sub-processes written in any language
// using ld_Proc_shObj, ld_Process_Recv, ld_Process_Send.
// This example starts the sub-process ProcWithUI.pl
// Such a sub-process has also the ability to synchronise its calling ORTD-simulation.
//


// The name of the program
ProgramName = 'ProcIO'; // must be the filename without .sce

//
// To run the generated controller stored in template.[i,r]par, call from a terminal the 
//
// ortd --baserate=1000 --rtmode 1 -s template -i 901 -l 0 --master_tcpport 10000
// 
// If you want to use harder real-time capabilities, run as root: 
// 
// sudo ortd --baserate=1000 --rtmode 1 -s template -i 901 -l 0 --master_tcpport 10000
// 








// The main real-time thread
function [sim, outlist, userdata] = Thread_MainRT(sim, inlist, userdata)
  // This will run in a thread

//   [sim, Tpause] = ld_const(sim, ev, 1/27);  // The sampling time that is constant at 27 Hz in this example
//   [sim, out] = ld_ClockSync(sim, ev, in=Tpause); // synchronise this simulation
// 
//   // print the time interval
//   [sim] = ld_printf(sim, ev, Tpause, "Time interval [s]", 1);

  // save the absolute time into a file
  [sim, time] = ld_clock(sim, ev);
  [sim] = ld_savefile(sim, ev, fname="AbsoluteTime.dat", source=time, vlen=1);

  //
  // Add you own control system here
  //

  // Start a sub-process with that is communicated through standard I/O mechanisms
  [sim] = ld_Proc_shObj(sim, 0, "Process", Visibility=0, executable="./ProcWithUI.pl", OptionList=list("-l") );

  // read data from the process's stdout and sync the simulation to that
  [sim, ExpectedBytes] = ld_constvecInt32(sim, 0, 8);
  [sim, out, NumBytes] = ld_Process_Recv(sim, 0, ObjectIdentifyer="Process", ExpectedBytes, MaxOutsize=8); // sync simulation

  [sim, DisAsm] = ld_DisassembleData(sim, ev, in=out, ...
                         outsizes=[1], ...
                         outtypes=[ ORTD.DATATYPE_FLOAT ] );

  [sim] = ld_printf(sim, ev, DisAsm(1), "The result that was send from the sub-process = ", 1);

  // send something
  // Sender ID
  [sim, SenderID] = ld_const(sim, ev, 1295793); // random number
  [sim, SenderID_int32] = ld_ceilInt32(sim, ev, SenderID);


  // generate a signal
  [sim, Signal ] = ld_play_simple(sim, ev, r=1:1000 );


  // print data
  [sim] = ld_printf(sim, ev, Signal, "A value send to the process = ", 1);

  // make a binary structure
  [sim, Data, NBytes] = ld_ConcateData(sim, ev, ...
                         inlist=list(SenderID_int32, Signal ), insizes=[1,1], ...
                         intypes=[ ORTD.DATATYPE_INT32, ORTD.DATATYPE_FLOAT ] );

  [sim, NBytes__] = ld_constvecInt32(sim, ev, vec=NBytes); // the number of bytes that are actually send is dynamic, but must be smaller or equal to 
  [sim] = ld_Process_Send(sim, 0, SendSize=NBytes__, ObjectIdentifyer="Process", in=Data, insize=NBytes);


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


