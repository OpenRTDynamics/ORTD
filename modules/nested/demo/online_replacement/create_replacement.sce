//
//    Copyright (C) 2012  Christian Klauer
//
//    This file is part of OpenRTDynamics, the Real Time Dynamics Toolbox
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


thispath = get_absolute_file_path('create_replacement.sce');
cd(thispath);


z = poly(0,'z');




//
// Set up simulation schematic
//

function [sim, outlist, userdata ] = replaceable_cntrl_main(sim, inlist, par)
//    
//    Schematic for the nested and online exchangeable controllers
//    here, the initial control schematics are defined if online = %F
//    Also this is used to defined the controllers during runtime of the 
//    control system
//    

  ev = 0;

  cntrlN = par(1); // the number of the nested schematics (one of two) "1" means the 
                   // dummy schematic which is activated while the 2nd "2" is exchanged during runtime
  userdata = par(2);
  
  string1 = userdata(1);
  
  input1 = inlist(1);
  

  printf("Defining the replacement controller\n");
  
  // a zero
  [sim, zero] = ld_const(sim, ev, 0);
  [sim] = ld_printf(sim, ev, in=zero, str="Hi, I''m the new simulation and now modified", insize=1);

  [sim] = ld_FlagProbe(sim, ev, in=zero, str="probe", insize=1)
  
      

  [sim, output] = ld_gain(sim, ev, input1, 2); // 
          

  
  outlist = list(output);
endfunction

  
//
// Set-up
//

    N = 2;
    online = %T; // define a real controller
    
            [par, userdata] = ld_simnest2_replacement( ...
                       insizes=[1], outsizes=[1], ...
                       intypes=ORTD.DATATYPE_FLOAT*[1], ...
                       outtypes=ORTD.DATATYPE_FLOAT*[1], ...
                       nested_fn=replaceable_cntrl_main, ...
                       userdata=list("Place", "any", "data"), N);


// save vectors to a file
save_irparam(par, 'replacement.ipar', 'replacement.rpar');

// clear
par.ipar = [];
par.rpar = [];




