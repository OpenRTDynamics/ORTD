exec('irpar.sci');
exec('libdyn.sci');
z=poly(0,'z');



testsignal = sin(linspace(0,1,100)*2*%pi*10);


//
// 
//


  // Set up simulation schematic
  sim = libdyn_new_simulation([], []);
  defaultevents = [0];
  stimevent = [1]; 
  [simulation_inputs] = libdyn_get_input_signals(sim); // get input signals of container

  [sim,tf_vz] = libdyn_new_blk_zTF(sim, defaultevents, 1/z); // a transfer function
  [sim,play] = libdyn_new_blk_play(sim, stimevent, testsignal, 1); // a sampler block
  
  [sim,tf_vz] = libdyn_conn_equation(sim, tf_vz, list(simulation_inputs, 0)); // an equation
//  sim = libdyn_connect_block(sim, tf_Ki, 0, sum_u, 1);

  sim = libdyn_connect_extern_ou(sim, play, 0, 0); // connect output
  
  // Collect and encode as irparem-set
  sim = libdyn_build_cl(sim); 
  shoulder_irpar = combine_irparam(sim.parlist);


// Pack into irpar set and write to file
parlist = new_irparam_set();

parlist = new_irparam_elemet_box(parlist, shoulder_irpar.ipar, shoulder_irpar.rpar, 1001);

arm_control_par = combine_irparam(parlist);
save_irparam(arm_control_par, 'box.ipar', 'box.rpar');
arm_control_par.ipar = [];
arm_control_par.rpar = [];

