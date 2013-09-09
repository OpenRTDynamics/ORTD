//
//    Copyright (C) 2010, 2011, 2012, 2013  Christian Klauer
//
//    This file is part of OpenRTDynamics, the Real Time Dynamics Framework
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
// $ sh ReadSensors.sh
//


thispath = get_absolute_file_path('UDPStreamSensors.sce');
cd(thispath);


// send data to:
DestHostname = "141.23.97.42";
DestUDPPort = 10000;


z = poly(0,'z');


//
// Set up simulation schematic
//




exec('../scilab_loader.sce');


function [sim, outlist, userdata] = run_thread_fn(sim, inlist, userdata)
  // This will run in a thread
  defaultevents = 0;
  [sim,one] = ld_const(sim, ev, 1);    

  //
  // Define non-constant sample times
  //

  [sim,Stop] = ld_play_simple(sim, events, r=[ zeros(100, 1); 1  ]);

  // Read Sensors
  ConfigStruct.rateAcc = 100; ConfigStruct.rateGyro = 100; ConfigStruct.rateMagn = 100; ConfigStruct.rateGPS = 0;

  [sim, out, SensorID] = ld_AndroidSensors(sim, events, in=Stop, ConfigStruct); // This synchronises the simulation
  [sim] = ld_printf(sim, events, out, "Sensors: ", 10);

  Signal = out;


  // Open an UDP-Port
  [sim] = ld_UDPSocket_shObj(sim, ev, ObjectIdentifyer="aSocket", Visibility=0, hostname="127.0.0.1", UDPPort=-1);

  // Packet counter, so the order of the network packages can be determined
  [sim, Counter] = ld_modcounter(sim, ev, in=one, initial_count=0, mod=10000)
  [sim, Counter_int32] = ld_ceilInt32(sim, ev, Counter);

  // Sender ID
  [sim, SenderID] = ld_const(sim, ev, 1295793); // random number
  [sim, SenderID_int32] = ld_ceilInt32(sim, ev, SenderID);

  // make a binary structure
  [sim, Data, NBytes] = ld_ConcateData(sim, ev, ...
                         inlist=list(SenderID_int32,   Counter_int32,           SensorID,            Signal ), ...
                         insizes=[1,                        1,                      1,                 10    ], ...
                         intypes=[ ORTD.DATATYPE_INT32, ORTD.DATATYPE_INT32, ORTD.DATATYPE_INT32, ORTD.DATATYPE_FLOAT ] );

  printf("Size of the UDP-packets will be %d bytes.\n", NBytes);

//   // send broadcast to the network 
//   [sim] = ld_UDPSocket_Send(sim, ev, ObjectIdentifyer="aSocket", ...
//                  in=Data, insize=NBytes, intype=ORTD.DATATYPE_BINARY);

  // 
  [sim, NBytes__] = ld_constvecInt32(sim, ev, vec=NBytes); // the number of bytes that are actually send is dynamic, but must be smaller or equal to 
  [sim] = ld_UDPSocket_SendTo(sim, ev, SendSize=NBytes__, ObjectIdentifyer="aSocket", ...
                              hostname=DestHostname, UDPPort=DestUDPPort, in=Data, ...
                              insize=NBytes);


  // output of schematic
  outlist = list();
endfunction




// This is the main top level schematic
function [sim, outlist] = schematic_fn(sim, inlist)  
  events = 0;

        [sim, zero] = ld_const(sim, events, 0);

//        [sim, startcalc] = ld_initimpuls(sim, events); // triggers your computation only once
        [sim, startcalc] = ld_const(sim, events, 1); // triggers your computation during each time step


        // Create a non-RT thread
        ThreadPrioStruct.prio1=ORTD.ORTD_RT_NORMALTASK; // or  ORTD.ORTD_RT_NORMALTASK
        ThreadPrioStruct.prio2=0; // for ORTD.ORTD_RT_REALTIMETASK: 1-99 as described in   man sched_setscheduler
        ThreadPrioStruct.cpu = -1; // -1 means dynamically assign CPU

        [sim, outlist] = ld_async_simulation(sim, events, ...
                              inlist=list(), ...
                              insizes=[], outsizes=[], ...
                              intypes=[], outtypes=[], ...
                              nested_fn = run_thread_fn, ...
                              TriggerSignal=startcalc, name="Thread1", ...
                              ThreadPrioStruct, userdata=list() );
       


  // output of schematic
  outlist = list();
endfunction



  
//
// Set-up
//

// defile events
defaultevents = [0]; // main event

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
save_irparam(par, 'UDPStreamSensors.ipar', 'UDPStreamSensors.rpar');

// clear
par.ipar = [];
par.rpar = [];





