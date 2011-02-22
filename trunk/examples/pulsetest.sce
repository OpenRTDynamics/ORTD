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
// Example that generates tones
//
// load with pulsetest.cpp and press 'q' or 'w' or ESC to exit
//

//exec('../scilab/irpar.sci');
//exec('../scilab/libdyn.sci');
z=poly(0,'z');


T_a = 1/44100;

testsignal = 1/T_a./(linspace(1000,300,40000));


//
// 
//

function [sim,sinus] = constant_sinus(sim, events, amp, frq)
  [sim,amp_] = ld_const(sim, initialevent, amp);
  [sim,frq_] = ld_const(sim, initialevent, (1/T_a)/frq);  
  
  [sim,sinus] = ld_fngen(sim, events, list(frq_,amp_), 0);
endfunction

// FIXME: raus
function [sim,sinus] = frqmod_sinus(sim, events, frqmodin, amp)
  [sim,amp_] = libdyn_new_blk_const(sim, initialevent, amp);
  
  [sim,sinus] = ld_fngen(sim, events, list(frqmodin(1),frqmodin(2),amp_,0), 0);
endfunction

function [sim,tone_modulator] = key_model(sim, keymodelevent, dynamicevent, key)
  dynconst_behave = linspace(0.00004, 0.00002, 10);
  //scf(1);clf;plot(dynconst_behave);

  I = (1)/(z-1);
  D = (z-1)/z^1;
  
  [sim,keyd] = ld_ztf(sim, keymodelevent, list(key), D);
  printf("D irpar id = %d\n", keyd.oid);

  // Die Abklingconstante
//  [sim,dynconst] = libdyn_new_blk_const(sim, keymodelevent, 0.00004);
  [sim,dynconst] = ld_lkup(sim, keymodelevent, list(key), 0, 1, dynconst_behave);
  
  // Nur positives durchlassen  
//  [sim,switch] = libdyn_new_blk_switch(sim, keymodelevent);
//  [sim,switch] = libdyn_conn_equation(sim, switch, list(keyd, 0, keyd, 0));  
  cntrl = keyd;  in = keyd;
  [sim,switch_pos, switch_neg] = ld_switch(sim, dynamicevent, cntrl, in)
  
  // The integrator of the dynamical system
  [sim,FB_integrator_in] = libdyn_new_feedback(sim);
  [sim,integrator] = ld_ztf(sim, dynamicevent, list(FB_integrator_in),I);
   
  //  [sim,feedback] = ld_gain(sim, dynamicevent, list(integrator, 0), 0.00004);
  [sim,feedback] = ld_mul(sim, dynamicevent, list(integrator, dynconst), 0, 0);
  
  
  [sim,sum1] = ld_sum(sim, dynamicevent, list(switch_pos, feedback), 1, -1);
  
  // connect the feedback to the integrator
  [sim] = libdyn_close_loop(sim, sum1, FB_integrator_in);
  
  // connect output
  tone_modulator = integrator;


  [sim, save1] = libdyn_dumptoiofile(sim, lowerevent, "debugsw.dat", list(switch_pos));
  [sim, save1] = libdyn_dumptoiofile(sim, lowerevent, "debugfb.dat", list(feedback));
  [sim, save1] = libdyn_dumptoiofile(sim, lowerevent, "debugsum.dat", list(sum1));
  [sim, save1] = libdyn_dumptoiofile(sim, lowerevent, "keyd.dat", list(keyd));
  [sim, save1] = libdyn_dumptoiofile(sim, lowerevent, "key_.dat", list(key));

endfunction

function [sim,tone] = tone_model(sim, defaultevents, tm, basefreq)
  // Two sine
  [sim,sinus1] = constant_sinus(sim, defaultevents, 0.25*1*10000, basefreq+800);
  [sim,sinus2] = constant_sinus(sim, defaultevents, 0.25*10000, basefreq+1000);

  
  // sum up
  [sim,tone_unmod] = ld_sum(sim, defaultevents, list(sinus1, sinus2), 1,1);
  
  [sim,tone] = ld_mul(sim, defaultevents, list(tone_unmod, tm), 0,0);
  
  //tone = tone_unmod;
endfunction


  // Set up simulation schematic
  sim = libdyn_new_simulation([], []);
  defaultevents = [0];
  dynamicevent = [1]; 
  lowerevent = [2];
  loopevent = [3];
  
  initialevent = [3];
  
  
   // get input signals of container
  [sim,simulation_inputs] = libdyn_get_external_ins(sim);
  //for i=1:12
    [sim,key0] = libdyn_new_oport_hint(sim, simulation_inputs, 0);
  //end
  [sim,key1] = libdyn_new_oport_hint(sim, simulation_inputs, 1);
  
  [sim,tm0] = key_model(sim, loopevent, defaultevents, key0);
  [sim,tm1] = key_model(sim, loopevent, defaultevents, key1);
//  tm1 = tm0;
  
  [sim, save0] = libdyn_dumptoiofile(sim, loopevent, "key.dat", list(key0));
  [sim, save1] = libdyn_dumptoiofile(sim, loopevent, "tm.dat", list(tm0));

//  // Two sine
//  [sim,sinus1] = constant_sinus(sim, defaultevents, 1*10000, 800);
//  [sim,sinus2] = constant_sinus(sim, defaultevents, 10000, 1000);
//
//  // A modulated sine
//  [sim,frq_modulator] = libdyn_new_blk_play(sim, dynamicevent, testsignal, 1, 1); // MOdulator
//  [sim,sinus_sweep] = frqmod_sinus(sim, defaultevents, list(frq_modulator,0), 10000);

  [sim, tone0] = tone_model(sim, defaultevents, tm0, 100);
  [sim, tone1] = tone_model(sim, defaultevents, tm1, 400);
  
  // sum up
  [sim,wave] = ld_sum(sim, defaultevents, list(tone0, tone1), 1,1);
  //wave = tone0;
  
  // connect output
  sim = libdyn_connect_outport(sim, wave, 0);
  
  
  
  // Collect and encode as irparem-set
  sim = libdyn_build_cl(sim); 
  shoulder_irpar = combine_irparam(sim.parlist);


// Pack into irpar set and write to file
parlist = new_irparam_set();

parlist = new_irparam_elemet_box(parlist, shoulder_irpar.ipar, shoulder_irpar.rpar, 901);

arm_control_par = combine_irparam(parlist);
save_irparam(arm_control_par, 'pulsetest.ipar', 'pulsetest.rpar');
arm_control_par.ipar = [];
arm_control_par.rpar = [];

