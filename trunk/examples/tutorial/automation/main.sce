thispath = get_absolute_file_path('main.sce');
cd(thispath);

z = poly(0,'z');


function [sim, reached] = reference_reached(sim, ev, r, y, N, eps)
// 
// check wheter the controller reached the constant reference
// 

  [sim, e] = ld_add(sim, ev, list(r,y), list(1,-1) );
  [sim, i1] = ld_ztf(sim, ev, e, 1/(3+1) * (1 + z^(-1) + z^(-2) + z^(-3) ) );
//   [sim, i2] = ld_ztf(sim, ev, u_Theta, (1-0.8)*(z-1)/(z-0.8) ) );

  [sim, i3] = ld_abs(sim, ev, i1);
  [sim, i4] = ld_compare_01(sim, ev, in=i3,  thr=eps);
  [sim, i5] = ld_not(sim, ev, in=i4);
  
  [sim, resetto] = ld_const(sim, ev, 0);
  [sim, count] = ld_counter(sim, ev, count=i5, reset=i4, resetto, initial=0);

  [sim, reached] = ld_compare_01(sim, ev, in=count,  thr=N);
endfunction

function [sim, reached] = greater_eps_for_N_steps(sim, ev, in, eps, N)
    // in order to be reached = 1, in must be greater than eps for N time steps
    // otherwise reached = 0
  [sim, i4] = ld_compare_01(sim, ev, in,  thr=eps);
  [sim, i5] = ld_not(sim, ev, in=i4);
  
  [sim, resetto] = ld_const(sim, ev, 0);
  [sim, count] = ld_counter(sim, ev, count=i4, reset=i5, resetto, initial=0);

  [sim, reached] = ld_compare_01(sim, ev, in=count,  thr=N);
    
endfunction





function [sim, outlist, active_state, x_global_kp1, userdata] = cntrled_ap_ident2_sm(sim, inlist, x_global, state, statename, userdata)
  // This function is called multiple times: once for each state.
  // At runtime these are different nested simulation. Switching
  // between them is done, where each simulation represents a
  // certain state.
  
  printf("defining state %s (#%d) ... \n", statename, state  );
  
  // define names for the first event in the simulation
  ev = 0;

  // get the par structure of the main function
  par = userdata;

 // [sim] = ld_print_angle(sim, sim.ev.stimev, r_p , termcode.red + 'r_Th_f: (Reference Elbow):' + termcode.reset);
  

  // print out some state information
  [sim] = ld_printf(sim, ev, in=inlist(1), str=termcode.red + "active state: "+string(statename) + termcode.reset, insize=1);
//   [sim] = ld_printf(sim, ev, in=inlist(2), str="state"+string(state)+": indata(2)", insize=1);
  [sim] = ld_printf(sim, ev, in=x_global, str="state"+string(state)+": x_global", insize=4);

  // inputs
  u_feedback = inlist(1);
  reset = inlist(2);
  y = inlist(3);

  // demultiplex x_global
  [sim, x_global] = ld_demux(sim, ev, vecsize=4, invec=x_global);

  // default data for output
  [sim, disturbing_signal] = ld_constvec(sim, ev, vec=[0]);
  [sim, pauseI] = ld_const(sim, ev, 0);

  // by default not finished
  [sim, finished] = ld_constvec(sim, ev, vec=[0]);

  select state
    case 1 // state 1
     // the desired reference low
     [sim, r_y] = ld_const(sim, ev, par.r_y_low);

     // reference reached detection
     [sim, reached] = reference_reached(sim, ev, r = r_y, y, N=par.reached_N, eps=par.reached_eps);

     // rememer the actuation variable in order to reach the reference r_y_hi
     x_global(1) = u_feedback;

     // wait for the controller to reach the ap      
     [ sim, active_state ] = ld_const(sim, ev, 0);  // by default: no state switch       
     [ sim, active_state ] = ld_cond_overwrite(sim, ev, in=active_state, condition=reached, setto=2);

      // print the reference
      [sim] = ld_print_angle(sim, ev, r_y, termcode.red + "Reference (go up  ) " + termcode.reset);
      [sim] = ld_print_angle(sim, ev, y, termcode.green + 'Th_f (Position Elbow):' + termcode.reset); 


    case 2 // state 2 move up
      // the desired reference low
      [sim, r_y] = ld_const(sim, ev, par.r_y_hi);

      // reference reached detection
      [sim, reached] = reference_reached(sim, ev, r = r_y, y, N=par.reached_N, eps=par.reached_eps);

      // rememer the actuation variable in order to reach the reference r_y_low
      x_global(2) = u_feedback;

      // wait for the controller to reach the ap      
      [ sim, active_state ] = ld_const(sim, ev, 0);  // by default: no state switch       
      [ sim, active_state ] = ld_cond_overwrite(sim, ev, in=active_state, condition=reached, setto=3);

      // print the reference
      [sim] = ld_print_angle(sim, ev, r_y, termcode.red + "Reference (go up  ) " + termcode.reset);
      [sim] = ld_print_angle(sim, ev, y, termcode.green + 'Th_f (Position Elbow):' + termcode.reset); 

    case 3 // state 3 move down
      // the desired reference low
      [sim, r_y] = ld_const(sim, ev, par.r_y_low);

      // reference reached detection
      [sim, reached] = reference_reached(sim, ev, r = r_y, y, N=par.reached_N, eps=par.reached_eps);
 
      // wait for the controller to reach the ap      
      [ sim, active_state ] = ld_const(sim, ev, 0);  // by default: no state switch       
      [ sim, active_state ] = ld_cond_overwrite(sim, ev, in=active_state, condition=reached, setto=4);

      // print the reference
      [sim] = ld_print_angle(sim, ev, r_y, termcode.red + "Reference (go down) " + termcode.reset);
      [sim] = ld_print_angle(sim, ev, y, termcode.green + 'Th_f (Position Elbow):' + termcode.reset); 


    case 4 // state 4

      // the desired reference low
      [sim, r_y] = ld_const(sim, ev, par.r_y_low);

      // generate the disturbing signal
      [sim, disturbing_signal ] = ld_play_simple(sim, ev,  r=[ par.dist_sig ] );
      u_lower = x_global(1); u_hi = x_global(2); 
      [sim, disturbing_signal ] = ld_muparser(sim, ev, inlist=list( u_lower, u_hi, disturbing_signal  ), str=" (u2-u1) * u3 ", float_param=[0] );

      // pause integrator if there is a step signal
      [sim, pauseI] = ld_compare_01(sim, ev, in=disturbing_signal,  thr=0.00001);

      // check if a certain band for y is left due to the disturbing_signal. Then stop and go to finished
      [sim, leftupper] = ld_compare_01(sim, ev, in=y,  thr=par.y_upper_thr);

      [sim, minus_y] = ld_gain(sim, ev, y, -1);
      [sim, leftlower] = ld_compare_01(sim, ev, in=minus_y,  thr=-par.y_lower_thr);

      // wait 10 simulation steps and then switch to state 3 if nothing happend
      //[ sim, timeout ] = ld_steps(sim, ev, activation_simsteps=[length(par.dist_sig)*6], values=[-1,1]); // when the 
      [sim, timeout ] = ld_play_simple(sim, ev,  r=[ zeros(par.dist_sig), 1 ] );

      // decision when to switch
      [ sim, active_state ] = ld_const(sim, ev, 0);  // by default: no state switch
     // [ sim, active_state ] = ld_cond_overwrite(sim, ev, in=active_state, condition=timeout, setto=3);
      [ sim, active_state ] = ld_cond_overwrite(sim, ev, in=active_state, condition=leftupper, setto=5);
      [ sim, active_state ] = ld_cond_overwrite(sim, ev, in=active_state, condition=leftlower, setto=5);

      // print the reference
      [sim] = ld_print_angle(sim, ev, r_y, termcode.red + "Reference " + termcode.reset);
      [sim] = ld_print_angle(sim, ev, y, termcode.green + 'Th_f (Position Elbow):' + termcode.reset); 

    case 5 // finished
     // the desired reference low
     [sim, r_y] = ld_const(sim, ev, par.r_y_hi); // doesn't matter

     [sim, finished] = ld_constvec(sim, ev, vec=[1]);

      // decision when to switch
     [ sim, active_state ] = ld_const(sim, ev, 0);  // by default: no state switch       

  end



  // multiplex the new global states
  [sim, x_global_kp1] = ld_mux(sim, ev, vecsize=4, inlist=x_global);
  
  // the user defined output signals of this nested simulation
  outlist = list(disturbing_signal, finished, pauseI, r_y);
endfunction

function [sim, u, finished, dsig ] = cntrled_ap_ident2(sim, ev, y, par)
// 
// 
//  
// 
// 



z = poly(0, 'z');


//   [sim, r_y] = ld_const(sim, ev, par.r_y);
//   [sim, e] = ld_add(sim, ev, list(r_y, y), [1, -1] );
//   
//   // check
//   [sim, reached] = reference_reached(sim, ev, r = r_y, y, N=par.reached_N, eps=par.reached_eps);

  // no reset for now
  [sim, reset] = ld_const(sim, ev, 0);

  // feedback of the actuation variable without the disturbing_signal
  [sim, u_feedback] = libdyn_new_feedback(sim);
  [sim, u_feedback__] = ld_ztf(sim, ev, u_feedback, 1/z); // to avoid algebraic loop

  // statemachine to generated disturbing signal
  [sim, outlist, x_global, active_state,userdata] = ld_statemachine(sim, ev, ...
      inlist=list(u_feedback__, reset, y), ..
      insizes=[1,1,1], outsizes=[1,1,1,1], ... 
      intypes=[ORTD.DATATYPE_FLOAT,ORTD.DATATYPE_FLOAT,ORTD.DATATYPE_FLOAT  ], outtypes=[ORTD.DATATYPE_FLOAT,ORTD.DATATYPE_FLOAT,ORTD.DATATYPE_FLOAT,ORTD.DATATYPE_FLOAT], ...
      nested_fn=cntrled_ap_ident2_sm, Nstates=5, state_names_list=list("control1", "control2", "control3", "generate_disturbing_signal", "finished"), ...
      inittial_state=1, x0_global=[0,0,0,0], userdata=par  );  

   // get the outputs of the statemachine
   disturbing_signal = outlist(1);
   finished = outlist(2);
   pauseI = outlist(3);
   reference = outlist(4);

  // A very low performance controller, which is active when finished is false
  r_y = reference;
  [sim, enable_controller] = ld_not(sim, ev, finished);
  [sim, e] = ld_add(sim, ev, list(r_y, y), [1, -1] ); // control error
  [sim, e] = ld_mult(sim, ev, list( e, enable_controller ), [0, 0] );  // zero error, when deactivated

       //
       [sim, enableI] = ld_not(sim, ev, pauseI);
       [sim, eI] = ld_mult(sim, ev, list( e, enableI ), [0, 0] );  // zero error for integrator
       [sim,uI] = ld_limited_integrator(sim, ev, eI, min__=0, max__=1/par.kI, Ta=par.Ta);
       [sim,u2] = ld_ztf(sim, ev, eI, par.Kpz);
    
       [sim,u1] = ld_add(sim, ev, list(uI,u2), [par.kI,1]);
       //

  [sim, u1] = ld_mult(sim, ev, list( u1, enable_controller ), [0, 0] ); // zero actuation var, when deactivated

  // Feed back u
  [sim] = libdyn_close_loop(sim, u1, u_feedback);

   // add disturbing_signal to actuation variable
   [sim, u] = ld_add(sim, ev, list(u1, disturbing_signal), [1, 1]);




   //
   [sim] = ld_printf(sim, ev, enableI, "enabling integral state ", 1);
   [sim] = ld_printf(sim, ev, disturbing_signal, "disturbing_signal ", 1);
   [sim] = ld_printf(sim, ev, finished, "finished", 1);

   [sim] = ld_printf(sim, ev, u, "u", 1);
   [sim] = ld_printf(sim, ev, uI, "uI", 1);

   dsig = disturbing_signal;
endfunction





//
// Set up simulation schematic
//


// This is the main top level schematic
function [sim, outlist] = schematic_fn(sim, inlist)
  // this is the default event
  ev = 0;

  // generate two signals   
  [sim, u1] = ld_play_simple(sim, ev, r=linspace(0,1,100) );
  [sim, u2] = ld_play_simple(sim, ev, r=sin( linspace(0,%pi*6,100) ) );
   
  //  calculate us = - u1 + 2*u2
  [sim,us] = ld_add(sim, ev, list(u1, u2), [-1, 2] );
  
  // save the signal us
  [sim] = ld_savefile(sim, ev, fname="result.dat", source=us, vlen=1);
  
  // output of schematic
  [sim, out] = ld_const(sim, ev, 0);
  outlist = list(out); // Simulation output #1
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
save_irparam(par, 'controller.ipar', 'controller.rpar');

// clear
par.ipar = [];
par.rpar = [];




// optionally execute
messages=unix_g(ORTD.ortd_executable+ ' -s controller -i 901 -l 100');

//
//// load results
//A = fscanfMat('result.dat');
//
//scf(1);clf;
//plot(A(:,1), 'k');

