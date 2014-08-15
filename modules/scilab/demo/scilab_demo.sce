//
//    Copyright (C) 2010, 2011, 2012, 2013, 2014  Christian Klauer
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
// Simple example for using scilab interface to libdyn
//
// execute within scilab and run "ortd -s simple_demo -i 901 -l 100" within 
// the directory of this file
//
// It will write output data to *dat files
//


thispath = get_absolute_file_path('scilab_demo.sce');
cd(thispath);

//
// Set up simulation schematic
//


// This is the main top level schematic
function [sim, outlist] = schematic_fn(sim, inlist)
   u1 = inlist(1); // Simulation input #1
   
  [sim, u1] = ld_constvec(sim, 0, [0.6,0.5,0,1,-1]);


  function [block]=EmbeddedScilabFn( block, flag )
    // This Scilab function is called during run-time in the embedded
    // Scilab instance
    // 
    // NOTE: Please note that the variables defined outside this
    //       function are typically nor available at run-time, unless
    //       they are copied by using ForwardVars=%t .
    //       This also holds true for self defined Scilab functions!
    //       They can be embedded using par.include_scilab_fns .
    //
    // NOTE: All error messages of runtime errors occuring in this 
    //       function will only be visible in the "stderr" output
    //       of the ortd interpreter. To log these errors, e.g.
    //       add a redirection of the error output, e.g.:
    //
    //       ortd <...>  2> Error.log
    //
    
    function outvec=calc_outputs(in, counter)
      printf("Computing in embedded Scilab...\n");
      outvec=(1:6)';
      outvec(1) = SomeFunction(in(1));
      outvec(2) = cfpar.V(2);
      outvec(3) = counter;
      printf("done\n");
    endfunction

    select flag
      case 1 // only the output flag is available
        in = block.inptr(1);  // inputs
	outvec = calc_outputs(in, block.states.Counter);
	block.outptr(1) = outvec; // write outputs
        block.states.Counter = block.states.Counter + 1; // update a state

      case 4 // init
        // e.g. open data from a file
	printf("init\n");
        block.states.Counter = 0; // initialise a state variable

      case 5 // terminate
	printf("terminate\n");
        // e.g. to close log files

    end
  endfunction
  

  //
  // Embedded Scilab. Run the function scilab_comp_fn defined above for one time step to perform the calibration
  // that is implemented in Scilab.
  //

  par.include_scilab_fns = list(SomeFunction, "SomeFunction"); // Transfer one or more Scilab functions to the embedded Scilab

  LF = char(10);
  par.InitStr = "cfpar.V=" + vec2str([1.2, 1.3, 1.4]) + LF; // This string is executed by the embedded Scilab in advance

  par.scilab_path = "BUILDIN_PATH";  // The path to the Scilab executable

  [sim, out] = ld_scilab4(sim, 0, in=u1, invecsize=5, outvecsize=6, ...
			  comp_fn=EmbeddedScilabFn, ForwardVars=%f, par);


  // Display the output of the Scilab computation
  [sim] = ld_printf(sim, 0, out, "Scilab output", 6);
  
  // output of schematic
  [sim,out] = ld_const(sim, 0, 0);
  outlist = list(out); // Simulation output #1
endfunction

function [out] = SomeFunction(in)
  out = in * 2;
endfunction

function str = vec2str(v)
  LF = char(10);  // line feed char
  str = "[";
  for i=1:(length(v)-1)
    str = str + string(v(i)) + "," + LF;
  end
  str = str + string( v($) ) + "];";
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

