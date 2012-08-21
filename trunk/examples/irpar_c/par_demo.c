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



#include <math.h>
#include "irpar.h"
//#include "irpar.c"










int main(int argc, char *argv[])
{
  
  /*
  int ipar[] = { 
    1,    
    2,    
    220,  
    1,    
    0,    
    0,    
    230,  
    3,    
    1,    
    3,    
    3,    
    2,    
    3
  };
  double rpar[] = {
    
    
     
    90,   
    100,  
    110,  
    9,    
    3,    
    5,    
    0,    
    7,    
    0,    
    2    
 
  };
  
  struct irpar_header_element_t ret;
  int i;
  
    printf("%x\n", &ret);

  irpar_get_element_by_id(&ret, ipar, rpar, 220);
  printf("irpar: id=%d, typ=%d\n", ret.id, ret.typ);
  printf("ipar = [%d,%d,%d], rpar=[%f,%f,%f,%f,%f]\n", ret.ipar_ptr[0], ret.ipar_ptr[1], ret.ipar_ptr[2], ret.rpar_ptr[0], ret.rpar_ptr[1], ret.rpar_ptr[2], ret.rpar_ptr[3], ret.rpar_ptr[4]);
  
  struct irpar_rvec_t vec;
  irpar_get_rvec(&vec, ipar, rpar, 220);
  
  
  
  printf("vec=\n");
  for (i = 0; i < vec.n; ++i)  {
    printf("|%f|\n", vec.v[i]);
  } 
  
  
  
  struct irpar_tf_t tf;
  irpar_get_tf(&tf, ipar, rpar, 230);
  
  printf("tfnum=\n");
  for (i = 0; i <= tf.degnum; ++i)  {
    printf("|%f|\n", tf.num[i]);
  } 
  
  printf("tfden=\n");
  for (i = 0; i <= tf.degden; ++i)  {
    printf("|%f|\n", tf.den[i]);
  } 
  */
  
  int *ipar_p;
  double *rpar_p;
  int ilen, rlen;
  char *fname_i = "demo.ipar";
  char *fname_r = "demo.rpar";
  
  irpar_load_from_afile(&ipar_p, &rpar_p, &ilen, &rlen, fname_i, fname_r);

  printf("->%d %f\n", ipar_p[2], rpar_p[2]);
}

