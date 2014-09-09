/*
    Copyright (C) 2010, 2011  Christian Klauer

    This file is part of OpenRTDynamics, the Real-Time Dynamics Framework

    OpenRTDynamics is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenRTDynamics is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with OpenRTDynamics.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>

#include "libdyn.h"
#include "libdyn_scicos_macros.h"

//#define GSL_INCLUDED 0

#if GSL_INCLUDED == 1
#include <gsl/gsl_multifit.h>
#endif

// FIXME raus
int compu_func_constmat(int flag, struct dynlib_block_t *block)
{
  
  //printf("comp_func gain: flag==%d\n", flag);
  int Nout = 1;
  int Nin = 0;

  double *out;

  double *rpar = libdyn_get_rpar_ptr(block);
  int *ipar = libdyn_get_ipar_ptr(block);

  int veclen = ipar[0];
  int nr = ipar[1];
  int nc = ipar[2];
  int datatype = ipar[3];
  
  double *vec = &rpar[0];
  
  switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:     
      out = (double *) libdyn_get_output_ptr(block,0);
      memcpy((void*) out, (void*) vec, veclen*sizeof(double) );
      
      return 0;
      break;
    case COMPF_FLAG_UPDATESTATES:
      return 0;
      break;
    case COMPF_FLAG_CONFIGURE:  // configure
      // BLOCKTYPE_STATIC enabled makes sure that the output calculation is called only once, since there is no input to rely on
      libdyn_config_block(block, BLOCKTYPE_STATIC, Nout, Nin, (void *) 0, 0); 
      libdyn_config_block_output(block, 0, veclen, DATATYPE_FLOAT, 0); 

      return 0;
      break;
    case COMPF_FLAG_INIT:  // init
      return 0;
      break;
    case COMPF_FLAG_DESTUCTOR: // destroy instance
      return 0;
      break;      
    case COMPF_FLAG_PRINTINFO:
      printf("This block creates a constant matrix and encodes the matrix as vector.\n");
      return 0;
      break;      
  }
}


// FIXME raus
int compu_func_vec2mat(int flag, struct dynlib_block_t *block)
{
  
  //printf("comp_func gain: flag==%d\n", flag);
  int Nout = 1;
  int Nin = 1;

  double *in;
  double *out;

  double *rpar = libdyn_get_rpar_ptr(block);
  int *ipar = libdyn_get_ipar_ptr(block);

  int size = ipar[0];
  int trans_type = ipar[1];
  
  switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:  
      in = (double *) libdyn_get_input_ptr(block,0);
      out = (double *) libdyn_get_output_ptr(block,0);
      if (trans_type <= 0) {
	out[0] = (double) 1;
	out[1] = (double) size;
      }
      else {
	out[0] = (double) size;
	out[1] = (double) 1;	
      }
      int i;
      for (i=0; i < size; ++i) {
	out[2+i] = ( in[i] );
      }
      //memcpy((void*) out, (void*) vec, veclen*sizeof(double) );
      
      return 0;
      break;
    case COMPF_FLAG_UPDATESTATES:
      return 0;
      break;
    case COMPF_FLAG_CONFIGURE:  // configure
      {
	if (size < 1) {
	  printf("size cannot be smaller than 1\n");
	  printf("size = %d\n", size);
	  return -1;
	}
	libdyn_config_block(block, BLOCKTYPE_STATIC, Nout, Nin, (void *) 0, 0);
	libdyn_config_block_input(block, 0, size, DATATYPE_FLOAT);
	libdyn_config_block_output(block, 0, size+2, DATATYPE_FLOAT,1 ); // in, intype,
      }
      return 0;
      break;
    case COMPF_FLAG_INIT:  // init
      return 0;
      break;
    case COMPF_FLAG_DESTUCTOR: // destroy instance
      return 0;
      break;      
    case COMPF_FLAG_PRINTINFO:
      printf("This block transforms a vector into a matrix.\n");
      return 0;
      break;      
  }
}

#define A(r,c)	(l_mat[ ((-1)+((r-1)*lm_nc + c)) ])
#define B(r,c)	(r_mat[ ((-1)+((r-1)*rm_nc + c)) ])

#define C(r,c)	(out[ ((-1)+((r-1)*rm_nc + c)) ])
int compu_func_matmul(int flag, struct dynlib_block_t *block)
{
  
  //printf("comp_func gain: flag==%d\n", flag);
  int Nout = 1;
  int Nin = 2;

  double *r_mat;
  double *l_mat;
  double *out;

  double *rpar = libdyn_get_rpar_ptr(block);
  int *ipar = libdyn_get_ipar_ptr(block);

  int lm_nr = ipar[0];
  int lm_nc = ipar[1];
  int rm_nr = ipar[2];
  int rm_nc = ipar[3];
  
  
  switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:  
      l_mat = (double *) libdyn_get_input_ptr(block,0);
      r_mat = (double *) libdyn_get_input_ptr(block,1);
      out = (double *) libdyn_get_output_ptr(block,0);
      
      int rows_c, colums_c, i;
      double temp;
      for (rows_c=1; rows_c <= lm_nr; ++rows_c) {
	for (colums_c=1; colums_c <= rm_nc; ++colums_c){
	  temp = 0;
// 	  printf("c%d,%d(%d) = ", rows_c, colums_c, ((-1)+((rows_c-1)*rm_nc + colums_c)) );
	  for (i=1; i <= lm_nc; ++i){
	      temp += (double) A(rows_c,i)*B(i,colums_c);
// 	      printf(" + a%d,%d[%d](%f) * b%d,%d[%d](%f)", rows_c, i, ((-1)+((rows_c-1)*lm_nc + i)), A(rows_c,i), i, colums_c, ((-1)+((rows_c-1)*rm_nc + colums_c)), B(i, colums_c) );
	  }
	  
	  C(rows_c,colums_c) = (double) temp;
// 	  printf(" = %f\n", C(rows_c,colums_c));
	}
      }
      //memcpy((void*) out, (void*) vec, veclen*sizeof(double) );
      
      return 0;
      break;
    case COMPF_FLAG_UPDATESTATES:
      return 0;
      break;
    case COMPF_FLAG_CONFIGURE:  // configure
      {
	if (rm_nr < 1 || rm_nc < 1 || lm_nr < 1 || lm_nc < 1) {
	  fprintf(stderr, "matmul: matrix size cannot be smaller than 1\n");
	  fprintf(stderr, "matmul:  -> out[%dx%d] = left_mat[%dx%d] * right_mat[%dx%d]\n", lm_nr, rm_nc, lm_nr, lm_nc, rm_nr, rm_nc);
	  return -1;
	}
	libdyn_config_block(block, BLOCKTYPE_STATIC, Nout, Nin, (void *) 0, 0);
	libdyn_config_block_input(block, 0, ((lm_nr*lm_nc)), DATATYPE_FLOAT);
	libdyn_config_block_input(block, 1, ((rm_nr*rm_nc)), DATATYPE_FLOAT);
	
	libdyn_config_block_output(block, 0, ((lm_nr*rm_nc)), DATATYPE_FLOAT,1 ); // in, intype,
      }
      return 0;
      break;
    case COMPF_FLAG_INIT:  // init
      return 0;
      break;
    case COMPF_FLAG_DESTUCTOR: // destroy instance
      return 0;
      break;      
    case COMPF_FLAG_PRINTINFO:
      printf("This block performs a matrix multiplication.\n");
      return 0;
      break;      
  }
}

#if GSL_INCLUDED == 1

#define C(i) (gsl_vector_get(workptr->c,(i)))

struct lsq_block_work {
	  gsl_matrix *X, *cov;
	  gsl_vector *y, *c;
	  gsl_multifit_linear_workspace *work;
};

int compu_func_leastsquares(int flag, struct dynlib_block_t *block)
{
  
  //printf("comp_func gain: flag==%d\n", flag);

  double *rpar = libdyn_get_rpar_ptr(block);
  int *ipar = libdyn_get_ipar_ptr(block);

  int n_observations = ipar[0];  // Length of each Vector. Needs to be equal for each Vector!
  
  int j = 0, i = 0, n = ipar[1];
  double chisq, *parameter;
  double *out, *vec, inpmatrix[n + 1][n_observations];
  
  int Nout = 2;
  int Nin = n + 1;
  
  switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:  
    {
      struct lsq_block_work *workptr = (void*) libdyn_get_work_ptr(block);
      
      for(i = 0; i <= n; i++) { // yvec is part of the input as well
	vec = (double *) libdyn_get_input_ptr(block,i);
	
	memcpy(&inpmatrix[i][0], vec, (sizeof(double) * n_observations));
	for(j = 0; j < n_observations; j++) {
	  if(i < n) {
	    gsl_matrix_set (workptr->X, j, i, vec[j]);
	  }
	  else {
	    gsl_vector_set (workptr->y, j, vec[j]);
	  }
	}
      }
      
      out = (double *) libdyn_get_output_ptr(block,0);
      parameter = (double *) libdyn_get_output_ptr(block,1);
      for(j = 0; j < n_observations; j++) {
	out[j] = 0;
      }

      gsl_multifit_linear (workptr->X, workptr->y, workptr->c, workptr->cov, &chisq, workptr->work);
      
      for (i = 0; i < n; i++) {
	parameter[i] = C(i);
	for(j = 0; j < n_observations; j++) {
	  out[j] = out[j] + (parameter[i] * inpmatrix[i][j]);
	} 
      }
    }
      
      return 0;
      break;
    case COMPF_FLAG_UPDATESTATES:
      return 0;
      break;
    case COMPF_FLAG_CONFIGURE:  // configure
      {
	if (n_observations < n) {
	  fprintf(stderr, "least squares algorithm: Number of observations (vector-length) cannot be smaller than the number of parameters!\n");

	  return -1;
	}
	libdyn_config_block(block, BLOCKTYPE_STATIC, Nout, Nin, (void *) 0, 0);
	for(i = 0; i <= n; i++) { // yvec is part of the input as well
	  libdyn_config_block_input(block, i, n_observations, DATATYPE_FLOAT);
	}	
	
	libdyn_config_block_output(block, 0, n_observations, DATATYPE_FLOAT,1 ); // in, intype,
	libdyn_config_block_output(block, 1, n, DATATYPE_FLOAT,1 ); // in, intype,
      }
      return 0;
      break;
    case COMPF_FLAG_INIT:  // init
{
      struct lsq_block_work *workptr = malloc(sizeof(struct lsq_block_work));

      workptr->X = gsl_matrix_alloc (n_observations, n);
      workptr->y = gsl_vector_alloc (n_observations);

      workptr->c = gsl_vector_alloc (n);
      workptr->cov = gsl_matrix_alloc (n, n);
      
      workptr->work = gsl_multifit_linear_alloc (n_observations, n);
      
      libdyn_set_work_ptr(block, (void *) workptr);
}
      return 0;
      break;
    case COMPF_FLAG_DESTUCTOR: // destroy instance
{

      struct lsq_block_work *workptr = (void*) libdyn_get_work_ptr(block);

      gsl_multifit_linear_free (workptr->work);
      gsl_matrix_free (workptr->X);
      gsl_vector_free (workptr->y);
      gsl_vector_free (workptr->c);
      gsl_matrix_free (workptr->cov);
      
      free(workptr);

}
      return 0;
      break;      
    case COMPF_FLAG_PRINTINFO:
      printf("This block performs a least squares algorithm.\n");
      return 0;
      break;      
  }
}

#endif


//#include "block_lookup.h"

int libdyn_module_matrix_siminit(struct dynlib_simulation_t *sim, int bid_ofs)
{
  // printf("libdyn module siminit function called\n"); 
  
  // Register my blocks to the given simulation
  int blockid = 69001;
  libdyn_compfnlist_add(sim->private_comp_func_list, blockid + 1, LIBDYN_COMPFN_TYPE_LIBDYN, &compu_func_constmat);
  libdyn_compfnlist_add(sim->private_comp_func_list, blockid + 2, LIBDYN_COMPFN_TYPE_LIBDYN, &compu_func_vec2mat);
  libdyn_compfnlist_add(sim->private_comp_func_list, blockid + 3, LIBDYN_COMPFN_TYPE_LIBDYN, &compu_func_matmul);
#if GSL_INCLUDED == 1
  libdyn_compfnlist_add(sim->private_comp_func_list, blockid + 4, LIBDYN_COMPFN_TYPE_LIBDYN, &compu_func_leastsquares);
#endif
  
#ifdef DEBUG
  printf("Added MATRIX module\n");
#endif  
  
}

//} // extern "C"
