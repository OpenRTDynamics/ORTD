//
//    Copyright (C) 2010, 2011, 2012, 2013  Christian Klauer
//
//    This file is part of OpenRTDynamics, the Real-Time Dynamics Framework
//
//    OpenRTDynamics is free software: you can redistribute it and/or modify
//    it under the terms of the GNU Lesser General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    OpenRTDynamics is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public License
//    along with OpenRTDynamics.  If not, see <http://www.gnu.org/licenses/>.
//

// 
// Test the real-time capabilities of the OS
// by running two threads at 500Hz on two different
// cores. One is using real-time priority and the
// other one is just a normal process.
// 



SchematicName = 'TestRealTime'; // must be the filename without .sce
thispath = get_absolute_file_path(SchematicName+'.sce');
cd(thispath);



z = poly(0,'z');

T_a = 0.1;

//
// Set up simulation schematic
//




function [sim, outlist, userdata] = run_thread_fn_rt(sim, inlist, userdata)
  // This will run in a thread
  [sim, Tpause] = ld_const(sim, events, 1/500);
  [sim, out] = ld_ClockSync(sim, events, in=Tpause);

//   [sim] = ld_printf(sim, events, Tpause, "Time interval [s]", 1);
  [sim, time] = ld_clock(sim, ev);
  [sim] = ld_savefile(sim, ev, fname="time500_rt.dat", source=time, vlen=1);



  [sim, result] = ld_constvec(sim, defaultevents, 1:10);
  // output of schematic
  outlist = list(result);
endfunction


function [sim, outlist, userdata] = run_thread_fn_nonrt(sim, inlist, userdata)
  // This will run in a thread
  [sim, Tpause] = ld_const(sim, events, 1/500);
  [sim, out] = ld_ClockSync(sim, events, in=Tpause);

//   [sim] = ld_printf(sim, events, Tpause, "Time interval [s]", 1);
  [sim, time] = ld_clock(sim, ev);
  [sim] = ld_savefile(sim, ev, fname="time500_nonrt.dat", source=time, vlen=1);



  [sim, result] = ld_constvec(sim, defaultevents, 1:10);
  // output of schematic
  outlist = list(result);
endfunction





// This is the main top level schematic
function [sim, outlist] = schematic_fn(sim, inlist)
  [sim,u2] = ld_const(sim, defaultevents, 2);
  
//  [sim,switch] = ld_const(sim, defaultevents, 0.76);
  [sim,reset] = ld_const(sim, defaultevents, 1.23);
  
  events = defaultevents;

       // input should be a signal vector of size 10
        [sim, input] = ld_constvec(sim, events, vec=1:10)

        [sim, zero] = ld_const(sim, events, 0);

//        [sim, startcalc] = ld_initimpuls(sim, events); // triggers your computation only once
        [sim, startcalc] = ld_const(sim, events, 1); // triggers your computation during each time step

        // Create a RT thread
        ThreadPrioStruct.prio1=ORTD.ORTD_RT_REALTIMETASK; // or  ORTD.ORTD_RT_NORMALTASK
        ThreadPrioStruct.prio2=50; // for ORTD.ORTD_RT_REALTIMETASK: 1-99 as described in   man sched_setscheduler
        ThreadPrioStruct.cpu = 0; // -1 means dynamically assign CPU

        [sim, outlist, computation_finished] = ld_async_simulation(sim, events, ...
                              inlist=list(input), ...
                              insizes=[10], outsizes=[10], ...
                              intypes=[ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT], ...
                              nested_fn = run_thread_fn_rt, ...
                              TriggerSignal=startcalc, name="Thread1", ...
                              ThreadPrioStruct, userdata=list() );

        // Create a non-RT thread
        ThreadPrioStruct.prio1=ORTD.ORTD_RT_NORMALTASK; // or  ORTD.ORTD_RT_NORMALTASK
        ThreadPrioStruct.prio2=0; // for ORTD.ORTD_RT_REALTIMETASK: 1-99 as described in   man sched_setscheduler
        ThreadPrioStruct.cpu = 1; // -1 means dynamically assign CPU

        [sim, outlist, computation_finished] = ld_async_simulation(sim, events, ...
                              inlist=list(input), ...
                              insizes=[10], outsizes=[10], ...
                              intypes=[ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT], ...
                              nested_fn = run_thread_fn_nonrt, ...
                              TriggerSignal=startcalc, name="Thread2", ...
                              ThreadPrioStruct, userdata=list() );
       


//   // dummy output  
//   [sim,x] = ld_const(sim, defaultevents, 1.123525);
//   
//   // output of schematic
   outlist = list();
endfunction



  
//
// Set-up
//

// defile events
defaultevents = [0]; // main event
ev = 0;

// set-up schematic by calling the user defined function "schematic_fn"
insizes = []; outsizes=[];
[sim_container_irpar, sim]=libdyn_setup_schematic(schematic_fn, insizes, outsizes);



//
// Save the schematic to disk (possibly with other ones or other irpar elements)
//

parlist = new_irparam_set();

// pack simulations into irpar container with id = 901
parlist = new_irparam_container(parlist, sim_container_irpar, 901);

// irparam set is complete, convert to vectors
par = combine_irparam(parlist);

// save vectors to a file
save_irparam(par, SchematicName+'.ipar', SchematicName+'.rpar');

// clear
par.ipar = [];
par.rpar = [];



// eval
try
  t_rt = fscanfMat('time500_rt.dat.finished');
  t_nonrt = fscanfMat('time500_nonrt.dat.finished');
//  t_rt = fscanfMat('time500_rt.dat');
//  t_nonrt = fscanfMat('time500_nonrt.dat');

  Dt_rt = diff(t_rt) * 1000;
  Dt_nonrt = diff(t_nonrt) * 1000;

  std_rt = stdev(Dt_rt);
  std_nonrt = stdev(Dt_nonrt);

  scf(1); clf;
  subplot(211); plot(Dt_rt, 'k'); plot(0,0,'+');    xtitle('stdev='+string(std_rt) +' [ms]', '', 'Sample Time rt - [ms]');
  subplot(212); plot(Dt_nonrt, 'k'); plot(0,0,'+');  xtitle('stdev='+string(std_nonrt) +' [ms]', '', 'Sample Time no rt - [ms]');

  xs2pdf(1, 'Result_intervals.pdf');
  
  
  scf(2); clf;  x=linspace(1, 3, 300);
  subplot(211); histplot(x, Dt_rt);   xtitle('', '[ms]', 'occurrences rt; normalised');
  subplot(212); histplot(x, Dt_nonrt);  xtitle('', '[ms]', 'occurrences no rt; normalised');
  
  xs2pdf(2, 'Result_histplot.pdf');
  
catch
  printf("Run the test with # sudo sh TestRealTime.sh and abort after some time using Cntrl-C\nThen run this script again.");
end


