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
// a constant matrix
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
// matrix multiplication of two matrices
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
