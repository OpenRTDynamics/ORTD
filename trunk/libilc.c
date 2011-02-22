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



/*
 * ILC & Sampling Library
 *
 *
 *
 */

#include <pthread.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "libilc.h"

#define debug_mode 1

double *ilc_reserve_vec(int Nsamples)
{
  return (double *) malloc(sizeof(double) * Nsamples);
}

void linear_combination_vec(double a, double b, double *u1, double *u2, double *out, int len)
{
  int i;

  for (i=0; i<len; ++i)
    out[i] = a*u1[i] + b*u2[i];
}

void put_zero_vec(double *u, int len)
{
  int i;

  for (i=0; i<len; ++i)
    u[i] = 0;
}

void copy_vec(double *u, double *y, int len)
{
  int i;

  for (i=0; i<len; ++i) {
    y[i] = u[i];
   // if (debug_mode==1) printf("copied %f\n", y[i]);
  }
}

void add_to_vec(double *u, double *y, int len)
{
  int i;

  for (i=0; i<len; ++i) {
    y[i] += u[i];
    //if (debug_mode==1) printf("added %f\n", y[i]);
  }
}

void flip_vec(double *u, int len)
{
  int i;

  if (debug_mode==1) printf("flip_vec: len=%i, len/2=%i\n", 7, 7/2); // Test

  for (i = 0; i < len/2; ++i) {  // FIXME: len/2 soll abrunden
	double tmp;
	tmp = u[i];
	u[i] = u[len-1-i];
	u[len-1-i] = tmp;
  }
}

void dump_vec(double *u, int len)
{
  int i;

  if (debug_mode==1) printf("dump_vec: ");
  for (i=0; i<len; ++i) {
    if (debug_mode==1) printf(" %f, ", u[i]);
  } 
  if (debug_mode==1) printf("\n");
}


/*
void funky_filtfilt_filter(double *u, double *y, double *q, int len) // 
{
	funky_filter(u, y, q, len); // Vorwärts
	flip_vec(y);
	
}
*/

void funky_filter(double *u, double *y, double *q, int len)
/* 
 * nichtkausaler Filter
 * q - Impulsantwort der Länge len*2 + 1
 * y - Filterausgang wird auf y aufaddiert
 */

#define funky_filter_transform_q(k) (q[len + k])  // q_(0) ist direkter Durchgriff

{
  int i,j;

  // Faltung y(i) = sum_0_len ( q_(i-j) * u(j) )
  if (debug_mode==1) printf("funky_filter: q(-1) = %f, q(0) = %f, q(1) = %f\n", funky_filter_transform_q(-1), funky_filter_transform_q(0), funky_filter_transform_q(1));

  for (i = 0; i<len; ++i) {
    float sum = 0;

    for (j = 0; j < len; ++j) {
      sum += funky_filter_transform_q(i-j) * u[j];
    }
    y[i] += sum;

    if (debug_mode==1) printf("funky_filter: u[%i]=%f, y[%i]=%f\n", i, u[i], i, y[i]);
  }
}

void funky_cvmatrix_filter(double *cvmatrix, double *u, double *y, int len)


{
  int z; // Zeile
  int s; // Spalte


//   printf("fcvf: Element 1,1: %f; E 1,2: %f\n", sqmatrix_element(cvmatrix, 1, 1, len), sqmatrix_element(cvmatrix, 1, 2, len));
//   printf("fcvf: Element len,len: %f; E len-1,len: %f\n", sqmatrix_element(cvmatrix, len, len, len), sqmatrix_element(cvmatrix, len-1, len, len));


  //printf("Matrix mult:\n");
  for (z = 1; z<=len; ++z) {
    double sum = 0;
    for (s = 1; s<=len; ++s) {
      sum += sqmatrix_element(cvmatrix, z, s, len) * u[s-1];
      //sum +=  u[z-1];
    }

    y[z-1] = sum;
    //printf("y(%i) = %f", z-1, sum);
  }
  //printf("\nend Matrix mult\n");
}


//
// Sampler class
//


struct siso_sampler_t *siso_sampler_new(int Nsamples, double *r)
{
  struct siso_sampler_t *sampler = (struct siso_sampler_t *) malloc(sizeof(struct siso_sampler_t));
  
  sampler->Nsamples = Nsamples;
  sampler->r = r;
 
  sampler->z = 0; // sample counter
  sampler->active = 0;
  
  sampler->hold_last_value = 0;
  sampler->mute_afterstop = 1;
  
  sampler->never_activated = 1;
  
  return sampler;
}

void siso_sampler_special_cfg(struct siso_sampler_t * sampler, int hold_last_value, int mute_afterstop)
{
  sampler->hold_last_value = hold_last_value;
  sampler->mute_afterstop = mute_afterstop;  
}

int siso_sampler_del(struct siso_sampler_t * sampler)
{
  free(sampler);
}

int siso_sampler_init_sampling(struct siso_sampler_t * sampler)
{
  //printf("Starting sampler sampling\n");
  
  sampler->never_activated = 0;
  
  sampler->z = 0;
  sampler->active = 1;
  sampler->stopped_by_command = 0;
}

int siso_sampler_stop_sampling(struct siso_sampler_t * sampler)
{
  //printf("Stopping sampler sampling\n");
  
  sampler->active = 0;
  sampler->stopped_by_command = 1; // aktiv gestoppt
}


double siso_sampler_play(struct siso_sampler_t * sampler, int update_states)
{
  if (sampler->active == 1 && sampler->z >= sampler->Nsamples) {// Stop, when the traject is over
    //printf("sampler ended\n");
    sampler->active = 0; 
    sampler->stopped_by_command = 0; // passiv gestoppt
  }
  
  if (sampler->active == 1) {
    double out = sampler->r[sampler->z];
    if (update_states == 1)
      ++sampler->z;
  
    return out;

  } else  { // Trajectory is not played
    if (sampler->mute_afterstop == 1 && sampler->stopped_by_command == 1)
      return 0.0;
    if (sampler->hold_last_value == 1 && sampler->never_activated == 0)
      return sampler->r[sampler->Nsamples-1];
      
    return 0.0;
  }
}



struct siso_ilc_mat_alg_t *siso_ilc_new(int Nsamples, double *r, double *Q, double *L, int mtot)
{
  struct siso_ilc_mat_alg_t *ilc = (struct siso_ilc_mat_alg_t *) malloc(sizeof(struct siso_ilc_mat_alg_t));
  
  ilc->Nsamples = Nsamples;
  ilc->r = r;
  ilc->Q = Q;
  ilc->L = L;
  ilc->mtot = mtot;
  
  ilc->e = ilc_reserve_vec(Nsamples);
  ilc->u_m1 = ilc_reserve_vec(Nsamples);
  ilc->u_m = ilc_reserve_vec(Nsamples);
  ilc->tmp = ilc_reserve_vec(Nsamples);
  
  ilc->z = 0; // sample counter
  ilc->active = 0;
  
  //put_zero_vec(ilc->u_m, ilc->Nsamples);
  
  
  int i;
  
  //put_zero_vec(ilc->u_m, ilc->Nsamples);
  for (i = 0; i < ilc->Nsamples; ++i)
    ilc->u_m[i] = 0.0;

  
  //printf("New ilc len=%d, mtot=%d, r(0)=%f, Q(0,0)=%f\n", ilc->Nsamples, ilc->mtot, ilc->r[0], ilc->Q[0]);
  
  return ilc;
}

int siso_ilc_del(struct siso_ilc_mat_alg_t * ilc)
{
  free(ilc->e);
  free(ilc->u_m1);
  free(ilc->u_m);
  free(ilc->tmp);
  free(ilc);
}

int siso_ilc_init_sampling(struct siso_ilc_mat_alg_t * ilc)
{
 // printf("Starting ilc sampling\n");
  
  ilc->z = 0;
  ilc->active = 1;
  
  int i;
  
  //put_zero_vec(ilc->u_m, ilc->Nsamples);
  /*for (i = 0; i < ilc->Nsamples; ++i)
    ilc->u_m[i] = 0;*/
}

int siso_ilc_update(struct siso_ilc_mat_alg_t * ilc)
{
  //printf("updating u_m\n");
  
  
  funky_cvmatrix_filter(ilc->L, ilc->e, ilc->tmp, ilc->Nsamples); // tmp=L*e
  
  copy_vec(ilc->u_m, ilc->u_m1, ilc->Nsamples); // u_m1 = u_m
  add_to_vec(ilc->u_m, ilc->tmp, ilc->Nsamples); // tmp=tmp+u_m
  
  funky_cvmatrix_filter(ilc->Q, ilc->tmp, ilc->u_m, ilc->Nsamples); // u_m=Q*tmp
  
}

double siso_ilc_sample(struct siso_ilc_mat_alg_t * ilc, double in, int update_states)
{
  double out;
  
  //printf("ilc sample update st=%d\n", update_states);
  if (ilc->active == 1) {
  
    if (ilc->z >= ilc->Nsamples + ilc->mtot) {// Stop, when the traject is over
        printf("ILC ended\n");
      siso_ilc_update(ilc);
     // dump_vec(ilc->u_m, ilc->Nsamples);
      
      ilc->active = 0; 
      
      return 0.0;
    }
  
    out = ilc->u_m[ilc->z];
  
  
    if (ilc->z >= ilc->mtot && update_states == 1)
      ilc->e[ilc->z - ilc->mtot] = ilc->r[ilc->z - ilc->mtot] - in; // e=r-y
  
    if (update_states == 1)
      ++ilc->z;
  
  
    return out;
  } else 
    return 0.0;

}

double siso_ilc_get_ref(struct siso_ilc_mat_alg_t * ilc)
{
  return ilc->r[ilc->z];
}