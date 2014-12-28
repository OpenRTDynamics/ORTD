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
// This is an example for setting up a select/case-structure
// using the nested module.
// After running this file, have a look into the variable "messages" to
// observed what happened.
//


SchematicName = 'SelectCase'; // must be the filename without .sce
thispath = get_absolute_file_path(SchematicName+'.sce');
cd(thispath);

z = poly(0,'z');



//
// Set up simulation schematic
//



//[sim, outlist_inner, userdata] = fn(sim, inlist_inner, Ncase, casename, userdata_nested);


function [sim, outlist, userdata] = SelectCaseFn(sim, inlist, Ncase, casename, userdata)
  // This function is called multiple times -- once to define each case
  // At runtime, all cases will become different nested simulations of 
  // which only one is active a a time. 
  
//  pause;
  printf("Defining case %s (#%d) ...\n", casename, Ncase );
  
  // define names for the first event in the simulation
  events = 0;
  
//  pause;
  
  in1 = inlist(1);
  in2 = inlist(2);
  
  
  // print out some state information
  [sim] = ld_printf(sim, events, in=inlist(1), str="case"+string(casename)+": indata(1)", insize=1);

  // sample data for the output
  [sim, outdata1] = ld_constvec(sim, events, vec=[1200]);

  // The signals "active_state" is used to indicate state switching: A value > 0 means the 
  // the state enumed by "active_state" shall be activated in the next time step.
  // A value less or equal to zero causes the statemachine to stay in its currently active
  // state

  select Ncase
    case 1 // case 1
      [sim] = ld_printf(sim, 0, in1, "Case 1 active: ", 1);

    case 2 // case 2
      [sim] = ld_printf(sim, 0, in1, "Case 2 active: ", 1);

    case 3 // case 3
      [sim] = ld_printf(sim, 0, in1, "Case 3 active: ", 1);
  end

  // the user defined output signals of this nested simulation
  outlist = list(outdata1);
endfunction


// This is the main top level schematic
function [sim, outlist] = schematic_fn(sim, inlist) 

  // some dummy input to the state machine
  [sim,data1] = ld_const(sim, 0, 1.234);   
  [sim, data2] = ld_constvec(sim, 0, vec=[12,13]);

  [sim, one] = ld_const(sim, 0, 1);
  [sim, ActiveSim] = ld_modcounter(sim, 0, in=one, initial_count=1, mod=3);
  [sim, ActiveSim] = ld_ceilInt32(sim, 0, ActiveSim)

  //[sim, outlist, userdata] = ld_CaseSwitchNest(sim, 0, inlist, insizes, outsizes, intypes, outtypes, nested_fn, SimnestName, SelectSignal, CaseNameList, userdata) 

  // set-up three states represented by three nested simulations
  [sim, outlist, userdata] = ld_CaseSwitchNest(sim, 0, ...
      inlist=list(data1, data2), ..
      insizes=[1,2], outsizes=[1], ... 
      intypes=[ORTD.DATATYPE_FLOAT,ORTD.DATATYPE_FLOAT  ], outtypes=[ORTD.DATATYPE_FLOAT], ...
      CaseSwitch_fn=SelectCaseFn, SimnestName="SwitchSelectTest", DirectFeedthrough=%t, SelectSignal=ActiveSim, list("Case1", "Case2", "Case3"), list("UserdataTest")  );

  


      
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

