// Interfacing functions are placed in this place

// function [sim,out] = ld_constmat(sim, events, mat) // PARSEDOCU_BLOCK
// // 
// // a constant matrix
// // 
// // mat *+ - the matrix
// // 
//   btype = 69001 + 1;
//   [nr, nc] = size(mat);
//   mat_length = nr*nc + 2;
// 
//   ipar = [mat_length; nr; nc; 0];
//   rpar = [nr,nc];
//   for i=1:nr,
//     rpar=cat(2,rpar, mat(i,:))
//   end
//   rpar=rpar';
// 
//   [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
//                    insizes=[1], outsizes=[ mat_length ], ...
//                    intypes=[ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT]  );
//  
//   [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
// endfunction
// 

// function [sim,out] = ld_constmat(sim, events, mat) // PARSEDOCU_BLOCK
// // 
// // a constant matrix
// // 
// // mat *+ - the matrix
// // 
// 
//   [nr, nc] = size(mat);
//   //rpar = [nr,nc];
//   rpar = [];
//   for i=1:nr,
//     rpar=cat(2,rpar, mat(i,:))
//   end
//   //rpar=rpar';
// 
//   [sim,out] = ld_constvec(sim, defaultevents, rpar );
// endfunction

function [sim,out] = ld_constmat(sim, events, mat) // PARSEDOCU_BLOCK
// 
// %PURPOSE: a constant matrix
// 
// mat *+ - the matrix
// 

//   [nr, nc] = size(mat);
//   //rpar = [nr,nc];
//   rpar = zeros(1,nr*nc);
//   for i=1:nr,
// //     rpar=cat(2,rpar, mat(i,:))
//     rpar( nc*(i-1)+1 : nc*i ) = mat(i,:);
//   end
//   //rpar=rpar';

  tmp = mat';

  [sim,out] = ld_constvec(sim, events, tmp(:)' );
endfunction

function [sim,out] = ld_matmul(sim, events, left_matrix, left_matrix_size, right_matrix, right_matrix_size) // PARSEDOCU_BLOCK
// %PURPOSE: matrix multiplication of two matrices
// 
// out = left_matrix * right_matrix 
//
//    left_matrix *+(left_matrix_size)  - matrix signal
//    left_matrix_size ---> matrix size [nr,nc]
//
//    right_matrix *+(right_matrix_size)  - matrix signal
//    right_matrix_size ---> matrix size [nr,nc]
//    
  btype = 69001 + 3;

  if (length(left_matrix_size) ~= 2)  then
    printf("ld_matmul: Incorrect size definition left_matrix_size\n");
    error(".");
  end
  if (length(right_matrix_size) ~= 2)  then
    printf("ld_matmul: Incorrect size definition right_matrix_size\n");
    error(".");
  end

  lm_nr = left_matrix_size(1);
  lm_nc = left_matrix_size(2);
  rm_nr = right_matrix_size(1);
  rm_nc = right_matrix_size(2);


  if (lm_nc ~= rm_nr)  then
    printf("ld_matmul: Inconsistent multiplication -> %dx%d * %dx%d!\n", lm_nr, lm_nc, rm_nr, rm_nc);
    error(".");
  end

  l_in_size = (lm_nr*lm_nc);
  r_in_size = (rm_nr*rm_nc);
  out_size  = (lm_nr*rm_nc);

  ipar = [lm_nr; lm_nc; rm_nr; rm_nc]; rpar = [];

  

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                                     insizes=[l_in_size, r_in_size], outsizes=[out_size], ...
                                     intypes=[ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT] );
 
  // libdyn_conn_equation connects multiple input signals to blocks
  [sim,blk] = libdyn_conn_equation(sim, blk, list( left_matrix, right_matrix ) );

  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim,y] = ld_matvecmul_3d(sim, events, A, v) // PARSEDOCU_BLOCK
// %PURPOSE: Matrix multiplication against a vector for 3d-vectors
// 
// y = A * v, whereby A is 3x3 and v is a vector of size 3
// 
// 
// 

  [sim,y] = ld_matmul(sim, events, A, [3,3], v, [3,1] );
endfunction

function [sim, v1, v2, v3] = ld_getvecelements_3d(sim, events, v);
// %PURPOSE: Return the elements of a 3d-vector
// 
// v = [v1, v2, v3]

  [sim,elements] = ld_demux(sim, events, vecsize=3, invec=v);
  
  v1 = elements(1); v2 = elements(2); v3 = elements(3);
endfunction

function [sim, v] = ld_setvecelements_3d(sim, events, v1, v2, v3);
// %PURPOSE: Combine three vector elements to a 3d-vector
// 
// v = [v1, v2, v3]

  [sim, v] = ld_mux(sim, events, vecsize=3, inlist=list( v1, v2, v3) );
endfunction

function [sim, out, param] = ld_leastsquares(sim, events, inplist, veclen, n_param) // PARSEDOCU_BLOCK
// %PURPOSE: estimate n linear model parameters (a,b,c...) and compute yvec_hat.
// 
// yvec_hat = a * phi0 + b * phi1 + c * phi2 ...
// Estimation of a, b, c ... minimizes the sum of squared errors (\chi^2 = \sum_i (yvec - yvec_hat)^2).
// out = yvec_hat
//
// inplist* - list(phi0*, phi1*, phi2*, ..., phi(n_param-1)*, yvec*)
// + phi0* - input vector0 (The vectors phiX form the matrix of predictor variables.)
// + phi1* - input vector1 (The vectors phiX form the matrix of predictor variables.)
// + phi2* - input vector2 (The vectors phiX form the matrix of predictor variables.)
// + ...
// + phi(n_param-1)* - input vector(n_param-1) (The vectors phiX form the matrix of predictor variables.)
// + yvec* - the original vector
//
// veclen - the number of observations which have been made (the length of each vector phiX, yvec and yvec_hat)
// n_param - the number of vectors belonging to Phi as well as the number of parameters in param*.
// out* = yvec_hat (the best prediction of yvec)
// param* - [a, b, c, ...]
//
// NOTICE: ld_leastsquares is not available by default since you need to include the GSL - GNU Scientific Library.
// Please follow the instructions which have been testet under Ubuntu 12.04. LTS:
//
// - install the required library with the following command or with a package-manager of your choice before installing ORTD:
// 'sudo apt-get install libgsl0-dev'
//
//    
  btype = 69001 + 4;

  ipar = [veclen, n_param]; rpar = [];

  if (veclen < n_param)
    error("ERROR: ld_leastsquares: Vectorsize must be greater or equal to the number of parameters to estimate.");
  end
  
  insizelist = [];
  intypelist = [];
  
  for j=1:(n_param + 1) // yvec is part of the list as well
    insizelist = [insizelist, veclen];
    intypelist = [intypelist, ORTD.DATATYPE_FLOAT];
  end
    

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                                     insizes=insizelist, outsizes=[veclen, n_param], ...
                                     intypes=intypelist, ...
                                     outtypes=[ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT] );
 
  // libdyn_conn_equation connects multiple input signals to blocks
  [sim,blk] = libdyn_conn_equation(sim, blk, inplist );

  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
  [sim,param] = libdyn_new_oport_hint(sim, blk, 1);   // 0th port
endfunction




