/*
    Copyright (C) 2010, 2011  Christian Klauer

    This file is part of OpenRTDynamics, the Real Time Dynamic Toolbox

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


#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "irpar.h"

#define irpar_get_n_ele(ipar) ( (ipar)[1] )

int irpar_get_nele( int *ipar, double *rpar)
{ // INLINE
  return irpar_get_n_ele(ipar);
}

void irpar_get_element(struct irpar_header_element_t *ret, int *ipar, double *rpar, int n)
{  
  int n_ele = irpar_get_n_ele(ipar);
  int ele_pos = 2 + n * IRPAR_HEADER_ELEMENT_LEN;
  int header_len = 2 + n_ele * IRPAR_HEADER_ELEMENT_LEN;
  int pos;
  

 // printf(". %d %x %d %x\n", ele_pos, ipar, ipar[ele_pos + 0], ret);
  ret->id = ipar[ele_pos + 0];
  ret->typ = ipar[ele_pos + 1];
  pos = ipar[ele_pos + 2]; // ipar pos
  ret->ipar_ptr = &ipar[pos+header_len];
  pos = ipar[ele_pos + 3]; // ipar pos
  ret->rpar_ptr = &rpar[pos];
  
  ret->rpar_len = ipar[ele_pos + 4];
  ret->ipar_len = ipar[ele_pos + 5];
}

int irpar_get_element_by_id(struct irpar_header_element_t *ret, int *ipar, double *rpar, int id)
{
  int i;
  
  i = 0;
  
  
  do { // Suche id
    irpar_get_element(ret, ipar, rpar, i);
    
    //printf("id = %d\n", ret->id);
    if (ret->id == id) {
    //  printf("found!\n"); //FIXME: Fill in other valules
      ret->setindex = i; // a unique index to this param set
      return 1;
    }
    
    i++;
  } while ((ret->id != id) && (i < irpar_get_n_ele(ipar))); // FIXME. Clean
  
  // not found
 // printf("not found!\n");
  ret->typ = 0;
  return 0;
}

int irpar_get_rvec(struct irpar_rvec_t *ret, int *ipar, double *rpar, int id)
{
  struct irpar_header_element_t ret_;
  
  int err = irpar_get_element_by_id(&ret_, ipar, rpar, id);
  if (err != 1)
    return -1;
  
  if (ret_.typ != IRPAR_RVEC)
    return -1;
  
  ret->v = ret_.rpar_ptr;  // vektor data
  ret->n = ret_.ipar_ptr[0]; // length
  
  return 0;
}

int irpar_get_ivec(struct irpar_ivec_t *ret, int *ipar, double *rpar, int id)
{
  struct irpar_header_element_t ret_;
  
  int err = irpar_get_element_by_id(&ret_, ipar, rpar, id);
  if (err != 1)
    return -1;
  
  if (ret_.typ != IRPAR_IVEC)
    return -1;
  
  ret->v = &(ret_.ipar_ptr[1]);  // vektor data
  ret->n = ret_.ipar_ptr[0]; // length
  
  return 0;
}


int irpar_get_tf(struct irpar_tf_t *ret, int *ipar, double *rpar, int id)
{
  struct irpar_header_element_t ret_;
  
  int err = irpar_get_element_by_id(&ret_, ipar, rpar, id);
  if (err != 1)
    return -1;
  
  if (ret_.typ != IRPAR_TF)
    return -1;
  
  ret->degnum = ret_.ipar_ptr[0]; // length
  ret->degden = ret_.ipar_ptr[1]; // length  

  //printf("%d %d\n", ret->degnum, ret->degden);

  ret->num = &ret_.rpar_ptr[0];  // 
  ret->den = &ret_.rpar_ptr[ret->degnum+1];  // 
  
  return 0;
}


int irpar_get_rmat(struct irpar_rmat_t *ret, int *ipar, double *rpar, int id)
{
  struct irpar_header_element_t ret_;
  
  int err = irpar_get_element_by_id(&ret_, ipar, rpar, id);
  if (err != 1)
    return -1;
  
  if (ret_.typ != IRPAR_RMAT)
    return -1;
  
  ret->m = ret_.ipar_ptr[0];  // 
  ret->n = ret_.ipar_ptr[1]; // 
  ret->v = ret_.rpar_ptr; // A(:)
  
  return 0;
}


int irpar_load_from_afile(int **ipar, double **rpar, int *ilen, int *rlen, char *fname_i, char *fname_r)
{
  double v;
  int d;
  int z;  
  int line_count;

 // printf("load irpar from afile %s %s\n", fname_i, fname_r);

  //
  // Load integer parameter vector
  //
 
  FILE *in = fopen ( fname_r,  "r" );
  if (in == NULL) {
    printf("ERROR: irpar.c: cannot open real vector file %s\n", fname_r);
    return -1;
  }
  
  line_count = 0;
  if ( in != NULL ) {
    char line[1024];

    while ( fgets ( line, sizeof line, in ) != NULL ) {
      size_t i = strspn ( line, " \t\n\v" );

      if ( line[i] == '#' )
        continue;

      /* Process non-comment line */
      ++line_count;
    }
  }
  
  *rlen = line_count;
  
  fclose(in);
  
  double *vec = malloc(sizeof(double)*line_count);
  
  
  
  in = fopen ( fname_r, "r" );
  if (in == NULL) {
    printf("ERROR: irpar.c: cannot open real vector file %s\n", fname_r);
    return -1;
  }

  z = 0;
  if ( in != NULL ) {
    char line[1024];

    while ( fgets ( line, sizeof line, in ) != NULL ) {
/*      size_t i = strspn ( line, " \t\n\v" );

      if ( line[i] == '#' )
        continue; */

      /* Process non-comment line */
      
      sscanf(line, "%lf", &v);
      vec[z] = v;
      ++z;
    }
  }
  fclose(in);
  

  
  //
  // Load integer parameter vector
  //
  
  
  in = fopen ( fname_i, "r" );
  if (in == NULL) {
    printf("ERROR: irpar.c: cannot open integer vector file %s\n", fname_i);
    return -1;
  }
  
  
  line_count = 0;
  if ( in != NULL ) {
    char line[1024];

    while ( fgets ( line, sizeof line, in ) != NULL ) {
      size_t i = strspn ( line, " \t\n\v" );

      if ( line[i] == '#' )
        continue;

      /* Process non-comment line */
      ++line_count;
    }
  }
  
  *ilen = line_count;
  fclose(in);
  int *vecd = malloc(sizeof(int)*line_count);
  
  
  in = fopen ( fname_i, "r" );
  if (in == NULL) {
    printf("ERROR: irpar.c: cannot open integer vector file %s\n", fname_i);
    return -1;
  }
  
  z = 0;
  if ( in != NULL ) {
    char line[1024];

    while ( fgets ( line, sizeof line, in ) != NULL ) {
      size_t i = strspn ( line, " \t\n\v" );

      if ( line[i] == '#' )
        continue;

      /* Process non-comment line */
      
      sscanf(line, "%d", &d);
      vecd[z] = d;
      ++z;
    }
  }
  fclose(in);
  
  *rpar = vec;
  *ipar = vecd;
  
  return 0;
}
 

