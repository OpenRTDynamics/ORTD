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
 * libdyn_blocks.c - Library for simple realtime controller implementations
 *
 *
 * Author: Christian Klauer 2009-2010
 *
 *
 *
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "libdyn.h"
#include "libdyn_blocks.h"

// Filewriter
#include "log.h"


#define PRINT_DEBUG_HINTS

// FIXME REMOVE THIS
int compu_func_bilinearint(int flag, struct dynlib_block_t *block)
{
// printf("comp_func bilint: flag==%d\n", flag);
    int Nout = 1;
    int Nin = 1;

    double Qn[2], Qd[2], T_a;
    struct dynlib_filter_t *filter;
    double *inp;
    double *out;
    double tmp;

    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
        inp = (double *) block->inlist[0].data;
        out = (double *) block->outlist[0].data;
        filter = (struct dynlib_filter_t *) block->work;
        //  printf("filter ptr = %d, inp=%f\n", filter, *inp);
        tmp = libdyn_out(filter, *inp, 0);
        // printf("INTOUT: %f\n", tmp);
        *out = tmp;
        return 0;
        break;
    case COMPF_FLAG_UPDATESTATES:
        inp = (double *) block->inlist[0].data;
        out = (double *) block->outlist[0].data;
        filter = (struct dynlib_filter_t *) block->work;
        libdyn_out(filter, *inp, 1);
        return 0;
        break;
    case COMPF_FLAG_CONFIGURE:  // configure
        libdyn_config_block(block, BLOCKTYPE_DYNAMIC, Nout, Nin, (void *) 0, 0);
        libdyn_config_block_input(block, 0, 1, DATATYPE_FLOAT); // in, intype,
        libdyn_config_block_output(block, 0, 1, DATATYPE_FLOAT, 1);

        T_a = 1;
        Qn[0] = T_a/2;
        Qn[1] = T_a/2;
        Qd[0] = -1;
        Qd[1] = 1; // Bilineare Trafo

        filter = libdyn_new_filter(1);
        libdyn_load_cf(filter, Qn, Qd, 1);
        libdyn_null_states(filter);
        block->work = (void *) filter;

        return 0;
        break;
    case COMPF_FLAG_INIT:  // init
        return 0;
        break;
    case COMPF_FLAG_DESTUCTOR: // destroy instance
        filter = (struct dynlib_filter_t *) block->work;
        libdyn_delete_filter(filter);

        return 0;
        break;
    }
}




// FIXME REMOVE THIS
int compu_func_TP1(int flag, struct dynlib_block_t *block)
{
// printf("comp_func TP1: flag==%d\n", flag);
    int Nout = 1;
    int Nin = 1;

    double Qn[2], Qd[2], T_a;
    struct dynlib_filter_t *filter;
    double *inp;
    double *out;
    double tmp;

    double *rpar = libdyn_get_rpar_ptr(block);
    double z_oo = rpar[0];

    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
        inp = (double *) libdyn_get_input_ptr(block,0);
        out = (double *) libdyn_get_output_ptr(block,0);
        filter = (struct dynlib_filter_t *) libdyn_get_work_ptr(block);
        //  printf("filter ptr = %d, inp=%f\n", filter, *inp);
        tmp = libdyn_out(filter, *inp, 0);
        //  printf("INTOUT: %f\n", tmp);
        *out = tmp;
        return 0;
        break;
    case COMPF_FLAG_UPDATESTATES:
        inp = (double *) libdyn_get_input_ptr(block,0);
        out = (double *) libdyn_get_output_ptr(block,0);
        filter = (struct dynlib_filter_t *) libdyn_get_work_ptr(block);
        libdyn_out(filter, *inp, 1);
        return 0;
        break;
    case COMPF_FLAG_CONFIGURE:  // configure
        libdyn_config_block(block, BLOCKTYPE_DYNAMIC, Nout, Nin, (void *) 0, 0);  // no dfeed
        libdyn_config_block_input(block, 0, 1, DATATYPE_FLOAT); // in, intype,
        libdyn_config_block_output(block, 0, 1, DATATYPE_FLOAT, 0);

        Qn[0] = 1-z_oo;
        Qn[1] = 0;
        Qd[0] = -z_oo;
        Qd[1] = 0; // 1-z_oo / z - z_oo

        filter = libdyn_new_filter(1);
        libdyn_load_cf(filter, Qn, Qd, 0);
        libdyn_null_states(filter);
        libdyn_set_work_ptr(block, (void *) filter);

        return 0;
        break;
    case COMPF_FLAG_INIT:  // init
        return 0;
        break;
    case COMPF_FLAG_DESTUCTOR: // destroy instance
        filter = (struct dynlib_filter_t *) libdyn_get_work_ptr(block);
        libdyn_delete_filter(filter);

        return 0;
        break;
    }
}

#define new_TP1_block(sim, z_oo_ptr) (libdyn_new_block((sim), &compu_func_TP1, 0, (z_oo_ptr), 0,  0));

struct dynlib_block_t *new_TP1_block_(struct dynlib_simulation_t *sim, double z_oo)
{
    // new lowpass of first order
    // z_oo (rpar) is only available for initialisation

    struct dynlib_block_t *tp1 = libdyn_new_block(sim, &compu_func_TP1, 0, &z_oo, 0,  0);

    return tp1;
}

/*
// FIXME: Reimplement this
int compu_func_zTF(int flag, struct dynlib_block_t *block) // FIXME: Rewrite the whole
{
    //printf("comp_func zTF: flag==%d\n", flag);
    int err;

    int Nout = 1;
    int Nin = 1;

    struct dynlib_filter_t *filter;
    double *inp;
    double *out;

    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);

    int dfeed = 0;

// printf("degn = %d, degd = %d\n", degn, degd);



    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
        inp = (double *) libdyn_get_input_ptr(block,0);
        out = (double *) libdyn_get_output_ptr(block,0);
        filter = (struct dynlib_filter_t *) libdyn_get_work_ptr(block);
        *out = libdyn_out(filter, *inp, 0);

//             printf("filter=%p\n", filter);

//  	printf("OOUT in=%f, out=%f\n", inp[0], out[0]);


        return 0;
        break;
    case COMPF_FLAG_UPDATESTATES:

//          	  printf(" SUP\n");

        inp = (double *) libdyn_get_input_ptr(block,0);
        out = (double *) libdyn_get_output_ptr(block,0);
        filter = (struct dynlib_filter_t *) libdyn_get_work_ptr(block);

        if (__libdyn_event_check(block, 1)) {
#ifdef PRINT_DEBUG_HINTS
            printf("nullstates irparid=%d\n", block->irpar_config_id);
#endif
            libdyn_null_states(filter);
        }
        if (__libdyn_event_check(block, 0)) {
            double tmp = libdyn_out(filter, *inp, 1);

// 	printf("filter=%p\n", filter);

//         if (block->irpar_config_id == 203) {
//     	  printf("OUT in=%f, out=%f SUP\n", inp[0], out[0]);
//         }


            //      if (block->irpar_config_id == 202)
            //	  printf("SUP in=%f, out=%f\n", inp[0], tmp);

//       if (block->irpar_config_id == 219) {
//  	printf("SOUT in=%f, out=%f\n", inp[0], tmp);
//       }


        }

        return 0;
        break;
    case COMPF_FLAG_RESETSTATES:
//       printf("00000000\n");
        filter = (struct dynlib_filter_t *) libdyn_get_work_ptr(block);
        libdyn_null_states(filter);

        out = (double *) libdyn_get_output_ptr(block,0);
        *out = 0;

        return 0;
        break;
    case COMPF_FLAG_CONFIGURE:  // configure
    {
        int degn = (ipar[0]);
        int degd = (ipar[1]);

        double *Qn = &(rpar[0]);
        double *Qd = &(rpar[degn+1]);

        if (degn == degd)
            dfeed = 1;

        libdyn_config_block(block, BLOCKTYPE_DYNAMIC, Nout, Nin, (void *) 0, 0);  // no dfeed
        libdyn_config_block_input(block, 0, 1, DATATYPE_FLOAT); // in, intype,
        //  printf("Setting dfeed to %d\n", dfeed);
        libdyn_config_block_output(block, 0, 1, DATATYPE_FLOAT, dfeed);


    }

    return 0;
    break;
    case COMPF_FLAG_INIT:  // init
    {
        int degn = (ipar[0]);
        int degd = (ipar[1]);

        double *Qn = &(rpar[0]);
        double *Qd = &(rpar[degn+1]);
	
	filter = libdyn_new_filter(degd);
        libdyn_load_cf(filter, Qn, Qd, degn);
        libdyn_null_states(filter);
        libdyn_set_work_ptr(block, (void *) filter);
    }

      

        return 0;
        break;
    case COMPF_FLAG_DESTUCTOR: // destroy instance
        filter = (struct dynlib_filter_t *) libdyn_get_work_ptr(block);
        libdyn_delete_filter(filter);

        return 0;
        break;

    case COMPF_FLAG_PRINTINFO:
        printf("I'm a tf block: \n");
       // filter = (struct dynlib_filter_t *) libdyn_get_work_ptr(block);
       // libdyn_print_cf(filter);
        return 0;
        break;
    }
}*/

/*
struct dynlib_block_t *new_zTF_block_(struct dynlib_simulation_t *sim, int degn, int degd, double *qn, double *qd)
{
    // new lowpass of first order
    // z_oo (rpar) is only available for initialisation

    int i;

    int ipar[2];
    double rpar[100];

    ipar[0] = degn;
    ipar[1] = degd;

// printf("ipar = %d\n", &ipar);
    //printf("ipar_ = %d\n", ipar[0]);


// printf("degn=%d, degd=%d, qd=[%f,%f,%f,%f]\n", degn, degd, qd[0], qd[1], qd[2], qd[3]);
    double normfac = qd[degd]; // Highest denom coefficent

    if (degn > degd) {
        printf("libdyn: zTF: Cannot create acausal filter\n");
        return 0;
    } else {

        for (i = 0; i <= degn; ++i)  // deg+1 coefficients
            rpar[i] = qn[i] / normfac; // [0..degn]

        for (i = 0; i <= degd; ++i)
            rpar[i+degn+1] = qd[i] / normfac;

        struct dynlib_block_t *ztf = libdyn_new_block(sim, &compu_func_zTF, ipar, rpar, 0,  0);


        return ztf;
    }
}*/



int compu_func_sum(int flag, struct dynlib_block_t *block)
{
// printf("comp_func sum: flag==%d\n", flag);
    int Nout = 1;
    int Nin = 2;

    double *inp1, *inp2;
    double *out;

    double *rpar = libdyn_get_rpar_ptr(block);
    double c_a = rpar[0];
    double c_b = rpar[1];

    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
        inp1 = (double *) libdyn_get_input_ptr(block,0);
        inp2 = (double *) libdyn_get_input_ptr(block,1);
        out = (double *) libdyn_get_output_ptr(block,0);

        *out = c_a * *inp1 + c_b * *inp2;

        return 0;
        break;
    case COMPF_FLAG_UPDATESTATES:
        return 0;
        break;
    case COMPF_FLAG_CONFIGURE:  // configure
        libdyn_config_block(block, BLOCKTYPE_STATIC, Nout, Nin, (void *) 0, 0);
        libdyn_config_block_input(block, 0, 1, DATATYPE_FLOAT); // in, intype,
        libdyn_config_block_input(block, 1, 1, DATATYPE_FLOAT); // in, intype,
        libdyn_config_block_output(block, 0, 1, DATATYPE_FLOAT, 1);

        return 0;
        break;
    case COMPF_FLAG_INIT:  // init
        return 0;
        break;
    case COMPF_FLAG_DESTUCTOR: // destroy instance
        return 0;
        break;
    }
}

//#define new_sum_block(sim, c_a, c_b) (libdyn_new_block((sim), &compu_func_TP1, 0, (z_oo_ptr), 0,  0));


struct dynlib_block_t *new_sum_block_(struct dynlib_simulation_t *sim, double *c)
{
    //
    // c_a, b_b (rpar) is only available for initialisation

//  printf("New sum: %f, %f\n", c[0],c[1]);
    struct dynlib_block_t *sum = libdyn_new_block(sim, &compu_func_sum, 0, c, 0,  0);

    return sum;
}




int compu_func_gain(int flag, struct dynlib_block_t *block)
{
    //printf("comp_func gain: flag==%d\n", flag);
    int Nout = 1;
    int Nin = 1;

    double *inp1;
    double *out;

    double *rpar = libdyn_get_rpar_ptr(block);
    double c = rpar[0];

    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
        inp1 = (double *) libdyn_get_input_ptr(block,0);
        out = (double *) libdyn_get_output_ptr(block,0);

        *out = c * *inp1;
        return 0;
        break;
    case COMPF_FLAG_UPDATESTATES:
        return 0;
        break;
    case COMPF_FLAG_CONFIGURE:  // configure
        libdyn_config_block(block, BLOCKTYPE_STATIC, Nout, Nin, (void *) 0, 0);
        libdyn_config_block_input(block, 0, 1, DATATYPE_FLOAT); // in, intype,
        libdyn_config_block_output(block, 0, 1, DATATYPE_FLOAT, 1);

        //printf("New sum block %f, %f\n", c_a, c_b);
        return 0;
        break;
    case COMPF_FLAG_INIT:  // init
        return 0;
        break;
    case COMPF_FLAG_DESTUCTOR: // destroy instance
        return 0;
        break;
    }
}
struct dynlib_block_t *new_gain_block_(struct dynlib_simulation_t *sim, double *c)
{
    //
    // c (rpar) is only available for initialisation

    struct dynlib_block_t *gain = libdyn_new_block(sim, &compu_func_gain, 0, c, 0,  0);

    return gain;
}


//
// A switching Block
// inputs = [control_in, signal_in]
// if control_in > 0 : signal_in is directed to output 1; output_2 is set to zero
// if control_in < 0 : signal_in is directed to output 2; output_1 is set to zero
//

int compu_func_switch(int flag, struct dynlib_block_t *block)
{
    //printf("comp_func switch: flag==%d\n", flag);
    int Nout = 2;
    int Nin = 2;

    double *inp;
    double *control_in;
    double *out1;
    double *out2;

    double *rpar = libdyn_get_rpar_ptr(block);

    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
        control_in = (double *) libdyn_get_input_ptr(block,0);
        inp = (double *) libdyn_get_input_ptr(block,1);
        out1 = (double *) libdyn_get_output_ptr(block,0);
        out2 = (double *) libdyn_get_output_ptr(block,1);

        if (*control_in > 0) {
            *out1 = *inp;
            *out2 = 0.0;
        } else {
            *out2 = *inp;
            *out1 = 0.0;
        }

//     printf("[%f, %f, %f, %f]\n", *control_in, *inp, *out1, *out2);

        return 0;
        break;
    case COMPF_FLAG_UPDATESTATES:
        return 0;
        break;
    case COMPF_FLAG_CONFIGURE:  // configure
        //printf("New switch Block\n");
        libdyn_config_block(block, BLOCKTYPE_STATIC, Nout, Nin, (void *) 0, 0);
        libdyn_config_block_input(block, 0, 1, DATATYPE_FLOAT); // in, intype,
        libdyn_config_block_input(block, 1, 1, DATATYPE_FLOAT); // in, intype,
        libdyn_config_block_output(block, 0, 1, DATATYPE_FLOAT, 1);
        libdyn_config_block_output(block, 1, 1, DATATYPE_FLOAT, 1);

        return 0;
        break;
    case COMPF_FLAG_INIT:  // init
        return 0;
        break;
    case COMPF_FLAG_DESTUCTOR: // destroy instance
        return 0;
        break;
    }
}
struct dynlib_block_t *new_switch_block_(struct dynlib_simulation_t *sim)
{
    struct dynlib_block_t *blk = libdyn_new_block(sim, &compu_func_switch, 0, 0, 0,  0);

    return blk;
}


//
// A swich with two inputs and one output depending on a state. initial state=initial_state=ipar[0]
// state 0 means the first input is feed through
// state 1 means the first input is feed through
//
// Switching occurs to state 0 if event 1 occurs
// Switching occurs to state 1 if event 2 occurs
// event 0 is the normal regular event
//
struct _2to2_switch_event_t {
    int state;
};

int compu_func_2to1_event_switch(int flag, struct dynlib_block_t *block)
{
    //printf("comp_func switch: flag==%d\n", flag);
    int Nout = 1;
    int Nin = 2;

    double *inp1, *inp2;
    double *out;

    double *rpar = libdyn_get_rpar_ptr(block);
    int *ipar = libdyn_get_ipar_ptr(block);

    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS: {
        inp1 = (double *) libdyn_get_input_ptr(block,0);
        inp2 = (double *) libdyn_get_input_ptr(block,1);
        out = (double *) libdyn_get_output_ptr(block,0);
        struct _2to2_switch_event_t *work = (struct _2to2_switch_event_t *) block->work;


        if (work->state == 0) {
            out[0] = inp1[0];
        } else {
            out[0] = inp2[0];
        }

//     printf("[%f, %f, %f, %f]\n", *control_in, *inp, *out1, *out2);
    }
    return 0;
    break;
    case COMPF_FLAG_UPDATESTATES: {
        struct _2to2_switch_event_t *work = (struct _2to2_switch_event_t *) block->work;

        if (__libdyn_event_check(block, 1)) { // switch to inp 1
            work->state = 0;
            printf("2to1sw: in1\n");
        }
        if (__libdyn_event_check(block, 2)) { // switch to inp 2
            work->state = 1;
            printf("2to1sw: in2\n");
        }
    }
    return 0;
    break;
    case COMPF_FLAG_CONFIGURE: { // configure
        //printf("New switch Block\n");
        libdyn_config_block(block, BLOCKTYPE_DYNAMIC, Nout, Nin, (void *) 0, 0);
        libdyn_config_block_input(block, 0, 1, DATATYPE_FLOAT); // in, intype,
        libdyn_config_block_input(block, 1, 1, DATATYPE_FLOAT); // in, intype,
        libdyn_config_block_output(block, 0, 1, DATATYPE_FLOAT, 1);

        block->work = (void *) malloc(sizeof(struct _2to2_switch_event_t));
        struct _2to2_switch_event_t *work = (struct _2to2_switch_event_t *) block->work;
        work->state = ipar[0]; // initial state
    }
    return 0;
    break;
    case COMPF_FLAG_INIT:  // init
        return 0;
        break;
    case COMPF_FLAG_DESTUCTOR: // destroy instance
        free(block->work);

        return 0;
        break;
    }
}

struct dynlib_block_t *new_2to1_event_switch_block(struct dynlib_simulation_t *sim, int *initial_state)
{
    struct dynlib_block_t *blk = libdyn_new_block(sim, &compu_func_2to1_event_switch, initial_state, 0, 0,  0);
    return blk;
}






int compu_func_sat(int flag, struct dynlib_block_t *block)
{
    //printf("comp_func gain: flag==%d\n", flag);
    int Nout = 1;
    int Nin = 1;

    double *inp1;
    double *out;

    double *rpar = libdyn_get_rpar_ptr(block);
    double min = rpar[0];
    double max = rpar[1];

    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
        inp1 = (double *) libdyn_get_input_ptr(block,0);
        out = (double *) libdyn_get_output_ptr(block,0);

        *out = *inp1;

        if (*out < min)
            *out = min;
        if (*out > max)
            *out = max;

        // printf("sat: in=%f, out=%f\n", *inp1, *out);

        return 0;
        break;
    case COMPF_FLAG_UPDATESTATES:
        return 0;
        break;
    case COMPF_FLAG_CONFIGURE:  // configure
        libdyn_config_block(block, BLOCKTYPE_STATIC, Nout, Nin, (void *) 0, 0);
        libdyn_config_block_input(block, 0, 1, DATATYPE_FLOAT); // in, intype,
        libdyn_config_block_output(block, 0, 1, DATATYPE_FLOAT, 1);

        return 0;
        break;
    case COMPF_FLAG_INIT:  // init
        return 0;
        break;
    case COMPF_FLAG_DESTUCTOR: // destroy instance
        return 0;
        break;
    case COMPF_FLAG_PRINTINFO:
        printf("I'm a sat block. lower bound = %f, upper one =%f\n", min, max);
        return 0;
        break;

    }
}
struct dynlib_block_t *new_sat_block_(struct dynlib_simulation_t *sim, double *c)
{
    //
    // c (rpar) is only available for initialisation

    struct dynlib_block_t *blk = libdyn_new_block(sim, &compu_func_sat, 0, c, 0,  0);

    return blk;
}


int compu_func_const(int flag, struct dynlib_block_t *block)
{
//   printf("comp_func const: flag==%d\n", flag);
    int Nout = 1;
    int Nin = 0;

    double *out;

    double *rpar = libdyn_get_rpar_ptr(block);
    double c = rpar[0];

    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
        out = (double *) libdyn_get_output_ptr(block,0);

        *out = c;
        return 0;
        break;
    case COMPF_FLAG_UPDATESTATES:
        return 0;
        break;
    case COMPF_FLAG_CONFIGURE:  // configure

        // BLOCKTYPE_STATIC enables makes sure that the output calculation is called only once
        libdyn_config_block(block, BLOCKTYPE_STATIC, Nout, Nin, (void *) 0, 0);
        libdyn_config_block_output(block, 0, 1, DATATYPE_FLOAT, 0);

        return 0;
        break;
    case COMPF_FLAG_INIT:  // init
        return 0;
        break;
    case COMPF_FLAG_DESTUCTOR: // destroy instance
        return 0;
        break;
    }
}
struct dynlib_block_t *new_const_block_(struct dynlib_simulation_t *sim, double *c)
{
    //
    // c (rpar) is only available for initialisation

    struct dynlib_block_t *block = libdyn_new_block(sim, &compu_func_const, 0, c, 0,  0);

    return block;
}


int compu_func_mul(int flag, struct dynlib_block_t *block)
{
// printf("comp_func sum: flag==%d\n", flag);
    int Nout = 1;
    int Nin = 2;

    double *inp1, *inp2;
    double *out;

    int *ipar = libdyn_get_ipar_ptr(block);
    int d1 = ipar[0];
    int d2 = ipar[1];

    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
        inp1 = (double *) libdyn_get_input_ptr(block,0);
        inp2 = (double *) libdyn_get_input_ptr(block,1);
        out = (double *) libdyn_get_output_ptr(block,0);

        *out = 1;

        if (d1 == 0)
            *out = *out * *inp1;
        if (d1 == 1)
            *out = *out / *inp1;
        if (d2 == 0) {
            *out = *out * *inp2;
        }
        if (d2 == 1)
            *out = *out / *inp2;

        //  printf("mulout = %f; in=[%f,%f] %d,%d\n", *out, *inp1, *inp2, d1, d2);

        return 0;
        break;
    case COMPF_FLAG_UPDATESTATES:
        return 0;
        break;
    case COMPF_FLAG_CONFIGURE:  // configure
        libdyn_config_block(block, BLOCKTYPE_STATIC, Nout, Nin, (void *) 0, 0);
        libdyn_config_block_input(block, 0, 1, DATATYPE_FLOAT); // in, intype,
        libdyn_config_block_input(block, 1, 1, DATATYPE_FLOAT); // in, intype,
        libdyn_config_block_output(block, 0, 1, DATATYPE_FLOAT, 1);

        //  printf("New mul block %d, %d\n", d1, d2);
        return 0;
        break;
    case COMPF_FLAG_INIT:  // init
        return 0;
        break;
    case COMPF_FLAG_DESTUCTOR: // destroy instance
        return 0;
        break;
    }
}

//#define new_sum_block(sim, c_a, c_b) (libdyn_new_block((sim), &compu_func_TP1, 0, (z_oo_ptr), 0,  0));


struct dynlib_block_t *new_mul_block_(struct dynlib_simulation_t *sim, int *d)
// d - int d[2];
{
//  printf("New sum: %f, %f\n", c[0],c[1]);
    struct dynlib_block_t *sum = libdyn_new_block(sim, &compu_func_mul, d, 0, 0,  0);

    return sum;
}







struct fn_gen_t {
    int counter;
    int shape;
};

int compu_func_fn_gen(int flag, struct dynlib_block_t *block)
{
// printf("comp_func sum: flag==%d\n", flag);
    int Nout = 1;
    int Nin = 2;

    double *period, *amp;
    double *out;

    int *ipar = libdyn_get_ipar_ptr(block);
    int shape = ipar[0];

    struct fn_gen_t *work = (struct fn_gen_t *) block->work;

    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
        period = (double *) libdyn_get_input_ptr(block,0);
        amp = (double *) libdyn_get_input_ptr(block,1);
        out = (double *) libdyn_get_output_ptr(block,0);

        //printf("fngen: period = %f, amp = %f\n", *period, *amp);

        int p = ceil(*period);
        double mod = work->counter % p;
        *out = *amp * sin( mod / p * 2*M_PI );

        return 0;
        break;
    case COMPF_FLAG_UPDATESTATES:
        work->counter++;

        return 0;
        break;
    case COMPF_FLAG_CONFIGURE:  // configure
        libdyn_config_block(block, BLOCKTYPE_DYNAMIC, Nout, Nin, (void *) 0, 0);
        libdyn_config_block_input(block, 0, 1, DATATYPE_FLOAT); // in, intype,
        libdyn_config_block_input(block, 1, 1, DATATYPE_FLOAT); // in, intype,
        libdyn_config_block_output(block, 0, 1, DATATYPE_FLOAT, 1);

        return 0;
        break;
    case COMPF_FLAG_INIT:  // init
        block->work = (void *) malloc(sizeof(struct fn_gen_t));
        work = (struct fn_gen_t *) block->work;

        work->counter = 0;

        //printf("New fn_gen block shape = %d\n", shape);

        return 0;
        break;
    case COMPF_FLAG_DESTUCTOR: // destroy instance
        free(work);

        return 0;
        break;
    }
}

//#define new_sum_block(sim, c_a, c_b) (libdyn_new_block((sim), &compu_func_TP1, 0, (z_oo_ptr), 0,  0));


struct dynlib_block_t *new_fn_gen_block_(struct dynlib_simulation_t *sim, int *shape)
{
    //
    // c_a, b_b (rpar) is only available for initialisation

    //printf("New fngen: %d\n", shape);
    struct dynlib_block_t *fn_gen = libdyn_new_block(sim, &compu_func_fn_gen, shape, 0, 0,  0);

    return fn_gen;
}




// MOVE, untested
struct ser2par_t {
    int z;
    double *buf;
    int update_output;
};

int compu_func_ser2par(int flag, struct dynlib_block_t *block)
{
    //printf("comp_func zTF: flag==%d\n", flag);

    double *inp;
    double *out;

    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);

    int len = ipar[0];
    int Nout = len;
    int Nin = 1;

    struct ser2par_t *s2p = (struct ser2par_t *) libdyn_get_work_ptr(block);

    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:

        if (s2p->update_output == 1) {
            inp = (double *) libdyn_get_input_ptr(block,0);
            out = (double *) libdyn_get_output_ptr(block,0);

            s2p->buf[len-1] = inp[0]; // store last and latest value

            // copy buf

            int i;
            for (i = 0; i<len; ++i)
                out[i] = s2p->buf[i];

            s2p->update_output = 0;
        }

        return 0;
        break;
    case COMPF_FLAG_UPDATESTATES:

        inp = (double *) libdyn_get_input_ptr(block,0);
        out = (double *) libdyn_get_output_ptr(block,0);

        if (s2p->z+1 == len) {// fast voll (bis auf das letzte element) --> reset
            s2p->z = 0;
            s2p->update_output = 1;
        } else {
            // store input
            s2p->buf[ s2p->z ] = inp[0];

            s2p->z++;
        }


        return 0;
        break;
    case COMPF_FLAG_CONFIGURE:  // configure
    {
        libdyn_config_block(block, BLOCKTYPE_DYNAMIC, Nout, Nin, (void *) 0, 0);  // no dfeed
        libdyn_config_block_input(block, 0, 1, DATATYPE_FLOAT); // in, intype,
        libdyn_config_block_output(block, 0, 1, DATATYPE_FLOAT, 1);

        struct ser2par_t *s2p = malloc(sizeof(struct ser2par_t));
        s2p->buf = malloc(len * sizeof(double));
        s2p->z = 0;
        s2p->update_output = 1;
        libdyn_set_work_ptr(block, (void *) s2p);
    }
    return 0;
    break;
    case COMPF_FLAG_INIT:  // init
        return 0;
        break;
    case COMPF_FLAG_DESTUCTOR: // destroy instance
    {
        struct ser2par_t *s2p = (struct ser2par_t *) libdyn_get_work_ptr(block);
        free(s2p->buf);
        free(s2p);
    }

    return 0;
    break;
    }
}

struct dynlib_block_t *new_ser2par_block(struct dynlib_simulation_t *sim, int *len)
{
    //printf("New fngen: %d\n", shape);
    struct dynlib_block_t *s2p = libdyn_new_block(sim, &compu_func_ser2par, len, 0, 0,  0);

    return s2p;
}


//
// sampler block that uses libilc
//

#include "libilc.h"
int compu_func_play_block(int flag, struct dynlib_block_t *block)
{
//   printf("comp_func play: flag==%d\n", flag);
    int Nout = 1;
    int Nin = 0;

    double *out;

    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);
    int Namples = ipar[0];
    int initial_play = ipar[1];
    int hold_last_value = ipar[2];
    int mute_afterstop = ipar[3];
    double *r = rpar;

    struct siso_sampler_t *sampler = (struct siso_sampler_t *) block->work;

    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:

        out =  (double *) libdyn_get_output_ptr(block,0);
        *out = siso_sampler_play(sampler, 0);

        //printf("play: = %f\n", *out);

        return 0;
        break;
    case COMPF_FLAG_UPDATESTATES:
        out =  (double *) libdyn_get_output_ptr(block,0);

        siso_sampler_play(sampler, 1);


        if (__libdyn_event_check(block, 1)) { // Start
            siso_sampler_init_sampling(sampler);
        }
        if (__libdyn_event_check(block, 2)) { // stop
            siso_sampler_stop_sampling(sampler);
        }


        return 0;
        break;
    case COMPF_FLAG_CONFIGURE:  // configure
        libdyn_config_block(block, BLOCKTYPE_DYNAMIC, Nout, Nin, (void *) 0, 0);  // BLOCKTYPE_DYNAMIC enables that the output calculation is called more than once
        libdyn_config_block_output(block, 0, 1, DATATYPE_FLOAT, 0);


        // printf("New sampler block r = [%f,%f,%f] len = %d\n", r[0],r[1],r[2], Namples);
        return 0;
        break;
    case COMPF_FLAG_INIT:  // init
        block->work = (void *) siso_sampler_new(Namples, r);

        siso_sampler_special_cfg(block->work, hold_last_value, mute_afterstop);
        if (initial_play==1)
            siso_sampler_init_sampling(block->work);

//       siso_sampler_stop_sampling(sampler);

        return 0;
        break;
    case COMPF_FLAG_DESTUCTOR: // destroy instance
        siso_sampler_del(sampler);

        return 0;
        break;
    case COMPF_FLAG_RESETSTATES:
//       printf("00000000\n");

        siso_sampler_stop_sampling(sampler);
        if (initial_play==1)
            siso_sampler_init_sampling(sampler);

        out =  (double *) libdyn_get_output_ptr(block,0);
        *out = siso_sampler_play(sampler, 0);

        return 0;
        break;
    case COMPF_FLAG_PRINTINFO:
        printf("I'm a sampler block\n");
        return 0;
        break;

    }
}

struct dynlib_block_t *new_play_block(struct dynlib_simulation_t *sim, int *par, double *r)
{   // par: int[2] : [length(r), initial_play]

    struct dynlib_block_t *s = libdyn_new_block(sim, &compu_func_play_block, par, r, 0,  0);
    return s;
}



int compu_func_lookup(int flag, struct dynlib_block_t *block) // OBSOLETE
{
    //printf("comp_func gain: flag==%d\n", flag);
    int Nout = 1;
    int Nin = 1;

    double *inp1;
    double *out;

    double *rpar = libdyn_get_rpar_ptr(block);
    int *ipar = libdyn_get_ipar_ptr(block);
    int len = ipar[0];
    double lowerin = rpar[0];
    double upperin = rpar[1];
    double *table = &rpar[2];

    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        inp1 = (double *) libdyn_get_input_ptr(block,0);
        out = (double *) libdyn_get_output_ptr(block,0);

        double nin = (*inp1 - lowerin) / (upperin - lowerin); // [0..1]

        //   printf("nin=%f\n", nin);

        if (nin < 0)
            nin = 0;
        if (nin > 1)
            nin = 1;

        int outindex = floor(nin * len);
        double remainder = nin*len - outindex; // Range [0..1]

        if ( outindex >= 0 && outindex+1 < len) { // check wheter index is out of array
            *out = table[outindex] * ( (1-remainder) ) +  table[outindex+1] * remainder; // linear interpolation
        } else if (outindex < 0) {
            *out = table[0];
        } else if (outindex >= len) {
            *out = table[len-1];
        }
        //    printf("lookup table in=%f, lowerin=%f, upperin=%f, out=%f, index=%d\n", *inp1, lowerin, upperin, *out, outindex);
    }
    return 0;
    break;
    case COMPF_FLAG_UPDATESTATES:
        return 0;
        break;
    case COMPF_FLAG_CONFIGURE:  // configure
        libdyn_config_block(block, BLOCKTYPE_STATIC, Nout, Nin, (void *) 0, 0);
        libdyn_config_block_input(block, 0, 1, DATATYPE_FLOAT); // in, intype,
        libdyn_config_block_output(block, 0, 1, DATATYPE_FLOAT, 1);

        printf("len = %d\n", block->inlist[0].len);

        return 0;
        break;
    case COMPF_FLAG_INIT:  // init
        return 0;
        break;
    case COMPF_FLAG_DESTUCTOR: // destroy instance
        return 0;
        break;
    case COMPF_FLAG_PRINTINFO:
        printf("I'm a lookup block\n");
        return 0;
        break;

    }
}

struct dynlib_block_t *new_lookup_block(struct dynlib_simulation_t *sim, int *ipar, double *rpar)
{
    //
    // ipar = [ len ]
    // rpar = [ lowerin, upperin, [ k1, k2, ..., klen ] ]

    struct dynlib_block_t *lookup = libdyn_new_block(sim, &compu_func_lookup, ipar, rpar, 0,  0);

    return lookup;
}



int compu_func_filedump(int flag, struct dynlib_block_t *block)
{
    // FIXME: maxlen und autostart nicht implementiert

    //printf("comp_func gain: flag==%d\n", flag);
    int Nout = 0;
    int Nin = 1;

    double *in;

    double *rpar = libdyn_get_rpar_ptr(block);
    int *ipar = libdyn_get_ipar_ptr(block);
    int maxlen = ipar[0];
    int autostart = ipar[1];
    int vlen = ipar[2];
    int fnamelen = ipar[3];
    int *codedfname = &ipar[4];


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
    }
    return 0;
    break;
    case COMPF_FLAG_UPDATESTATES:
    {
        struct filewriter_t *filewriter = (struct filewriter_t *) block->work;

        in = (double *) libdyn_get_input_ptr(block,0);

//     printf("in=%f fwh=%x\n", in[0], filewriter);
        log_dfilewriter_log(filewriter, in);
    }
    return 0;
    break;
    case COMPF_FLAG_RESETSTATES:
    {
        struct filewriter_t *filewriter = (struct filewriter_t *) block->work;
        log_dfilewriter_reset(filewriter);
    }
    return 0;
    break;
    case COMPF_FLAG_CONFIGURE:  // configure
    {

        // one Port of length vlen
        libdyn_config_block(block, BLOCKTYPE_DYNAMIC, Nout, Nin, (void *) 0, 0);
        libdyn_config_block_input(block, 0, vlen, DATATYPE_FLOAT);
    }
    return 0;
    break;
    case COMPF_FLAG_INIT:  // init
    {
        char filename[250];

        if (fnamelen > sizeof(filename)) {
            printf("compu_func_filedump: ERROR: Filename too long\n");
            return -1;
        }

        // Decode filename
        int i;
        for (i = 0; i < fnamelen; ++i)
            filename[i] = codedfname[i];

        filename[i] = 0; // String termination

        //      printf("Decoded filename = %s\n", filename);

        int num_elements = 10000 / vlen; // Buffersize is alwas aroud 10000 * sizeof(double)

        struct filewriter_t *filewriter = log_dfilewriter_new(vlen, num_elements, filename);
        if (filewriter == 0) {
            printf("compu_func_filedump: ERROR: Connot create filewriter - maybe check filename\n");
            return -1;
        }

        libdyn_set_work_ptr(block, (void *) filewriter);

        // FIXME: Fehler abfangen

    }

    return 0;
    break;
    case COMPF_FLAG_DESTUCTOR: // destroy instance
    {
        struct filewriter_t *filewriter = (struct filewriter_t *) block->work;

        log_dfilewriter_del(filewriter);
    }
    return 0;
    break;
    case COMPF_FLAG_PRINTINFO:
        printf("I'm a filedumper block\n");
        return 0;
        break;

    }
}

struct dynlib_block_t *new_filedump_block(struct dynlib_simulation_t *sim, int *ipar, double *rpar)
{
    //ipar=[maxlen, autostart, vlen, length(fname),  fname(:)' ]

    struct dynlib_block_t *lookup = libdyn_new_block(sim, &compu_func_filedump, ipar, rpar, 0,  0);

    return lookup;
}


int compu_func_compare(int flag, struct dynlib_block_t *block)
{
    //printf("comp_func gain: flag==%d\n", flag);
    int Nout = 1;
    int Nin = 1;

    double *inp1;
    double *out;

    double *rpar = libdyn_get_rpar_ptr(block);
    double thr = rpar[0];
    int *ipar = libdyn_get_ipar_ptr(block);
    int mode = ipar[0];

    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
        inp1 = (double *) libdyn_get_input_ptr(block,0);
        out = (double *) libdyn_get_output_ptr(block,0);

        if (*inp1 <= thr)
            *out = (mode == 0) ? -1 : 0;
        if (*inp1 > thr)
            *out = 1;

        return 0;
        break;
    case COMPF_FLAG_UPDATESTATES:
        return 0;
        break;
    case COMPF_FLAG_CONFIGURE:  // configure
        libdyn_config_block(block, BLOCKTYPE_STATIC, Nout, Nin, (void *) 0, 0);
        libdyn_config_block_input(block, 0, 1, DATATYPE_FLOAT); // in, intype,
        libdyn_config_block_output(block, 0, 1, DATATYPE_FLOAT, 1);

        return 0;
        break;
    case COMPF_FLAG_INIT:  // init
        return 0;
        break;
    case COMPF_FLAG_DESTUCTOR: // destroy instance
        return 0;
        break;
    case COMPF_FLAG_PRINTINFO:
        printf("I'm a compare block. If input > %f: output = 1; else -1 or 0; depending on mode=%d\n", thr, mode);
        return 0;
        break;
    }
}

int compu_func_flipflop(int flag, struct dynlib_block_t *block)
{
// printf("comp_func flipflop: flag==%d\n", flag);
    int Nout = 1;
    int Nin = 3;

    int *ipar = libdyn_get_ipar_ptr(block);
    int initial_state = ipar[0];
    int *state = (void*) libdyn_get_work_ptr(block);

    double *set0;
    double *set1;
    double *reset;
    double *output;



    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
        set0= (double *) libdyn_get_input_ptr(block,0);
        set1 = (double *) libdyn_get_input_ptr(block,1);
        reset = (double *) libdyn_get_input_ptr(block,2);
        output = (double *) libdyn_get_output_ptr(block,0);

        *output = state[0];

        return 0;
        break;
    case COMPF_FLAG_UPDATESTATES:
        set0= (double *) libdyn_get_input_ptr(block,0);
        set1 = (double *) libdyn_get_input_ptr(block,1);
        reset = (double *) libdyn_get_input_ptr(block,2);
        output = (double *) libdyn_get_output_ptr(block,0);

        if (set0[0] > 0) {
            state[0] = 0;
// 	printf("state change to 0\n");
        } else if (set1[0] > 0) {
            state[0] = 1;
// 	printf("state change to 1\n");
        } else if (reset[0] > 0) {
            state[0] = initial_state;
// 	printf("state change to reset\n");
        }

        return 0;
        break;
    case COMPF_FLAG_CONFIGURE:  // configure
        //printf("New flipflop Block\n");
        libdyn_config_block(block, BLOCKTYPE_DYNAMIC, Nout, Nin, (void *) 0, 0);
        libdyn_config_block_input(block, 0, 1, DATATYPE_FLOAT); // in, intype,
        libdyn_config_block_input(block, 1, 1, DATATYPE_FLOAT); // in, intype,
        libdyn_config_block_input(block, 2, 1, DATATYPE_FLOAT); // in, intype,
        libdyn_config_block_output(block, 0, 1, DATATYPE_FLOAT, 1);
//       printf("configured\n");

        return 0;
        break;
    case COMPF_FLAG_INIT:  // init
    {
        int *state = malloc(sizeof(int));
        libdyn_set_work_ptr(block, (void *) state);
        *state = initial_state;
    }
    return 0;
    break;
    case COMPF_FLAG_RESETSTATES:
    {
        *state = initial_state;
    }
    return 0;
    break;
    case COMPF_FLAG_DESTUCTOR: // destroy instance
    {
        void *buffer = (void*) libdyn_get_work_ptr(block);
        free(buffer);
    }
    return 0;
    break;
    case COMPF_FLAG_PRINTINFO:
        printf("I'm a flipflop block. initial_state = %d\n", initial_state);
        return 0;
        break;
    }
}


int compu_func_printf(int flag, struct dynlib_block_t *block)
{

//   printf("comp_func printf: flag==%d\n", flag);
    int Nout = 0;
    int Nin = 1;

    double *in;

    double *rpar = libdyn_get_rpar_ptr(block);
    int *ipar = libdyn_get_ipar_ptr(block);

    int vlen = ipar[0];
    int fnamelen = ipar[1];
    int *codedfname = &ipar[2];


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        /*      in = (double *) libdyn_get_input_ptr(block,0);
              char *str = (char *) block->work;

              printf("%s [", str);
              int i;
              for (i = 0; i < vlen; ++i) {
        	printf("%f, ", in[i]);
              }
              printf("].\n"); */
    }
    return 0;
    break;
    case COMPF_FLAG_UPDATESTATES:
    {
        in = (double *) libdyn_get_input_ptr(block,0);
        char *str = (char *) block->work;

        printf("%s [", str);
        int i;
        for (i = 0; i < vlen; ++i) {
            printf("%f, ", in[i]);
        }
        printf("].\n");
    }
    return 0;
    break;
    case COMPF_FLAG_CONFIGURE:  // configure
    {

        // one Port of length vlen
        libdyn_config_block(block, BLOCKTYPE_DYNAMIC, Nout, Nin, (void *) 0, 0);
        libdyn_config_block_input(block, 0, vlen, DATATYPE_FLOAT);
    }
    return 0;
    break;
    case COMPF_FLAG_INIT:  // init
    {
        char *str = (char *) malloc(fnamelen+1);

        // Decode filename
        int i;
        for (i = 0; i < fnamelen; ++i)
            str[i] = codedfname[i];

        str[i] = 0; // String termination

        libdyn_set_work_ptr(block, (void*) str);

//      printf("Decoded filename = %s\n", filename);
    }
    return 0;
    break;
    case COMPF_FLAG_DESTUCTOR: // destroy instance
    {
        char *str = (char *) block->work;

        free(str);
    }
    return 0;
    break;
    case COMPF_FLAG_PRINTINFO:
        printf("I'm a printf block\n");
        return 0;
        break;

    }
}


// int compu_func_and(int flag, struct dynlib_block_t *block)
// {
//   //printf("comp_func gain: flag==%d\n", flag);
//   int Nout = 1;
//   int Nin = 1;
//
//   double *inp1;
//   double *out;
//
// //   double *rpar = libdyn_get_rpar_ptr(block);
// //   double c = rpar[0];
//
//   switch (flag) {
//     case COMPF_FLAG_CALCOUTPUTS:
//       inp1 = (double *) libdyn_get_input_ptr(block,0);
//       out = (double *) libdyn_get_output_ptr(block,0);
//
//       *out = *inp1;
//       return 0;
//       break;
//     case COMPF_FLAG_UPDATESTATES:
//       return 0;
//       break;
//     case COMPF_FLAG_CONFIGURE:  // configure
//       libdyn_config_block(block, BLOCKTYPE_STATIC, Nout, Nin, (void *) 0, 0);
//       libdyn_config_block_input(block, 0, 1, DATATYPE_FLOAT); // in, intype,
//       libdyn_config_block_output(block, 0, 1, DATATYPE_FLOAT, 1);
//
//       //printf("New sum block %f, %f\n", c_a, c_b);
//       return 0;
//       break;
//     case COMPF_FLAG_INIT:  // init
//       return 0;
//       break;
//     case COMPF_FLAG_DESTUCTOR: // destroy instance
//       return 0;
//       break;
//   }
// }












// Obsolete
int compu_func_interface(int flag, struct dynlib_block_t *block)
{
// printf("comp_func sum: flag==%d\n", flag);
    int Nout = 1;
    int Nin = 1;

    double *out, *in;

    int *ipar = libdyn_get_ipar_ptr(block);
    int len = ipar[0];

    struct fn_gen_t *work = (struct fn_gen_t *) block->work;

    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
        in = (double *) libdyn_get_input_ptr(block,0);
        out = (double *) libdyn_get_output_ptr(block,0);

        //printf("copy %f, len = %d\n", in[0], len);

        int i;
        for (i=0; i<len; ++i) // copy in to out FIXME: DO this by setting outptr to inptr
            out[i] = in[i];

        return 0;
        break;
    case COMPF_FLAG_UPDATESTATES:
        return 0;
        break;
    case COMPF_FLAG_CONFIGURE:  // configure
        libdyn_config_block(block, BLOCKTYPE_STATIC, Nout, Nin, (void *) 0, 0);
        libdyn_config_block_input(block, 0, len, DATATYPE_FLOAT); // in, intype,
        libdyn_config_block_output(block, 0, len, DATATYPE_FLOAT, 1);

        //printf("New fn_gen block shape = %d\n", shape);
        return 0;
        break;
    case COMPF_FLAG_INIT:  // configure
        return 0;
        break;
    case COMPF_FLAG_DESTUCTOR: // destroy instance
        return 0;
        break;
    }
}

//#define new_sum_block(sim, c_a, c_b) (libdyn_new_block((sim), &compu_func_TP1, 0, (z_oo_ptr), 0,  0));


struct dynlib_block_t *new_interface_block(struct dynlib_simulation_t *sim, int *len)
{
    //printf("New fngen: %d\n", shape);
    struct dynlib_block_t *interface = libdyn_new_block(sim, &compu_func_interface, len, 0, 0,  0);

    return interface;
}






// OBSOLETE UNTESTED AND NEVER USED
struct dynlib_super_block_t {
    struct dynlib_simulation_t *content_sim;
    struct dynlib_simulation_t *dest_sim;

    int Nin, Nout;
};

int compu_func_super(int flag, struct dynlib_block_t *block)
{
// printf("comp_func sum: flag==%d\n", flag);
    int Nout = 1;
    int Nin = 2;

    struct dynlib_super_block_t *super_info;
    super_info = (struct dynlib_super_block_t *) libdyn_get_work_ptr(block);

    double *inp1, *inp2;
    double *out;

    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
        inp1 = (double *) libdyn_get_input_ptr(block,0);
        inp2 = (double *) libdyn_get_input_ptr(block,1);
        out = (double *) libdyn_get_output_ptr(block,0);

        *out = 0;
        return 0;
        break;
    case COMPF_FLAG_UPDATESTATES:
        return 0;
        break;
    case COMPF_FLAG_CONFIGURE:  // configure
        printf("NEW SUPER\n");

        super_info = (struct dynlib_super_block_t *) libdyn_get_extraparam(block);

        //libdyn_set_work_ptr(block, (void *) super_info);
        libdyn_config_block(block, BLOCKTYPE_DYNAMIC, super_info->Nout, super_info->Nin, (void *) super_info, 1); // own outcache


//      libdyn_config_block_input(block, 0, 1, DATATYPE_FLOAT); // in, intype,
//      libdyn_config_block_input(block, 1, 1, DATATYPE_FLOAT); // in, intype,
//      libdyn_config_block_output(block, 0, 1, DATATYPE_FLOAT, 1);

        //printf("New sum block %f, %f\n", c_a, c_b);
        return 0;
        break;
    case COMPF_FLAG_INIT:  // configure
        return 0;
        break;
    case COMPF_FLAG_DESTUCTOR: // destroy instance
        return 0;
        break;
    }
}

//#define new_sum_block(sim, c_a, c_b) (libdyn_new_block((sim), &compu_func_TP1, 0, (z_oo_ptr), 0,  0));

struct dynlib_block_t *new_super_block_(struct dynlib_simulation_t *sim, struct dynlib_simulation_t *content)
{
    //
    //

    struct dynlib_super_block_t *super_info = (struct dynlib_super_block_t *) malloc(sizeof(struct dynlib_super_block_t));

    super_info->content_sim = content;
    super_info->dest_sim = sim;

    struct dynlib_block_t *super = libdyn_new_block(sim, &compu_func_super, 0, 0, 0,  (void *) super_info);

    return super;
}

