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
exec('../scilab/irpar.sci');
exec('../scilab/libdyn.sci');


T_as =1/50;
T_am = 1/1;

noplot = 0;
z = poly(0,'z');




u1 = [0,1,1,1,1,1,1, 1,1,1,1,  0,0,0, 0*ones(1,10) ];
u2 = [0,0,0,0,0,0,0, 0.5,  0,  0,  0,    0,0,1, 0*ones(1,10) ];



//
// 
//



if 1==1 then

//function sim = define_acc_controller()
  HP = z/z; //(z-1) /( (z-0.9988) );  // <-------- (1-0.96)*(z-1), z-0.96
  INT1 = 1 * T_am/2 * (z+1)/(z-1);
  INT2 = 1 * T_am/2 * (z+1)/(z-1);
  INT3 = 1 * T_am/2 * (z+1)/(z-1);
  INT4 = 1 * T_am/2 * (z+1)/(z-1);
  
//  INT1 = INT1 / (1+ 0.01*INT1);
//  INT2 = INT2 / (1+ 0.01*INT2);
//  INT3 = INT3 / (1+ 0.01*INT3);
//  INT4 = INT4 / (1+ 0.01*INT4);

  TP = z/z; //((1-0.9))/(z-0.9);


  INT1 = INT1 * 0.05 * TP;
  INT2 = INT2 * 0.05 * TP;
  INT3 = INT3 * 0.05 * TP;
  INT4 = INT4 * 0.05 * TP;
  
//  TP = ((1-0.99)/(z-0.99));
//  INT1 = TP;
//  INT2 = TP;
//  INT3 = TP;
//  INT4 = TP;
  
  A_i = 1/(z^4);

  // Set up simulation schematic
  sim = libdyn_new_simulation([1;1;1], [1;1;1;1]);
  defaultevents = [0];
  
  [simulation_inputs] = libdyn_get_input_signals(sim);
  
  

  [sim,tf_Ai] = libdyn_new_blk_zTF(sim, defaultevents, A_i);
  [sim,tf_HP] = libdyn_new_blk_zTF(sim, defaultevents, HP);
  
  [sim,switch1] = libdyn_new_blk_switch(sim, defaultevents);
  [sim,switch2] = libdyn_new_blk_switch(sim, defaultevents);

  [sim,tf_INT1] = libdyn_new_blk_zTF(sim, defaultevents, INT1);
  [sim,tf_INT2] = libdyn_new_blk_zTF(sim, defaultevents, INT2);
  [sim,tf_INT3] = libdyn_new_blk_zTF(sim, defaultevents, INT3);
  [sim,tf_INT4] = libdyn_new_blk_zTF(sim, defaultevents, INT4);

  
  [sim,sum_e1] = libdyn_new_blk_sum(sim, defaultevents, 1, -1);
  [sim,sum_e2] = libdyn_new_blk_sum(sim, defaultevents, -1, 1);

  [sim,mul_e1] = libdyn_new_blk_mul(sim, defaultevents, 1, 0);
  [sim,mul_e2] = libdyn_new_blk_mul(sim, defaultevents, 1, 0);

  [sim,sum_ofs1] = libdyn_new_blk_sum(sim, defaultevents, 1, 1);
  [sim,sum_ofs2] = libdyn_new_blk_sum(sim, defaultevents, 1, 1);

  [sim,const_ofs] = libdyn_new_blk_const(sim, defaultevents, 1);

  [sim,const1] = libdyn_new_blk_const(sim, defaultevents, 2.5);
  
  
  // test sin generator
  [sim,const_period] = libdyn_new_blk_const(sim, defaultevents, 10);
  [sim,const_amp] = libdyn_new_blk_const(sim, defaultevents, 1);
  [sim,sin1] = libdyn_new_blk_fngen(sim, defaultevents, 0);
  
//  sim = libdyn_connect_block(sim, const_period, 0, sin1, 0);  
//  sim = libdyn_connect_block(sim, const_amp, 0, sin1, 1);  
  [sim,sin1] = libdyn_conn_equation(sim, sin1, list(const_period, 0, const_amp, 0));

  
  [sim,interf_sin] = libdyn_new_interface(sim, defaultevents, 1);
  sim = libdyn_connect_block(sim, sin1, 0, interf_sin, 0);    
  
  // test mrate
  lowrate_event = [1];
  [sim,tf_vz1] = libdyn_new_blk_zTF(sim, lowrate_event, 1/z);
  sim = libdyn_connect_block(sim, interf_sin, 0, tf_vz1, 0);    
//  sim = libdyn_connect_extern_in(sim, 0, tf_vz1, 0);
  
  
  
//  sim = libdyn_connect_extern_in(sim, 0, tf_Ai, 0);
  [sim,tf_Ai] = libdyn_conn_equation(sim, tf_Ai, list(simulation_inputs, 0));
  sim = libdyn_connect_extern_in(sim, 1, tf_HP, 0);

    sim = libdyn_connect_block(sim, tf_Ai, 0, switch1, 0);  
  sim = libdyn_connect_block(sim, tf_Ai, 0, switch1, 1);  
  sim = libdyn_connect_block(sim, tf_HP, 0, switch2, 0);  
  sim = libdyn_connect_block(sim, tf_HP, 0, switch2, 1);  
//  sim = libdyn_connect_extern_in(sim, 1, switch2, 0);
//  sim = libdyn_connect_extern_in(sim, 1, switch2, 1);

  sim = libdyn_connect_block(sim, switch1, 0, tf_INT1, 0);
  sim = libdyn_connect_block(sim, switch1, 1, tf_INT2, 0);
  sim = libdyn_connect_block(sim, switch2, 0, tf_INT3, 0);
  sim = libdyn_connect_block(sim, switch2, 1, tf_INT4, 0);

  sim = libdyn_connect_block(sim, tf_INT1, 0, sum_e1, 0);
  sim = libdyn_connect_block(sim, tf_INT3, 0, sum_e1, 1);
  sim = libdyn_connect_block(sim, tf_INT2, 0, sum_e2, 0);
  sim = libdyn_connect_block(sim, tf_INT4, 0, sum_e2, 1);
  
  sim = libdyn_connect_block(sim, sum_e1, 0, sum_ofs1, 0);  // plus Ofset
  sim = libdyn_connect_block(sim, const_ofs, 0, sum_ofs1, 1);
  sim = libdyn_connect_block(sim, sum_e2, 0, sum_ofs2, 0);
  sim = libdyn_connect_block(sim, const_ofs, 0, sum_ofs2, 1);
  

  sim = libdyn_connect_block(sim, tf_INT1, 0, mul_e1, 0); // 1; -1
  sim = libdyn_connect_block(sim, tf_INT3, 0, mul_e1, 1);
  sim = libdyn_connect_block(sim, tf_INT2, 0, mul_e2, 0); // -1; 1
  sim = libdyn_connect_block(sim, tf_INT4, 0, mul_e2, 1);


  sim = libdyn_connect_extern_ou(sim, sum_ofs1, 0, 0);
  sim = libdyn_connect_extern_ou(sim, tf_vz1, 0, 1);
  

//  sim = libdyn_connect_extern_ou(sim, switch1, 0, 2);
//  sim = libdyn_connect_extern_ou(sim, switch2, 0, 3);
  sim = libdyn_connect_extern_ou(sim, interf_sin, 0, 2);
  sim = libdyn_connect_extern_ou(sim, const1, 0, 3);
  
  sim = libdyn_connect_extern_ou(sim, tf_INT1, 0, 4);
  sim = libdyn_connect_extern_ou(sim, tf_INT3, 0, 5);
  sim = libdyn_connect_extern_ou(sim, const1, 0, 6);
  
  sim = libdyn_build_cl(sim);
//endfunction


//sim = define_acc_controller;
sim_adapt_container_irpar = combine_irparam(sim.parlist);







//
//
//


parlist = new_irparam_set();

parlist = new_irparam_elemet_rvec(parlist, u1,  920);
parlist = new_irparam_elemet_rvec(parlist, u2,  921);

// pack simulations into irpar container
//parlist = new_irparam_elemet_box(parlist, sim_acc_container_irpar.ipar, sim_acc_container_irpar.rpar, 900);
parlist = new_irparam_elemet_box(parlist, sim_adapt_container_irpar.ipar, sim_adapt_container_irpar.rpar, 901);


arm_control_par = combine_irparam(parlist);
save_irparam(arm_control_par, 'ldtest.ipar', 'ldtest.rpar');
arm_control_par.ipar = [];
arm_control_par.rpar = [];


//unix('./dynlib_test3');
unix('./dynlib_test4');



A = fscanfMat('ldtest_out.dat');

scf(1);clf;
plot(A(:,1), 'k');
plot(A(:,2), 'g');
plot(A(:,3), 'r');

