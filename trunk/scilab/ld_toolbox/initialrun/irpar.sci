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
// Some functions for combining various data structures
// into one rpar + ipar set
// 
// decoding is done by irpar.h
//

IRPAR_RVEC = 1;
IRPAR_RMAT = 2;
IRPAR_TF=  3;
IRPAR_IVEC = 4;

IRPAR_LIBDYN_BLOCK = 100;
IRPAR_LIBDYN_CONNLIST = 101;


function parlist = new_irparam_set()
  parlist = list();
endfunction

function parlist = new_irparam_elemet(parlist, id, typ, ipar, rpar)
//    clear par;
//    

//  par = [];
//  par.id = id;
//  par.typ = typ;
//  par.ipar = ipar(:);
//  par.rpar = rpar(:);
//  parlist($+1) = par;
//
  parlist($+1) = struct( 'id', id, 'typ', typ, 'ipar', ipar(:), 'rpar', rpar(:)  );
endfunction

function parlist = new_irparam_elemet_rvec(parlist, v, id)
//  parlist = new_irparam_elemet(parlist, id, 1, length(v), v(:));
  
  parlist($+1) = struct( 'id', id, 'typ', 1, 'ipar', length(v), 'rpar', v(:)  );
  
endfunction

function parlist = new_irparam_elemet_ivec(parlist, v, id)
//  parlist = new_irparam_elemet(parlist, id, IRPAR_IVEC, [length(v); v(:) ], []);

  parlist($+1) = struct( 'id', id, 'typ', 4, 'ipar', [length(v); v(:)], 'rpar', []  );
endfunction

function parlist = new_irparam_elemet_rmat(parlist, A, id)
  [m,n] = size(A);
  parlist = new_irparam_elemet(parlist, id, 2, [m,n] , A(:));
endfunction

function parlist = new_irparam_elemet_tf(parlist, tf, id)
  degden = degree(tf.den);
  den = coeff(tf.den);
  normfac = den(degden+1);
  parlist = new_irparam_elemet(parlist, id, 3, [degree(tf.num),degden] , [coeff(tf.num),den]);
endfunction

function parlist = new_irparam_elemet_box(parlist, ipar, rpar, id)
//    printf("***********\n");
  parlist = new_irparam_elemet(parlist, id, 10, ipar , rpar);
endfunction

// same as new_irparam_elemet_box but with a irparlsit structure
function parlist = new_irparam_container(parlist, irparlist, id)
//  parlist = new_irparam_elemet(parlist, id, 10, irparlist.ipar , irparlist.rpar);
  
  parlist($+1) = struct( 'id', id, 'typ', 10, 'ipar', irparlist.ipar(:), 'rpar', irparlist.rpar(:) );
endfunction





function blockparam = combine_irparam(parlist)
  // merge i+rpar
  
//  tic();

  rpar_ptr = []; //list();
  ipar_ptr = []; //list();

  rparges = [];
  iparges = [];


  for i = 1:length(parlist)
    ipar_ptr = [ ipar_ptr; length(iparges) ];
    iparges = [iparges; parlist(i).ipar(:) ];

    rpar_ptr = [rpar_ptr; length(rparges) ];
    rparges = [rparges; parlist(i).rpar(:) ];
  end

  // build header
 
  header_entries = length(parlist);
  version = 1;
  header = [version; header_entries];

  for i = 1:length(parlist)
    pip_len = length(parlist(i).ipar(:));
    prp_len = length(parlist(i).rpar(:));
    
    header = [header; parlist(i).id; parlist(i).typ; ipar_ptr(i); rpar_ptr(i); pip_len; prp_len];
  end

  // summ up
  blockparam = struct( 'ipar', [header; iparges], 'rpar', rparges );

//  blockparam.ipar = [header; iparges];
//  blockparam.rpar = rparges;
//  
  
//  
//  time= toc();
//  printf("Required time to combine irpar: %f s\n", time);
endfunction

function save_irparam(blockparam, fname_ipar, fname_rpar)
  fprintfMat(fname_ipar, blockparam.ipar, '%11.0f');  
    fprintfMat(fname_rpar, blockparam.rpar, '%10.25f');  
endfunction
