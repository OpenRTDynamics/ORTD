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


struct siso_ilc_mat_alg_t {
  int Nsamples;
  
  double *r;
  double *Q;
  double *L;
  int mtot;
  
  double *e;
  double *u_m1, *u_m;
  double *tmp;
  
  
  
  int z;
  int active;
};

#define sqmatrix_element(A,m,n,len) ( (A)[(m) + ((n)-1)*(len) - 1] )

double *ilc_reserve_vec(int Nsamples);
void linear_combination_vec(double a, double b, double *u1, double *u2, double *out, int len);
void put_zero_vec(double *u, int len);
void add_to_vec(double *u, double *y, int len);
void flip_vec(double *u, int len);
void dump_vec(double *u, int len);

void funky_filter(double *u, double *y, double *q, int len);
void funky_cvmatrix_filter(double *cvmatrix, double *u, double *y, int len);



struct siso_ilc_mat_alg_t *siso_ilc_new(int Nsamples, double *r, double *Q, double *L, int mtot);
int siso_ilc_del(struct siso_ilc_mat_alg_t * ilc);
int siso_ilc_init_sampling(struct siso_ilc_mat_alg_t * ilc);
int siso_ilc_update(struct siso_ilc_mat_alg_t * ilc);
double siso_ilc_sample(struct siso_ilc_mat_alg_t * ilc, double in, int update_states);
double siso_ilc_get_ref(struct siso_ilc_mat_alg_t * ilc);


struct siso_sampler_t {
  int Nsamples;
  
  double *r;
  
  int z;
  int active;
  
  int hold_last_value;
  int mute_afterstop;
  
  int stopped_by_command; // 1 wenn nachdem siso_sampler_stop_sampling aufgerufen wird
                          // falls Ende durch ein Ende des Datensatzes eingeleitet wird
			  // ist dies 0
			  
  int never_activated;
};


struct siso_sampler_t *siso_sampler_new(int Nsamples, double *r);
void siso_sampler_special_cfg(struct siso_sampler_t * sampler, int hold_last_value, int mute_afterstop);
int siso_sampler_del(struct siso_sampler_t * sampler);
int siso_sampler_init_sampling(struct siso_sampler_t * sampler);
int siso_sampler_stop_sampling(struct siso_sampler_t * sampler);
double siso_sampler_play(struct siso_sampler_t * sampler, int update_states);
