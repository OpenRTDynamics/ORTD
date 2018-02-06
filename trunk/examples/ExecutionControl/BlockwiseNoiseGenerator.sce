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
// To run from a terminal:   $ ortdrun -s BlockwiseNoiseGenerator -l 1
//

SchematicName = 'BlockwiseNoiseGenerator'; // must be the filename without .sce
thispath = get_absolute_file_path(SchematicName+'.sce');
cd(thispath);

z = poly(0,'z');



//
// Set up simulation schematic
//



//[sim, outlist_inner, userdata] = fn(sim, inlist_inner, Ncase, casename, userdata_nested);


function [sim, outlist, userdata] = LoopFn(sim, inlist, LoopCounter, userdata)
  AmpModulationFac = inlist(1);
  
  [sim, AmpModulationFac_LP] = ld_ztf(sim, 0, AmpModulationFac, (1-0.7)/(z-0.7) );
  
  // Generate band-limited noise
  [sim, Rand] = ld_Random(sim, 0, Method=0, Seed=1);
  [sim, Rand] = ld_mult(sim, 0, list(Rand, AmpModulationFac_LP), [ 0,0 ] )
  [sim, Noise] = ld_ztf(sim, 0, Rand, (1-0.9)/(z-0.9) );

  // Write into a vector  
  [sim,WriteIndex] = ld_add_ofsInt32(sim, 0, in=LoopCounter, ofs=1);

  [sim, WriteIndexFloat] = ld_Int32ToFloat(sim, 0, WriteIndex);
  //[sim] = ld_printf(sim, 0, WriteIndexFloat, "WriteIndex ", 1);
  
  // [sim, out] = ld_collectValues(sim, events, in, WriteIndex, memorysize, DefaultVal, inVecsize )
  [sim, CollectedData] = ld_collectValues(sim, 0, in=Noise, WriteIndex, memorysize=10, DefaultVal=0, inVecsize=1);
  //[sim] = ld_printf(sim, 0,  CollectedData  , "CollectedData ", 10);

  // the user defined output signals of this nested simulation
  outlist = list(CollectedData);
endfunction


// This is the main top level schematic
function [sim, outlist] = schematic_fn(sim, inlist) 

  // some dummy input to the state machine
  [sim, NoiseAmp] = ld_play_simple(sim, 0, [ 0, 1, 2, 3, 4, 5, 6, 5, 4, 3, 2, 1, 0 ] );

  // create a modulo counter to repeatedly shift the currently active simulation
  [sim, NSamples] = ld_play_simple(sim, 0, [10,10]);

  
 // [sim] = ld_printf(sim, 0,  NSamples  , "Number of iterations of the nested simulation", 1);
    
  [sim, NSamples_i32] = ld_ceilInt32(sim, 0, NSamples);

  // This for loop generates 10 samples (modulated by NSamples) of noise for each step performed in the main simulation;
  // date is stored in a vector of size 10
  [sim, outlist, userdata] = ld_ForLoopNest2(sim, 0, ...
      inlist=list(NoiseAmp), ..
      insizes=[1], outsizes=[10], ... 
      intypes=[ORTD.DATATYPE_FLOAT  ], outtypes=[ORTD.DATATYPE_FLOAT], ...
      ForLoop_fn=LoopFn, SimnestName="NoiseGeneratorLoop", ResetAfterLoop=0, NitSignal=NSamples_i32, list()  );

  BlockwiseNoise = outlist(1);

  [sim] = ld_printf(sim, 0,  NSamples  , "Filter produces NSamples = ", 1);
  [sim] = ld_printf(sim, 0,  BlockwiseNoise  , "BlockwiseNoise ", 10);

  [sim] = ld_savefile(sim, 0, fname="BlockwiseNoise.dat", source=BlockwiseNoise, vlen=10);
      
  // output of schematic
  outlist = list(); 
endfunction


  
// Print results

try
  A=fscanfMat('BlockwiseNoise.dat')';
  
  NoiseSignal = A(:);
  
  scf(1); clf;
  plot(NoiseSignal);

catch
  printf("No results to display; run the ortd simulation at first.");    
end







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

