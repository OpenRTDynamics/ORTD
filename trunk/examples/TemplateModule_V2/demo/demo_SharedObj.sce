// 
//     Copyright (C) 2010, 2011, 2012, 2013  Christian Klauer
// 
//     This file is part of OpenRTDynamics, the Real-Time Dynamics Framework
// 
//     OpenRTDynamics is free software: you can redistribute it and/or modify
//     it under the terms of the GNU Lesser General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
// 
//     OpenRTDynamics is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU Lesser General Public License for more details.
// 
//     You should have received a copy of the GNU Lesser General Public License
//     along with OpenRTDynamics.  If not, see <http://www.gnu.org/licenses/>.
// 

SchematicName = 'demo_SharedObj';
thispath = get_absolute_file_path(SchematicName+'.sce');
cd(thispath);

// 
// If you like to automatically compile the computational 
// function into an ortd-plugin uncomment the line below:
// Then you can just run this scilab-script to make the
//  changes to the C-portions visible.
// 

// CompilerMessages=unix_g('cd .. ; make plugin'); disp(CompilerMessages); // compile plugin ortd_plugin.so





z = poly(0,'z');

// 
// Load the interfacing function "ld_TemplateWrite"
// 

exec('../scilab_loader.sce');


//
// Set up simulation schematic
//





function [sim] = ld_Template_shObj(sim, events, ObjectIdentifyer, Visibility) // PARSEDOCU_BLOCK
// ADD SOME DOCUMENTATION HERE, that will be copied to the scilab help
// abs - block
//
// 

// introduce some parameters that are refered to by id's
parameter1 = 12345;
vec = [1,2,3];

   // pack all parameters into a structure "parlist"
   parlist = new_irparam_set();

   parlist = new_irparam_elemet_ivec(parlist, parameter1, 10); // id = 10
//    parlist = new_irparam_elemet_ivec(parlist, vec, 11); // vector of integers (double vectors are similar, replace ivec with rvec)
//    parlist = new_irparam_elemet_ivec(parlist, ascii(str), 12); // id = 12; A string parameter

   p = combine_irparam(parlist); // convert to two vectors of integers and floating point values respectively

// Set-up the block parameters. There are no I/O ports
  Uipar = [ p.ipar ];
  Urpar = [ p.rpar ];
  btype = 999911111 + 10; // Reference to the block's type (computational function). Use the same id you are giving via the "libdyn_compfnlist_add" C-function

  [sim] =  libdyn_CreateSharedObjBlk(sim, btype, ObjectIdentifyer, Visibility, Uipar, Urpar); 
endfunction




// This is the main top level schematic
function [sim, outlist] = schematic_fn(sim, inlist)
  // this is the default event
  ev = 0;

  // generate two signals   
  [sim, u1] = ld_play_simple(sim, ev, r=linspace(0,1,100) );
  [sim, u2] = ld_play_simple(sim, ev, r=sin( linspace(0,%pi*6,100) ) );



  [sim] = ld_Template_shObj(sim, ev, ObjectIdentifyer="TestSharedObj", Visibility=2);


//   // print data
//   [sim] = ld_printf(sim, ev, out, "output = ", 1);
//   
//   // save the signal us
//   [sim] = ld_savefile(sim, ev, fname="result.dat", source=out, vlen=1);
  
  // output of schematic
  [sim, out] = ld_const(sim, ev, 0);
  outlist = list(out); // Simulation output #1
endfunction






function [sim] =  libdyn_CreateSharedObjBlk(sim, btype, ObjectIdentifyer, Visibility, Uipar, Urpar)
  
  events = 0;
  insizes=[]; // Input port sizes
  outsizes=[]; // Output port sizes
  dfeed=[1];  // for each output 0 (no df) or 1 (a direct feedthrough to one of the inputs)
  intypes=[]; // datatype for each input port
  outtypes=[]; // datatype for each output port

  blocktype = 2; // 1-BLOCKTYPE_DYNAMIC (if block uses states), 2-BLOCKTYPE_STATIC (if there is only a static relationship between in- and output)


  // Create the block
   [sim, blk] = libdyn_CreateBlockAutoConfig(sim, events, btype, blocktype, Uipar, Urpar, insizes, outsizes, intypes, outtypes, dfeed, ObjectIdentifyer, Visibility);
//   [sim, blk] = libdyn_CreateBlockAutoConfig(sim, events, btype, blocktype, Uipar, Urpar, insizes, outsizes, intypes, outtypes, dfeed, 'test');

  // ensure the block is included in the simulation even without any I/O ports
 sim = libdyn_include_block(sim, blk);
  
  // end new fn (sim)
endfunction




function [sim, blk] = libdyn_CreateBlockAutoConfig(sim, events, btype, blocktype, Uipar, Urpar, insizes, outsizes, intypes, outtypes, dfeed, varargin)
  [lhs,rhs]=argn(0);
// 
// Create a I/O Configuration for the block that can be read out by the libdyn_AutoConfigureBlock() - C function
// during block's configuration
// 

  // ObjectIdentifyer


  if length(insizes) ~= length(intypes) then
    error("length(insizes) ~= length(intypes)");
  end
  if length(outsizes) ~= length(outtypes) then
    error("length(outsizes) ~= length(outtypes)");
  end

  

  param = [blocktype];

  parlist = new_irparam_set();

   parlist = new_irparam_elemet_ivec(parlist, insizes, 10); 
   parlist = new_irparam_elemet_ivec(parlist, outsizes, 11); 
   parlist = new_irparam_elemet_ivec(parlist, intypes, 12); 
   parlist = new_irparam_elemet_ivec(parlist, outtypes, 13); 
   parlist = new_irparam_elemet_ivec(parlist, dfeed, 14); 
   parlist = new_irparam_elemet_ivec(parlist, param, 15); 

   parlist = new_irparam_elemet_ivec(parlist, Uipar, 20); 
   parlist = new_irparam_elemet_rvec(parlist, Urpar, 21); 

   rhs=argn(2);
   if ( rhs > 11 ) then
     if (rhs == 13) then
       ObjectIdentifyer = varargin(1);
       Visibility = varargin(2);
       printf("Defining a Shared Object %s Visibility is %d\n", ObjectIdentifyer, Visibility);
       parlist = new_irparam_elemet_ivec(parlist, ascii(ObjectIdentifyer), 30); 
       parlist = new_irparam_elemet_ivec(parlist, Visibility, 31);     
     end
     if (rhs == 12) then
       ObjectIdentifyer = varargin(1);
       printf("Accessing a Shared Object %s\n", ObjectIdentifyer);
       parlist = new_irparam_elemet_ivec(parlist, ascii(ObjectIdentifyer), 30);        
     end
   end

   
   blockparam = combine_irparam(parlist);

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar=[ blockparam.ipar  ], rpar=[ blockparam.rpar ], ...
                  insizes, outsizes, ...
                  intypes, outtypes );
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
save_irparam(par, SchematicName+'.ipar', SchematicName+'.rpar');

// clear
par.ipar = [];
par.rpar = [];



// 
// // optionally execute 
// // Hereby, ORTD.ortd_executable must be the filename of the "libdyn_generic_exec_scilab"-executable
// messages=unix_g(ORTD.ortd_executable+ ' -s '+SchematicName+' -i 901 -l 100'); // simulate 100 samples
// 
// 
// // load results
// A = fscanfMat('result.dat');
// 
// // and show them
// scf(1);clf;
// plot(A(:,1), 'k');
// 
