//
//    Copyright (C) 2010, 2011, 2012  Christian Klauer
//
//    This file is part of OpenRTDynamics, the Real Time Dynamic Toolbox
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
// Example for varying sampling times
//
// This is an example for running simulations in a thread with non-constant
// sampling time. Using the "ld_synctimer" the interval time to the next
// simulation step can be specified by an input sigal to this block.
// Currently the "ld_synctimer" only works for nested simulations.
//
// Run with 
//
// $ sh timed_thread.sh
//


thispath = get_absolute_file_path('udp_receiver.sce');
cd(thispath);


z = poly(0,'z');


//
// Set up simulation schematic
//




function [sim, outlist, userdata] = run_thread_fn(sim, inlist, userdata)
  // This will run once in a thread


  defaultevents = 0;
    
  inputv = inlist(1);
//  [sim] = ld_printf(sim, defaultevents, inputv, "inputv = ", 10);

  //
  // Define non-constant sample times
  //

//  [sim, Tpause] = ld_const(sim, events, 1);
//  [sim,Tpause] = ld_play_simple(sim, events, r=[0.51, 0.52, 0.53, 0.54, 0.55, -1]);
  [sim,Tpause] = ld_play_simple(sim, events, r=[ exp( linspace( -4, -0.5, 40) ), -1  ]);

  // Set the time interval between the simulation steps
  // The following block syncronises the simulation
  // Only one block that syncs a simulation can be included
  // in a simulation.

  [sim, Data, SrcAddr] = ld_UDPSocket_Recv(sim, events, ObjectIdentifyer="aSocket", outsize=20);

//   [sim] = ld_printf(sim, events, out, "out ", 10);

  // demo for disassembling this structure -- just to show how it works
  [sim, DisAsm] = ld_DisassembleData(sim, ev, in=Data, ...
                         outsizes=[1,1,1,1], ...
                         outtypes=[ ORTD.DATATYPE_INT32, ORTD.DATATYPE_INT32, ORTD.DATATYPE_INT32, ORTD.DATATYPE_FLOAT ] );

  [sim, DisAsm(1)] = ld_Int32ToFloat(sim, ev, DisAsm(1) );
  [sim, DisAsm(2)] = ld_Int32ToFloat(sim, ev, DisAsm(2) );
  [sim, DisAsm(3)] = ld_Int32ToFloat(sim, ev, DisAsm(3) );

  [sim] = ld_printf(sim, ev, DisAsm(1), "DisAsm(1) (SenderID)       = ", 1);
  [sim] = ld_printf(sim, ev, DisAsm(2), "DisAsm(2) (Packet Counter) = ", 1);
  [sim] = ld_printf(sim, ev, DisAsm(3), "DisAsm(3) (SourceID)       = ", 1);
  [sim] = ld_printf(sim, ev, DisAsm(4), "DisAsm(4) (Signal)         = ", 1);



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

        // Open an UDP-Port
//         [sim] = ld_UDPSocket_shObj(sim, events, ObjectIdentifyer="aSocket", Visibility=0, hostname="127.0.0.1", UDPPort=10000);
        [sim] = ld_UDPSocket_shObj(sim, events, ObjectIdentifyer="aSocket", Visibility=0, hostname="localhost", UDPPort=10000);

 

        // Create a thread
        ThreadPrioStruct.prio1=ORTD.ORTD_RT_NORMALTASK, ThreadPrioStruct.prio2=0, ThreadPrioStruct.cpu = -1;

        [sim, outlist, computation_finished] = ld_async_simulation(sim, events, ...
                              inlist=list(input), ...
                              insizes=[10], outsizes=[10], ...
                              intypes=[ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT], ...
                              nested_fn = run_thread_fn, ...
                              TriggerSignal=startcalc, name="Thread1", ...
                              ThreadPrioStruct, userdata=list() );

         output1 = outlist(1);
         // computation_finished is one, when finished else zero

       [sim] = ld_printf(sim, events, in=computation_finished, str="computation_finished", insize=1);
       


  // dummy output  
  [sim,x] = ld_const(sim, defaultevents, 1.123525);
  
  // output of schematic
  outlist = list(x);
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

// irparam set is complete, convert to vectors
par = combine_irparam(parlist);

// save vectors to a file
save_irparam(par, 'udp_receiver.ipar', 'udp_receiver.rpar');

// clear
par.ipar = [];
par.rpar = [];





