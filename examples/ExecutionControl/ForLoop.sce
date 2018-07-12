//
//    Copyright (C) 2011, 2012, 2013, 2014, 2015  Christian Klauer
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

// NOTE: CURRENTLY UNDER DEVELOPMENT


//
// This is an example for "for-loops"
// using the nested module.
// After running this file, have a look into the variable "messages" to
// observed what happened.
//
// To run from a terminal:   $ ortdrun -s ForLoop -l 1
//

SchematicName = 'ForLoop'; // must be the filename without .sce
thispath = get_absolute_file_path(SchematicName+'.sce');
cd(thispath);

z = poly(0,'z');



//
// Set up simulation schematic
//

function [sim, outlist, userdata] = ForLoopFn(sim, inlist, LoopCounter, userdata)
  // input data
  in1 = inlist(1);
  in2 = inlist(2);

  // sample data for the output
  [sim, outdata1] = ld_constvec(sim, 0, vec=[1200]);

  [sim, LoopCounter] = ld_Int32ToFloat(sim, 0, LoopCounter);
  [sim] = ld_printf(sim, 0, LoopCounter, "Loop count active: ", 1);

  // the user defined output signals of this nested simulation
  outlist = list(outdata1);
endfunction


// This is the main top level schematic
function [sim, outlist] = schematic_fn(sim, inlist) 

  // some dummy input to the state machine
  [sim,data1] = ld_const(sim, 0, 1.234);   
  [sim, data2] = ld_constvec(sim, 0, vec=[12,13]);

  // create a modulo counter to repeatedly shift the currently active simulation
  [sim, one] = ld_const(sim, 0, 1);
  [sim, CountTo] = ld_modcounter(sim, 0, in=one, initial_count=1, mod=10);
  [sim, CountTo] = ld_add_ofs(sim, 0, CountTo, 1);
  
  [sim] = ld_printf(sim, 0,  CountTo  , "Number of iterations of the nested simulation", 1);
    
  [sim, CountTo] = ld_ceilInt32(sim, 0, CountTo);

  // set-up the for-loop
  [sim, outlist, userdata] = ld_ForLoopNest(sim, 0, ...
      inlist=list(data1, data2), ..
      insizes=[1,2], outsizes=[1], ... 
      intypes=[ORTD.DATATYPE_FLOAT,ORTD.DATATYPE_FLOAT  ], outtypes=[ORTD.DATATYPE_FLOAT], ...
      ForLoop_fn=ForLoopFn, SimnestName="ForLoopTest", NitSignal=CountTo, list("UserdataTest")  );


  [sim] = ld_printf(sim, 0,  outlist(1)  , "output ", 1);

      
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

// irparam set is complete convert
par = combine_irparam(parlist);

// save vectors to a file
save_irparam(par, SchematicName+'.ipar', SchematicName+'.rpar');

// clear
par.ipar = [];
par.rpar = [];




// optionally execute
messages=unix_g(ORTD.ortd_executable+ ' -s '+SchematicName+' -i 901 -l 100');

//
//// load results
//A = fscanfMat('result.dat');
//
//scf(1);clf;
//plot(A(:,1), 'k');

