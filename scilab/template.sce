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





//exec('../scilab/irpar.sci');
//exec('../scilab/libdyn.sci');
z=poly(0,'z');



//
//  template schematic
//

function [sim,sinus] = constant_sinus(sim, events, amp, frq)
  [sim,amp_] = libdyn_new_blk_const(sim, events, amp);
  [sim,frq_] = libdyn_new_blk_const(sim, events, (1/T_a)/frq);
  
  [sim,sinus] = ld_fngen(sim, defaultevents, list(frq_,0,amp_,0), 0);
endfunction

  //
  // Set up simulation schematic
  //
  sim = libdyn_new_simulation([], []);
  defaultevents = [0];
  specialevent = [1]; 
  
   // get input signals of container
  [sim,simulation_inputs] = libdyn_get_external_ins(sim);
  [sim,in0] = libdyn_new_oport_hint(sim, simulation_inputs, 0);
  [sim,in1] = libdyn_new_oport_hint(sim, simulation_inputs, 1);
  
  
  // sum up
  [sim,sumin] = ld_sum(sim, defaultevents, list(in0, in1), 1,1);

  //
  [sim,output] = ld_ztf(sim, defaultevents, list(sumin,0), (1-0.8)/(z-0.8) );
  
  // connect output
  sim = libdyn_connect_extern_ou(sim, output, 0, 0); // connect output
  
  
 
  //
  // Collect and encode as irparem-set
  //
  sim = libdyn_build_cl(sim); 
  shoulder_irpar = combine_irparam(sim.parlist);


// Pack into irpar set and write to file
parlist = new_irparam_set();

parlist = new_irparam_elemet_box(parlist, shoulder_irpar.ipar, shoulder_irpar.rpar, 101);

template_par = combine_irparam(parlist);
save_irparam(template_par, 'template.ipar', 'template.rpar');
template_par.ipar = [];
template_par.rpar = [];

