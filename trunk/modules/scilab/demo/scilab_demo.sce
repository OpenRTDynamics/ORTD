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
// Simple example for using scilab interface to libdyn
//
// execute within scilab and run "libdyn_generic_exec -s simple_demo -i 901 -l 100" within 
// the directory of this file
//
// It will write output data to *dat files
//


thispath = get_absolute_file_path('scilab_demo.sce');
cd(thispath);


z = poly(0,'z');


//
// Set up simulation schematic
//

// 




// This is the main top level schematic
function [sim, outlist] = schematic_fn(sim, inlist)
   u1 = inlist(1); // Simulation input #1
   
  [sim, u1] = ld_constvec(sim, defaultevents, [0.6,0.5,0,1,-1]);
  // 
//  [sim,out] = ld_scilab(sim, defaultevents, in=u1, invecsize=5, outvecsize=5, "", ...
//  "a=1;\n save(\'a.dat\', a);  \n scilab_interf.outvec1 = scilab_interf.invec1 * 2", "", "/usr/local/bin/scilab533");



// The raw interface

//  [sim,out] = ld_scilab(sim, defaultevents, in=u1, invecsize=5, outvecsize=6, "", ...
//  "scilab_interf.outvec1 = [ scilab_interf.invec1 * 2; 9999 ] ", "", "/localhome/arne/openrtdynamics/trunk/scilab-5.3.3/bin/scilab"); // Adapt the path to the scilab5 executable to your needs
//  [sim] = ld_printf(sim, defaultevents, out, "resultvector = ", 6);



  function [block]=sample_comp_fn( block, flag )
    // This scilab function is called during run-time
    // NOTE: Please note that the variables defined outside this
    //       function are typically nor available at run-time.
    //       This also holds true for self defined Scilab functions!
    
    function outvec=calc_outputs()
      printf("...\n");
      outvec=(1:6)';
    endfunction

    select flag
      case 1 // only the output flag is available
	printf("update outputs\n");
  //      outvec = [1:6]';

  //       in = block.inptr(1)(1:3);  // inputs

	outvec = calc_outputs();

	block.outptr(1) = outvec;

      case 4 // init
	printf("init\n");

      case 5 // terminate
	printf("terminate\n");

      case 10 // configure
	printf("configure\n");
	block.invecsize = 5;
	block.outvecsize = 6;

    end
  endfunction
  

// The nicer interface. If BUILDIN_PATH is not found: Do a make clean ; make config; make ; make install on openrtdynamics
  [sim, out] = ld_scilab2(sim, defaultevents, in=u1, comp_fn=sample_comp_fn, include_scilab_fns=list(), scilab_path="BUILDIN_PATH");

  [sim] = ld_printf(sim, 0, out, "Scilab output", 6);
  
  // output of schematic
  [sim,out] = ld_const(sim, defaultevents, 0);
  outlist = list(out); // Simulation output #1
endfunction


  
//
// Set-up
//

// defile events
defaultevents = [0]; // main event

// set-up schematic by calling the user defined function "schematic_fn"
insizes = [1]; outsizes=[1];
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
save_irparam(par, 'scilab_demo.ipar', 'scilab_demo.rpar');

// clear
par.ipar = [];
par.rpar = [];




// optionally execute
//messages=unix_g(ORTD.ortd_executable+ ' -s scilab_demo -i 901 -l 1');


// load results
//A = fscanfMat('result.dat');

//scf(1);clf;
//plot(A(:,1), 'k');


