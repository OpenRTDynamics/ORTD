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
 * libdyn_blocks.h - Library for simple realtime controller implementations
 *
 *
 * Author: Christian Klauer 2009-2010
 *
 *
 *
 *
 */

int compu_func_bilinearint(int flag, struct dynlib_block_t *block);
int compu_func_TP1(int flag, struct dynlib_block_t *block);
struct dynlib_block_t *new_TP1_block_(struct dynlib_simulation_t *sim, double z_oo);

// int compu_func_zTF(int flag, struct dynlib_block_t *block);
// struct dynlib_block_t *new_zTF_block_(struct dynlib_simulation_t *sim, int degn, int degd, double *qn, double *qd);


int compu_func_sum(int flag, struct dynlib_block_t *block);
struct dynlib_block_t *new_sum_block_(struct dynlib_simulation_t *sim, double *c);
int compu_func_gain(int flag, struct dynlib_block_t *block);
struct dynlib_block_t *new_gain_block_(struct dynlib_simulation_t *sim, double *c);
int compu_func_switch(int flag, struct dynlib_block_t *block);
struct dynlib_block_t *new_switch_block_(struct dynlib_simulation_t *sim);
int compu_func_2to1_event_switch(int flag, struct dynlib_block_t *block);
struct dynlib_block_t *new_2to1_event_switch_block(struct dynlib_simulation_t *sim, int *initial_state);
int compu_func_sat(int flag, struct dynlib_block_t *block);
struct dynlib_block_t *new_sat_block_(struct dynlib_simulation_t *sim, double *c);
int compu_func_const(int flag, struct dynlib_block_t *block);
struct dynlib_block_t *new_const_block_(struct dynlib_simulation_t *sim, double *c);
int compu_func_mul(int flag, struct dynlib_block_t *block);
struct dynlib_block_t *new_mul_block_(struct dynlib_simulation_t *sim, int *d);
int compu_func_fn_gen(int flag, struct dynlib_block_t *block);
struct dynlib_block_t *new_fn_gen_block_(struct dynlib_simulation_t *sim, int *shape);
int compu_func_ser2par(int flag, struct dynlib_block_t *block);
struct dynlib_block_t *new_ser2par_block(struct dynlib_simulation_t *sim, int *len);
int compu_func_play_block(int flag, struct dynlib_block_t *block);
struct dynlib_block_t *new_play_block(struct dynlib_simulation_t *sim, int *par, double *r);
int compu_func_lookup(int flag, struct dynlib_block_t *block);
struct dynlib_block_t *new_lookup_block(struct dynlib_simulation_t *sim, int *ipar, double *rpar);
int compu_func_filedump(int flag, struct dynlib_block_t *block);
struct dynlib_block_t *new_filedump_block(struct dynlib_simulation_t *sim, int *ipar, double *rpar);
int compu_func_compare(int flag, struct dynlib_block_t *block);
int compu_func_delay(int flag, struct dynlib_block_t *block);
int compu_func_flipflop(int flag, struct dynlib_block_t *block);
int compu_func_printf(int flag, struct dynlib_block_t *block);

int compu_func_interface(int flag, struct dynlib_block_t *block);
struct dynlib_block_t *new_interface_block(struct dynlib_simulation_t *sim, int *len);
int compu_func_super(int flag, struct dynlib_block_t *block);
struct dynlib_block_t *new_super_block_(struct dynlib_simulation_t *sim, struct dynlib_simulation_t *content);




int compu_func_scope(int flag, struct dynlib_block_t *block);



