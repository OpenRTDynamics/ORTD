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

thispath = get_absolute_file_path('matrix_test.sce');
cd(thispath);



//
// Set up simulation schematic
//


// This is the main top level schematic
function [sim, outlist] = schematic_fn(sim, inlist)
  // Input - Konstante - 1x1
  [sim,u_c] = ld_const(sim, defaultevents, 1.3);

  // Input - Konstante - 1xX (Vektor)
  [sim,u_v] = ld_constvec(sim, defaultevents, [1,2,4.3,4,6.3,-6,-1,6] );

  // Input - Konstante - XxX (Matrix)
  [sim,u_m] = ld_constmat(sim, defaultevents, [1,2; 3,4;] );

  // Transformation: Vector -> Matrix
  [sim,u_m1] = ld_vec2mat(sim, defaultevents, 1, u_v, 8);

  // Transformation: Vector -> Matrix
  [sim,u_m2] = ld_vec2mat(sim, defaultevents, -1, u_v, 8);


  // Transformation:  Matrix -> Vector
  [sim,u_v1] = ld_mat2vec(sim, defaultevents, u_m1, 8);
  [sim,u_v2] = ld_mat2vec(sim, defaultevents, u_m2, 8);


  [sim,u_m_2] = ld_matmul(sim, defaultevents, u_m, [2,2], u_m, [2,2]);

  //[sim] = ld_printf(sim, defaultevents, u_c, "u_c = ", 1);
  //[sim] = ld_printf(sim, defaultevents, u_v, "u_v = ", 8);
  //[sim] = ld_printf(sim, defaultevents, u_m, "u_m = ", 8);
  
  // save result to file
  [sim, save0] = ld_dumptoiofile(sim, defaultevents, "results_matrix_u_c.dat", u_c);
  //[sim, save1] = ld_dumptoiofile(sim, defaultevents, "results_matrix_u_v.dat", u_v); <--- Funzt nicht mit Vektoren!
  // save the vector to a file
  [sim]=ld_savefile(sim, defaultevents, fname="results_matrix_u_v.dat", source=u_v, vlen=8);
  [sim]=ld_savefile(sim, defaultevents, fname="results_matrix_u_v1.dat", source=u_v, vlen=8);
  [sim]=ld_savefile(sim, defaultevents, fname="results_matrix_u_v2.dat", source=u_v, vlen=8);
  // save the matrix to a file
  [sim]=ld_savefile(sim, defaultevents, fname="results_matrix_u_m.dat", source=u_m, vlen=6);
  [sim]=ld_savefile(sim, defaultevents, fname="results_matrix_u_m_2.dat", source=u_m_2, vlen=6);
  [sim]=ld_savefile(sim, defaultevents, fname="results_matrix_u_m1.dat", source=u_m1, vlen=10);
  [sim]=ld_savefile(sim, defaultevents, fname="results_matrix_u_m2.dat", source=u_m2, vlen=10);

  // output of schematic
  outlist = list(u_c);
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
save_irparam(par, 'matrix_test.ipar', 'matrix_test.rpar');

// clear
par.ipar = [];
par.rpar = [];




// optionally execute
messages = unix_g(ORTD.ortd_executable+' -s matrix_test -i 901 -l 1');


// // load results
// A = fscanfMat('results_matrix_u_c.dat');
// B = fscanfMat('results_matrix_u_v.dat');
// 
// scf(1);clf;
// plot(A(:,1), 'k');
// plot(B, 'k');

