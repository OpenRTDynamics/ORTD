//
//    Copyright (C) 2011  Markus Valtin
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

thispath = get_absolute_file_path('matmul_test.sce');
cd(thispath);



//
// Set up simulation schematic
//


M1 = [1;2;3;4];
M2 = M1';
M3 = [1,2; 3,4;];
M4 = [1,2; 3,4; 5,6; 7,8]

V1 = [0.1,0.2,0.3,0.4];

// This is the main top level schematic
function [sim, outlist] = schematic_fn(sim, inlist)
  [sim,u_v] = ld_constvec(sim, defaultevents, M1 );
  // Transformation: Vector -> Matrix
//  [sim,u_m1] = ld_vec2mat(sim, defaultevents, 1, u_v, 4);  // 4x1 Matrix

  [sim,u_m1] = ld_constmat(sim, defaultevents, M1 ); // 4x1 Matrix

  [sim,u_m2] = ld_constmat(sim, defaultevents, M2 ); // 1x4 Matrix
  [sim,u_m3] = ld_constmat(sim, defaultevents, M3 ); //  2x2 Matrix

  [sim,u_m4] = ld_constmat(sim, defaultevents, M4 ); //  4x2 Matrix


  [sim,u_v1] = ld_constvec(sim, defaultevents, V1 );



  // Matrix Multiplikationen
  [lm_nr, lm_nc]=size(M1); [rm_nr, rm_nc]=size(M2);
  [sim,y1] = ld_matmul(sim, defaultevents, u_m1, [lm_nr,lm_nc], u_m2, [rm_nr,rm_nc]);


  [lm_nr, lm_nc]=size(M2); [rm_nr, rm_nc]=size(M4);
  [sim,y2] = ld_matmul(sim, defaultevents, u_m2, [lm_nr,lm_nc], u_m4, [rm_nr,rm_nc]);


  [lm_nr, lm_nc]=size(M4); [rm_nr, rm_nc]=size(M3);
  [sim,y3] = ld_matmul(sim, defaultevents, u_m4, [lm_nr,lm_nc], u_m3, [rm_nr,rm_nc]);


  // Matrix - Vektor Multiplikationen
  [sim,y4] = ld_matmul(sim, defaultevents, u_v1, [1,4], u_m4, [4,2]);



  //[sim] = ld_printf(sim, defaultevents, y1, "y = ", 1);
  
  // save result to file
  // save the matrix to a file
  [sim]=ld_savefile(sim, defaultevents, fname="results_matmul_y1.dat", source=y1, vlen=16);
  [sim]=ld_savefile(sim, defaultevents, fname="results_matmul_y2.dat", source=y2, vlen=2);
  [sim]=ld_savefile(sim, defaultevents, fname="results_matmul_y3.dat", source=y3, vlen=8);

  [sim]=ld_savefile(sim, defaultevents, fname="results_matmul_y4.dat", source=y4, vlen=2);
  // output of schematic
  outlist = list(u_v);
endfunction




//
// Set-up
//

// defile events
defaultevents = [0]; // main event

// set-up schematic by calling the user defined function "schematic_fn"
insizes = [1]; outsizes=[4];
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
save_irparam(par, 'matmul_test.ipar', 'matmul_test.rpar');

// clear
par.ipar = [];
par.rpar = [];




// optionally execute
messages = unix_g(ORTD.ortd_executable+' -s matmul_test -i 901 -l 1');


// // load results
// A = fscanfMat('results_matrix_u_c.dat');


y1 = M1*M2;
y2 = M2*M4;
y3 = M4*M3;

y4 = V1*M4;
