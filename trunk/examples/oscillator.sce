//
// An example for OpenRTDynamics (openrtdynamics.sf.net) implementing an oscillator.
// 
// This example shows the basics of ORTD: How blocks are connected using signal variables,
// feedback loops and structures similar to superblocks, that are representated by Scilab
// functions.
// 
// Because this script automatically runs the simulation and presents the results,
// it also shows how a rapid development and testing process is enabled by the 
// ORTD-framwork.
//
// Execute this within scilab. The shell command "ortd -s oscillator -i 901 -l 1000"
// will be executed to run this example that will write output data to *dat files.
// Finally, the results are plotted by this script.
//
//



SchematicName = 'oscillator'; // must be the filename without .sce
thispath = get_absolute_file_path(SchematicName+'.sce');
cd(thispath);

z = poly(0,'z'); // for defining transfer functions in z-domain

T_a = 0.1; // sample time

//
// Set up simulation schematic
//

// Superblock: A simple oscillator without damping
function [sim, x,v] = oscillator(sim, u)
    // create a feedback signal
    [sim,x_feedback] = libdyn_new_feedback(sim);

        // use this as a normal signal
        [sim,a] = ld_add(sim, ev, list(u, x_feedback), [1, -1]);
        [sim,v] = ld_ztf(sim, ev, a, 1/(z-1) * T_a ); // Integrator approximation
        [sim,x] = ld_ztf(sim, ev, v, 1/(z-1) * T_a ); // Integrator approximation  
    
        // feedback gain
        [sim,x_gain] = ld_gain(sim, ev, x, 0.6);
    
    // close loop x_gain = x_feedback
    [sim] = libdyn_close_loop(sim, x_gain, x_feedback);
endfunction

// Superblock: A more complex oscillator with damping
function [sim, x,v] = damped_oscillator(sim, u)
    // create feedback signals
    [sim,x_feedback] = libdyn_new_feedback(sim);

        [sim,v_feedback] = libdyn_new_feedback(sim);

            // use this as a normal signal
            [sim,a] = ld_add(sim, ev, list(u, x_feedback), [1, -1]);
            [sim,a] = ld_add(sim, ev, list(a, v_feedback), [1, -1]);
    
            [sim,v] = ld_ztf(sim, ev, a, 1/(z-1) * T_a ); // Integrator approximation
    
            // feedback gain
            [sim,v_gain] = ld_gain(sim, ev, v, 0.1);
    
            // close loop v_gain = v_feedback
        [sim] = libdyn_close_loop(sim, v_gain, v_feedback);
    
    
        [sim,x] = ld_ztf(sim, ev, v, 1/(z-1) * T_a ); // Integrator approximation  
    
        // feedback gain
        [sim,x_gain] = ld_gain(sim, ev, x, 0.6);
    
    // close loop x_gain = x_feedback
    [sim] = libdyn_close_loop(sim, x_gain, x_feedback);
endfunction


// This is the main top level schematic
function [sim, outlist] = schematic_fn(sim, inlist)
  [sim,u] = ld_const(sim, ev, 1);
  
  // example of conditional schmeatic generation
  damped = 1; // please choose 1 or 0
  
  if (damped == 1) then
    [sim, x,y] = damped_oscillator(sim, u);
  else
    [sim, x,y] = oscillator(sim, u);  
  end
  
  // print the current value of x
  [sim] = ld_printf(sim, ev, x, "x = ", 1);

  // print a barchart
  [sim, x__] = ld_gain(sim, ev, x, 15);
  [sim] = ld_printfbar(sim, ev, in=x__, str="x ");
  
  // save resulting samples to a file
  [sim, save0] = ld_dumptoiofile(sim, ev, "result.dat", x);
  
  // output of schematic
  outlist = list(x);
endfunction



  
//
// Set-up
//

// default events
ev = [0]; // main event

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
save_irparam(par, 'oscillator.ipar', 'oscillator.rpar');

// clear
par.ipar = [];
par.rpar = [];




// run the executable for 1000 simulation steps by calling an external program (the command "ortd")
// The messages send to the standard output are stored in the variable "messages".
messages=unix_g(ORTD.ortd_executable+ ' -s '+SchematicName+' -i 901 -l 1000');
// disp(messages);  // enable to see the output of the interpreter

// load results
A = fscanfMat('result.dat');

scf(1);clf;
plot(A(:,1), 'k');

