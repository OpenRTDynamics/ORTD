//
//    Copyright (C) 2010, 2011, 2013  Christian Klauer
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
// Example showing how to run nested simulations in a thread (e.g. to run computationally
// intensive tasks without disturbing the real-time properties of a real-time loop).
// Triggering is performed by the upper level schematic through the variable "startcalc".
//
// 
// Note: Also have a look at synchronised_thread_withMemory.sce, showing how to use shared memories
//       combination with thread.
// 
// 
// 
// 


thispath = get_absolute_file_path('synchronised_thread.sce');
cd(thispath);


z = poly(0,'z');
T_a = 0.1;

//
// Set up simulation schematic
//








 
// This is the main top level schematic
function [sim, outlist] = schematic_fn(sim, inlist)
  [sim,u2] = ld_const(sim, defaultevents, 2);
  
//  [sim,switch] = ld_const(sim, defaultevents, 0.76);
  [sim,reset] = ld_const(sim, defaultevents, 1.23);
  
  events = defaultevents;




// 
// The following code creates a thread that runns the function "evaluation_Thread".
// This nested simulation can be triggered by the signal "startcalc" defined below
// 
          // Define dummy input signal to the computation
          [sim, CompInput] = ld_constvec(sim, events, vec=1:10)

//         [sim, startcalc] = ld_initimpuls(sim, 0); // triggers the computation only once 
          [sim, startcalc] = ld_const(sim, events, 1); // triggers your computation during each time step



          // Please note: There is currently a BUG that prevents that the input values are forwarded (as of 25.11.13)
          //              As a work around used the memory mechanism described in synchronised_thread_withMemory.sce.
          [sim] = ld_printf(sim, 0, CompInput, "The input is: ", 10);

	  // Create a thread for performing the computation in the background
	  function [sim, outlist, userdata] = evaluation_Thread(sim, inlist, userdata)
	    // This will run in a thread
	    defaultevents = 0;
	      
	    // input to the calculation
	    inputv = inlist(1);
	    [sim] = ld_printf(sim, 0, inputv, "This is the computation running in a thread. The input is: ", 10);


	    //
	    // A resource demanding calculation can be performed here
	    //
	    
	    [sim, result] = ld_constvec(sim, 0, 1); // dummmy calculation

	    // output of schematic
	    outlist = list(result);
	  endfunction

	  // Thread properties
	  ThreadPrioStruct.prio1=ORTD.ORTD_RT_NORMALTASK;
	  ThreadPrioStruct.prio2=0, ThreadPrioStruct.cpu = -1;

	  [sim, outlist__, computation_finished] = ld_async_simulation(sim, 0, ...
				inlist=list(CompInput), ...
				insizes=[10], outsizes=[1], ...
				intypes=[ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT], ...
				nested_fn = evaluation_Thread, ...
				TriggerSignal=startcalc, name="Computation Thread", ...
				ThreadPrioStruct, userdata=list() );


          // The output of the nested simulation
          ComputationOutput = outlist__(1);

// 
// 
// 
// 



       [sim] = ld_printf(sim, events, in=computation_finished, str="computation_finished", insize=1);
       


  
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
save_irparam(par, 'synchronised_thread.ipar', 'synchronised_thread.rpar');

// clear
par.ipar = [];
par.rpar = [];





