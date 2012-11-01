//
//    Copyright (C) 2010, 2011  Christian Klauer
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
//  NOTE: Plesae do not try to run the simulation (the real-time executable)
//        on your normal PC. There is a direct hardware interface only
//        running on the BeagleBone! To actually run this, move this directory
//        to your BeagleBone; run ./build.sh to compile the C-functions and do sudo ./run.sh
//


//
// Simple example for using the Scilab interface to libdyn (includes example for GPIO access on the BeagleBone)
//
//
// It will write output data to *dat files
//


thispath = get_absolute_file_path('demo_controller.sce');
cd(thispath);


z = poly(0,'z');


function [sim,oututSignal] = sensor_actor_block(sim, events, InputSignal)
// This is the Scilab interface to the hardware accessing C computational function (like S-function in simulink)

  // parameters
  GPIO_PORT_NUM = 1;

  IntegerParam = GPIO_PORT_NUM; // ipar
  FloatParam = [ 1.2, 1.4 ]; // rpar

  // definition of the block
  btype = 100001 + 0; // An id starting at 100001. Each comp. function must have its own
  [sim,blk] = libdyn_new_block(sim, events, btype, [ IntegerParam ], [ FloatParam ], ...
                   insizes=[1], outsizes=[1], ...  // vector sizes for each in/out port.  e.g. for two ports of size 1 use [1,1]
                   intypes=[ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT]  ); // datatype for each port e.g. [ORTD.DATATYPE_FLOAT,ORTD.DATATYPE_FLOAT] for two ports

  // connect the inputs of the comp. fn.
  [sim,blk] = libdyn_conn_equation(sim, blk, list(InputSignal) );  // for more signals use: list(sig1, sig2, ...)

  // connect the outputs of the comp. fn. e.g. the measurements of the hardware
  [sim,oututSignal] = libdyn_new_oport_hint(sim, blk, 0);   // 1th port // for more ports duplicate this line
//  [sim,oututSignal2] = libdyn_new_oport_hint(sim, blk, 1);  // 2th port (not defined in this example)
endfunction



//
// Set up simulation schematic
//


// This is the main top level schematic
function [sim, outlist] = schematic_fn(sim, inlist)
      
//  [sim, actuator] = ld_const(sim, defaultevents, 10);
  // play a pre-defined random signal of length 10000
  [sim, actuator] = ld_play_simple(sim, defaultevents, rand(10000,1)-0.5 ); 

  // the hardware interface as defined above
  [sim,measure] = sensor_actor_block(sim, defaultevents, InputSignal=actuator);  

  // save result to file
  [sim] = ld_savefile(sim, defaultevents, fname="result.dat", source=measure, vlen=1)

  // output of schematic (dummy)
  outlist = list(measure); 
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
save_irparam(par, 'demo_controller.ipar', 'demo_controller.rpar');

// clear
par.ipar = [];
par.rpar = [];

// build the C-Code for the plugin
//unix_g("./build.sh");

messagebox(msg="Plesae do not try to run the simulation (the real-time executable)"+...
               " on your normal PC. There is a direct hardware interface only"+...
               " running on the BeagleBone! To actually run this, move this directory"+...
               " to your BeagleBone; run ./build.sh to compile the C-functions and do sudo ./run.sh", msgboxtitle="", msgboxicon="warning");


// optionally execute
//messages=unix_g(ORTD.ortd_executable+ ' -s demo_controller -i 901 -l 100');


// load results
//A = fscanfMat('result.dat');
//
//scf(1);clf;
//plot(A(:,1), 'k');

