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

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <math.h>

#include "libdyn.h"
#include "libdyn_scicos_macros.h"




//
// A 2 to 1 switching Block
// inputs = [control_in, signal_in1, signal_in2]
// if control_in > 0 : out = signal_in1
// if control_in < 0 : out = signal_in2
//

int compu_func_switch2to1(int flag, struct dynlib_block_t *block)
{
    //printf("comp_func switch: flag==%d\n", flag);
    int Nout = 1;
    int Nin = 3;

    double *inp1, *inp2;
    double *control_in;
    double *out1;
    double *out2;

    double *rpar = libdyn_get_rpar_ptr(block);

    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
        control_in = (double *) libdyn_get_input_ptr(block,0);
        inp1 = (double *) libdyn_get_input_ptr(block,1);
        inp2 = (double *) libdyn_get_input_ptr(block,2);
        out1 = (double *) libdyn_get_output_ptr(block,0);

        if (*control_in > 0) {
            *out1 = *inp1;
        } else {
            *out1 = *inp2;
        }

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
        libdyn_config_block_input(block, 2, 1, DATATYPE_FLOAT); // in, intype,

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
        printf("I'm a switch 2 to 1 block\n");
        return 0;
        break;

    }
}

/*
  Demultiplexer

*/
int compu_func_demux(int flag, struct dynlib_block_t *block)
{
    // printf("comp_func demux: flag==%d\n", flag);
    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);

    int size = ipar[0];
    int datatype = ipar[1];
    int Nout = size;
    int Nin = 1;

    double *in;


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        in = (double *) libdyn_get_input_ptr(block,0);

        int i;

// 	printf("--");
        for (i = 0; i < size; ++i) {
            double *out = (double *) libdyn_get_output_ptr(block, i);
            *out = in[i];
// 	    printf("%f ", in[i]);
        }
//         printf("--\n");

    }
    return 0;
    break;
    case COMPF_FLAG_UPDATESTATES:
        return 0;
        break;
    case COMPF_FLAG_CONFIGURE:  // configure
    {
        if (size < 1) {
            printf("Demux size cannot be smaller than 1\n");
            printf("demux size = %d\n", size);

            return -1;
        }

        libdyn_config_block(block, BLOCKTYPE_STATIC, Nout, Nin, (void *) 0, 0);

        int i;
        for (i = 0; i < size; ++i) {
            libdyn_config_block_output(block, i, 1, DATATYPE_FLOAT,1 ); // in, intype,
        }

        libdyn_config_block_input(block, 0, size, DATATYPE_FLOAT);
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
        printf("I'm demux block\n");
        return 0;
        break;

    }
}


/*
  Demultiplexer

*/
int compu_func_mux(int flag, struct dynlib_block_t *block)
{
    //  printf("comp_func mux: flag==%d; irparid = %d\n", flag, block->irpar_config_id);
    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);

    int size = ipar[0];
    int datatype = ipar[1];
    int Nout = 1;
    int Nin = size;

    double *in;


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        double *out = (double *) libdyn_get_output_ptr(block,0);

        int i;

        for (i = 0; i < size; ++i) {
            double *in = (double *) libdyn_get_input_ptr(block, i);
            out[i] = *in;
        }

    }
    return 0;
    break;
    case COMPF_FLAG_UPDATESTATES:
        return 0;
        break;
    case COMPF_FLAG_CONFIGURE:  // configure
    {
        if (size < 1) {
            printf("Mux size cannot be smaller than 1\n");
            printf("mux size = %d\n", size);

            return -1;
        }

        libdyn_config_block(block, BLOCKTYPE_STATIC, Nout, Nin, (void *) 0, 0);

        int i;
        for (i = 0; i < size; ++i) {
            libdyn_config_block_input(block, i, 1, DATATYPE_FLOAT);
        }

        libdyn_config_block_output(block, 0, size, DATATYPE_FLOAT, 1);
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
        printf("I'm mux block\n");
        return 0;
        break;

    }
}



int ortd_compu_func_hysteresis(int flag, struct dynlib_block_t *block)
{
// printf("comp_func flipflop: flag==%d\n", flag);
    int Nout = 1;
    int Nin = 1;

    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);

    int initial_state = ipar[0];
    double switch_on_level = rpar[0];
    double switch_off_level = rpar[1];
    double onout = rpar[2];
    double offout = rpar[3];

    int *state = (void*) libdyn_get_work_ptr(block);

    double *in;
    double *output;



    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
        in= (double *) libdyn_get_input_ptr(block,0);
        output = (double *) libdyn_get_output_ptr(block,0);

        *output = (state[0] > 0) ? onout : offout ;

        return 0;
        break;
    case COMPF_FLAG_UPDATESTATES:
        in = (double *) libdyn_get_input_ptr(block,0);
        output = (double *) libdyn_get_output_ptr(block,0);

        if (state[0] < 0 && *in > switch_on_level) {
// 	printf("sw on\n");
            state[0] = 1;
        }

        if (state[0] > 0 && *in < switch_off_level) {
// 	printf("sw off\n");
            state[0] = -1;
        }

        return 0;
        break;
    case COMPF_FLAG_RESETSTATES:
        *state = initial_state;

        return 0;
        break;
    case COMPF_FLAG_CONFIGURE:  // configure
        //printf("New flipflop Block\n");
        libdyn_config_block(block, BLOCKTYPE_DYNAMIC, Nout, Nin, (void *) 0, 0);
        libdyn_config_block_input(block, 0, 1, DATATYPE_FLOAT); // in, intype,
        libdyn_config_block_output(block, 0, 1, DATATYPE_FLOAT, 1);

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
    case COMPF_FLAG_DESTUCTOR: // destroy instance
    {
        void *buffer = (void*) libdyn_get_work_ptr(block);
        free(buffer);
    }
    return 0;
    break;
    case COMPF_FLAG_PRINTINFO:
        printf("I'm a hysteresis block. initial_state = %d\n", initial_state);
        return 0;
        break;
    }
}


int ortd_compu_func_modcounter(int flag, struct dynlib_block_t *block)
{
    int Nout = 1; // # input ports
    int Nin = 1; // # output ports

    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);

    int initial_state = ipar[0];
    int mod = ipar[1];

    int *state = (void*) libdyn_get_work_ptr(block);

    double *in;
    double *output;

    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
        in= (double *) libdyn_get_input_ptr(block,0);
        output = (double *) libdyn_get_output_ptr(block,0);

        *output = state[0];

        return 0;
        break;
    case COMPF_FLAG_UPDATESTATES:
        in = (double *) libdyn_get_input_ptr(block,0);
        output = (double *) libdyn_get_output_ptr(block,0);

        if (*in > 0) {
            *state = *state + 1;
            if (*state >= mod)
                *state = 0;
        }

        return 0;
        break;
    case COMPF_FLAG_RESETSTATES: // reset states
        *state = initial_state;

        return 0;
        break;
    case COMPF_FLAG_CONFIGURE:  // configure
        libdyn_config_block(block, BLOCKTYPE_DYNAMIC, Nout, Nin, (void *) 0, 0);
        libdyn_config_block_input(block, 0, 1, DATATYPE_FLOAT); // in, intype,
        libdyn_config_block_output(block, 0, 1, DATATYPE_FLOAT, 1);

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
    case COMPF_FLAG_DESTUCTOR: // destroy instance
    {
        void *buffer = (void*) libdyn_get_work_ptr(block);
        free(buffer);
    }
    return 0;
    break;
    case COMPF_FLAG_PRINTINFO:
        printf("I'm a modcounter block. initial_state = %d\n", initial_state);
        return 0;
        break;
    }
}

int ortd_compu_func_jumper(int flag, struct dynlib_block_t *block)
{
    //  printf("comp_func mux: flag==%d; irparid = %d\n", flag, block->irpar_config_id);
    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);

    int steps = ipar[0];
    int Nout = 1;
    int Nin = 1;

    double *in;


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        double *out = (double *) libdyn_get_output_ptr(block,0);
        double *in = (double *) libdyn_get_input_ptr(block, 0);


        int i;

        for (i = 0; i < steps; ++i) {
            if (*in == i) {
                out[i] = 1;
            } else {
                out[i] = 0;
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
        if (steps < 1) {
            printf("steps cannot be smaller than 1\n");
            printf("steps = %d\n", steps);

            return -1;
        }

        libdyn_config_block(block, BLOCKTYPE_STATIC, Nout, Nin, (void *) 0, 0);

        libdyn_config_block_input(block, 0, 1, DATATYPE_FLOAT);
        libdyn_config_block_output(block, 0, steps, DATATYPE_FLOAT, 1);
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
        printf("I'm a jumper block\n");
        return 0;
        break;

    }
}


int ortd_compu_func_memory(int flag, struct dynlib_block_t *block)
{
// printf("comp_func flipflop: flag==%d\n", flag);
    int Nout = 1;
    int Nin = 2;

    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);

    double initial_state = rpar[0];

    double *state = (void*) libdyn_get_work_ptr(block);

    double *in, *rememberin;
    double *output;



    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
        in= (double *) libdyn_get_input_ptr(block,0);
        rememberin = (double *) libdyn_get_input_ptr(block,1);
        output = (double *) libdyn_get_output_ptr(block,0);

        if (*rememberin > 0)
            state[0] = *in;

        *output = state[0];

        return 0;
        break;
    case COMPF_FLAG_UPDATESTATES:
        in = (double *) libdyn_get_input_ptr(block,0);
        rememberin = (double *) libdyn_get_input_ptr(block,1);
        output = (double *) libdyn_get_output_ptr(block,0);

        if (*rememberin > 0)
            state[0] = *in;

        return 0;
        break;
    case COMPF_FLAG_RESETSTATES:
        *state = initial_state;

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
    {
        double *state__ = malloc(sizeof(double));
        libdyn_set_work_ptr(block, (void *) state__);
        *state__ = initial_state;
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
        printf("I'm a memory block. initial_state = %f\n", initial_state);
        return 0;
        break;
    }
}



int compu_func_abs(int flag, struct dynlib_block_t *block)
{
    //  printf("comp_func mux: flag==%d; irparid = %d\n", flag, block->irpar_config_id);
    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);

    int Nout = 1;
    int Nin = 1;


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        double *out = (double *) libdyn_get_output_ptr(block,0);
        double *in = (double *) libdyn_get_input_ptr(block, 0);

        out[0] = (*in > 0) ? *in : -(*in) ;

    }
    return 0;
    break;
    case COMPF_FLAG_UPDATESTATES:
        return 0;
        break;
    case COMPF_FLAG_CONFIGURE:  // configure
    {
        libdyn_config_block(block, BLOCKTYPE_STATIC, Nout, Nin, (void *) 0, 0);

        libdyn_config_block_input(block, 0, 1, DATATYPE_FLOAT);
        libdyn_config_block_output(block, 0, 1, DATATYPE_FLOAT, 1);
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
        printf("I'm an abs() block\n");
        return 0;
        break;

    }
}



/*
  extract_element block
*/
int compu_func_extract_element(int flag, struct dynlib_block_t *block)
{
//     printf("comp_func eele: flag==%d\n", flag);
    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);

    int vecsize = ipar[0];
    int datatype = ipar[1];
    int Nout = 1;
    int Nin = 2;

    double *invec, *inpointer;


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        invec = (double *) libdyn_get_input_ptr(block,0);
        inpointer = (double *) libdyn_get_input_ptr(block,1);
        double *out = (double *) libdyn_get_output_ptr(block, 0);

        int index = *inpointer - 1;

        // prevent out of bounds access
        if (index < 0)
            index = 0;
        if (index > vecsize-1)
            index = vecsize-1;

        *out = invec[ index ];

    }
    return 0;
    break;
    case COMPF_FLAG_UPDATESTATES:
        return 0;
        break;
    case COMPF_FLAG_CONFIGURE:  // configure
    {
        if (vecsize < 1) {
            printf("size cannot be smaller than 1\n");
            printf("size = %d\n", vecsize);

            return -1;
        }

        libdyn_config_block(block, BLOCKTYPE_STATIC, Nout, Nin, (void *) 0, 0);

        libdyn_config_block_input(block, 0, vecsize, DATATYPE_FLOAT);
        libdyn_config_block_input(block, 1, 1, DATATYPE_FLOAT);

        libdyn_config_block_output(block, 0, 1, DATATYPE_FLOAT,1 ); // in, intype,
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
        printf("I'm a oneelementextractor block\n");
        return 0;
        break;

    }
}



int compu_func_constvec(int flag, struct dynlib_block_t *block)
{
//   printf("comp_func const: flag==%d\n", flag);
    int Nout = 1;
    int Nin = 0;

    double *out;

    double *rpar = libdyn_get_rpar_ptr(block);
    int *ipar = libdyn_get_ipar_ptr(block);

    int veclen = ipar[0];
    int datatype = ipar[1];

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
    }
}

int ortd_compu_func_counter(int flag, struct dynlib_block_t *block)
{
    int Nin = 3; // # output ports
    int Nout = 1; // # input ports

    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);

    double initial_state = rpar[0];

    double *state = (void*) libdyn_get_work_ptr(block);

    double *incount, *inreset, *inresetto;
    double *output;

    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
        /*      incount = (double *) libdyn_get_input_ptr(block,0);
              inreset = (double *) libdyn_get_input_ptr(block,1);
              inresetto = (double *) libdyn_get_input_ptr(block,2);*/

        output = (double *) libdyn_get_output_ptr(block,0);

        *output = state[0];
//       printf("out = %f\n", *output);

        return 0;
        break;
    case COMPF_FLAG_UPDATESTATES:
        incount = (double *) libdyn_get_input_ptr(block,0);
        inreset = (double *) libdyn_get_input_ptr(block,1);
        inresetto = (double *) libdyn_get_input_ptr(block,2);


        state[0] += incount[0];
//       printf("upd %f %f\n", state[0], incount[0]);

        if (inreset[0] > 0)
            state[0] = inresetto[0];

        return 0;
        break;
    case COMPF_FLAG_RESETSTATES: // reset states
        *state = initial_state;

        return 0;
        break;
    case COMPF_FLAG_CONFIGURE:  // configure
        libdyn_config_block(block, BLOCKTYPE_DYNAMIC, Nout, Nin, (void *) 0, 0);

        libdyn_config_block_input(block, 0, 1, DATATYPE_FLOAT); // in, intype,
        libdyn_config_block_input(block, 1, 1, DATATYPE_FLOAT); // in, intype,
        libdyn_config_block_input(block, 2, 1, DATATYPE_FLOAT); // in, intype,
        libdyn_config_block_output(block, 0, 1, DATATYPE_FLOAT, 1);

        return 0;
        break;
    case COMPF_FLAG_INIT:  // init
    {
        double *state = malloc(sizeof(double));
        libdyn_set_work_ptr(block, (void *) state);
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
        printf("I'm a counter block. initial_state = %f\n", initial_state);
        return 0;
        break;
    }
}

int compu_func_lookuptable(int flag, struct dynlib_block_t *block)
{
    //printf("comp_func gain: flag==%d\n", flag);
    int Nout = 1;
    int Nin = 1;

    double *inp1;
    double *out;

    double *rpar = libdyn_get_rpar_ptr(block);
    int *ipar = libdyn_get_ipar_ptr(block);
    int len = ipar[0];
    int interpolation = ipar[1];
    double lowerin = rpar[0];
    double upperin = rpar[1];
    double *table = &rpar[2];

    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        inp1 = (double *) libdyn_get_input_ptr(block,0);
        out = (double *) libdyn_get_output_ptr(block,0);

	// normalise to 0..1
        double nin = (*inp1 - lowerin) / (upperin - lowerin); // [0..1]

        //   printf("nin=%f\n", nin);

// 	// bounds checking
//         if (nin < 0)
//             nin = 0;
//         if (nin > 1)
//             nin = 1;

        if (interpolation == 1) {

            int outindex = floor(nin * (len-1));
            double remainder = nin*(len-1) - outindex; // Range [0..1[

            if ( outindex >= 0 && outindex < len) { // check wheter index is out of array
                *out = table[outindex] * ( (1-remainder) ) +  table[outindex+1] * remainder; // linear interpolation
            } else if (outindex < 0) {
                *out = table[0];
            } else if (outindex >= len) {
                *out = table[len-1];
            }
            
//              *out = remainder;

        } else if (interpolation == 0) {
            int outindex = round(nin * (len-1));
	    
// 	    printf("lb=%f ub=%f u=%f, nin=%f, outindex=%d\n", lowerin, upperin, *inp1, nin, outindex);

	    if ( outindex >= 0 && outindex < len) { // check wheter index is out of array
                *out = table[outindex]; // no interpolation
            } else if (outindex < 0) {
                *out = table[0];
            } else if (outindex >= len) {
                *out = table[len-1];
            }
            
// 	    *out = outindex;
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

//       printf("len = %d\n", block->inlist[0].len);

        return 0;
        break;
    case COMPF_FLAG_INIT:  // init
        return 0;
        break;
    case COMPF_FLAG_DESTUCTOR: // destroy instance
        return 0;
        break;
    case COMPF_FLAG_PRINTINFO:
        printf("I'm a lookup table block\n");
        return 0;
        break;

    }
}









int ortd_compu_func_vectordiff(int flag, struct dynlib_block_t *block)
{
    // printf("comp_func demux: flag==%d\n", flag);
    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);

    int size = ipar[0];
    int datatype = ipar[1];
    int Nout = 1;
    int Nin = 1;

    double *in;


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        in = (double *) libdyn_get_input_ptr(block,0);


        double *out = (double *) libdyn_get_output_ptr(block, 0);

        int i;

        for (i=0; i < size-1; ++i) {
            out[i] = in[i+1] - in[i];
        }

    }
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

        libdyn_config_block_output(block, 0, size-1, DATATYPE_FLOAT,1 ); // in, intype,
        libdyn_config_block_input(block, 0, size, DATATYPE_FLOAT);
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
        printf("I'm vectordiff block\n");
        return 0;
        break;

    }
}




int ortd_compu_func_vectorfindthr(int flag, struct dynlib_block_t *block)
{
    // printf("comp_func demux: flag==%d\n", flag);
    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);

    int size = ipar[0];
    int greater = ipar[1];
    int Nin = 2;
    int Nout = 1;




    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        double *inv = (double *) libdyn_get_input_ptr(block,0);
        double *thr = (double *) libdyn_get_input_ptr(block,1);


        double *index = (double *) libdyn_get_output_ptr(block, 0); // index

        *index = -1;

        int i;

        if (greater > 0) {

            for (i=0; i < size; ++i) {
                if (inv[i] > *thr) {
                    *index = i+1;
                    break;
                }
            }

        } else {

            for (i=0; i < size; ++i) {
                if (inv[i] < *thr) {
                    *index = i+1;
                    break;
                }
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
        if (size < 1) {
            printf("size cannot be smaller than 1\n");
            printf("size = %d\n", size);

            return -1;
        }

        libdyn_config_block(block, BLOCKTYPE_STATIC, Nout, Nin, (void *) 0, 0);

        libdyn_config_block_output(block, 0, 1, DATATYPE_FLOAT,1 ); // in, intype,
        libdyn_config_block_input(block, 0, size, DATATYPE_FLOAT);
        libdyn_config_block_input(block, 1, 1, DATATYPE_FLOAT);
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
        printf("I'm vector thr find block\n");
        return 0;
        break;

    }
}

int ortd_compu_func_vectorabs(int flag, struct dynlib_block_t *block)
{
    // printf("comp_func demux: flag==%d\n", flag);
    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);

    int size = ipar[0];
    int Nout = 1;
    int Nin = 1;

    double *in;


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        in = (double *) libdyn_get_input_ptr(block,0);


        double *out = (double *) libdyn_get_output_ptr(block, 0);

        int i;

        for (i=0; i < size; ++i) {
            out[i] = ( in[i] < 0 ? -in[i] : in[i] );
        }

    }
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

        libdyn_config_block_output(block, 0, size, DATATYPE_FLOAT,1 ); // in, intype,
        libdyn_config_block_input(block, 0, size, DATATYPE_FLOAT);
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
        printf("I'm vectorabs block\n");
        return 0;
        break;

    }
}

int ortd_compu_func_vectorgain(int flag, struct dynlib_block_t *block)
{
    // printf("comp_func demux: flag==%d\n", flag);
    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);

    int size = ipar[0];
    double gain = rpar[0];
    int Nout = 1;
    int Nin = 1;

    double *in;


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        in = (double *) libdyn_get_input_ptr(block,0);


        double *out = (double *) libdyn_get_output_ptr(block, 0);

        int i;

        for (i=0; i < size; ++i) {
            out[i] = ( gain*in[i]  );
        }

    }
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

        libdyn_config_block_output(block, 0, size, DATATYPE_FLOAT,1 ); // in, intype,
        libdyn_config_block_input(block, 0, size, DATATYPE_FLOAT);
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
        printf("I'm vectorgain block\n");
        return 0;
        break;

    }
}


int ortd_compu_func_vectorextract(int flag, struct dynlib_block_t *block)
{
    // printf("comp_func demux: flag==%d\n", flag);
    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);

    int size = ipar[0];
    int window_len = ipar[1];
    int Nout = 1;
    int Nin = 2;

    double *in;


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        in = (double *) libdyn_get_input_ptr(block,0);
        double *from = (double *) libdyn_get_input_ptr(block, 1);


        double *out = (double *) libdyn_get_output_ptr(block, 0);

        int from_ = *from; // round to integer
        if (from_ < 1) from_ = 1;
        if (from_ > size-window_len+1) from_ = size-window_len+1;

        int i;

        for (i=0; i < window_len; ++i) {
            out[i] = ( in[ i + from_-1  ]  );
        }

    }
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

        libdyn_config_block_output(block, 0, window_len, DATATYPE_FLOAT,1 ); // in, intype,
        libdyn_config_block_input(block, 0, size, DATATYPE_FLOAT);
        libdyn_config_block_input(block, 1, 1, DATATYPE_FLOAT); // from input
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
        printf("I'm vectorextract block\n");
        return 0;
        break;

    }
}


int ortd_compu_func_vectorfindminmax(int flag, struct dynlib_block_t *block)
{
    // printf("comp_func demux: flag==%d\n", flag);
    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);

    int size = ipar[0];
    int findmax = ipar[1];
    int Nout = 1;
    int Nin = 1;

    double *in;


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        in = (double *) libdyn_get_input_ptr(block,0);
        double *out = (double *) libdyn_get_output_ptr(block, 0);

        int i;
        double potential_minmax;

        potential_minmax = in[0];

        if (findmax > 0) {

            for (i=1; i < size; ++i) {
                if (potential_minmax < in[i]) {
                    potential_minmax = in[i];
                }
            }

        } else {

            for (i=1; i < size; ++i) {
                if (potential_minmax > in[i]) {
                    potential_minmax = in[i];
                }
            }

        }

        *out = potential_minmax;

    }
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

        libdyn_config_block_output(block, 0, 1, DATATYPE_FLOAT,1 ); // in, intype,
        libdyn_config_block_input(block, 0, size, DATATYPE_FLOAT);
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
        printf("I'm vectorfindminmax block\n");
        return 0;
        break;

    }
}


int ortd_compu_func_vectoraddscalar(int flag, struct dynlib_block_t *block)
{
    // printf("comp_func demux: flag==%d\n", flag);
    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);

    int size = ipar[0];
    int Nout = 1;
    int Nin = 2;

    double *in;


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        in = (double *) libdyn_get_input_ptr(block,0);
        double *toadd = (double *) libdyn_get_input_ptr(block, 1);

        double *out = (double *) libdyn_get_output_ptr(block, 0);

        int i;
        for (i=0; i < size; ++i) {
            out[i] = ( in[ i ] + *toadd  );
        }

    }
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

        libdyn_config_block_output(block, 0, size, DATATYPE_FLOAT,1 ); // in, intype,
        libdyn_config_block_input(block, 0, size, DATATYPE_FLOAT);
        libdyn_config_block_input(block, 1, 1, DATATYPE_FLOAT); // add input
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
        printf("I'm vectoraddscalar block\n");
        return 0;
        break;

    }
}


int ortd_compu_func_vectorsum(int flag, struct dynlib_block_t *block)
{
    // printf("comp_func demux: flag==%d\n", flag);
    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);

    int size = ipar[0];
    int Nout = 1;
    int Nin = 1;

    double *in;


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        in = (double *) libdyn_get_input_ptr(block,0);
        double *out = (double *) libdyn_get_output_ptr(block, 0);

        int i;
        double sum=0;

        for (i=0; i < size; ++i) {
            sum += ( in[i] );
        }
        out[0] = sum;

    }
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

        libdyn_config_block_output(block, 0, 1, DATATYPE_FLOAT,1 ); // in, intype,
        libdyn_config_block_input(block, 0, size, DATATYPE_FLOAT);
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
        printf("I'm vectorsum block\n");
        return 0;
        break;

    }
}

int ortd_compu_func_vector_muladd(int flag, struct dynlib_block_t *block)
{
    // printf("comp_func demux: flag==%d\n", flag);
    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);

    int size = ipar[0];
    int Nout = 1;
    int Nin = 2;

    double *in1, *in2;


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        in1 = (double *) libdyn_get_input_ptr(block,0);
        in2 = (double *) libdyn_get_input_ptr(block,0);
        double *out = (double *) libdyn_get_output_ptr(block, 0);

        int i;
        double sum=0;

        for (i=0; i < size; ++i) {
            sum += ( in1[i] * in2[i] );
        }
        out[0] = sum;

    }
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

        libdyn_config_block_output(block, 0, 1, DATATYPE_FLOAT,1 ); // in, intype,
        libdyn_config_block_input(block, 0, size, DATATYPE_FLOAT);
        libdyn_config_block_input(block, 1, size, DATATYPE_FLOAT);
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
        printf("I'm vector_muladd block\n");
        return 0;
        break;

    }
}

//#include "block_lookup.h"

int libdyn_module_basic_ldblocks_siminit(struct dynlib_simulation_t *sim, int bid_ofs)
{
    // printf("libdyn module siminit function called\n");

    // Register my blocks to the given simulation

    int blockid_ofs = 60001;

    printf("Adding basic ld_blocks module\n");

    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs, LIBDYN_COMPFN_TYPE_LIBDYN, &compu_func_switch2to1);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 1, LIBDYN_COMPFN_TYPE_LIBDYN, &compu_func_demux);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 2, LIBDYN_COMPFN_TYPE_LIBDYN, &compu_func_mux);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 3, LIBDYN_COMPFN_TYPE_LIBDYN, &ortd_compu_func_hysteresis);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 4, LIBDYN_COMPFN_TYPE_LIBDYN, &ortd_compu_func_modcounter);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 5, LIBDYN_COMPFN_TYPE_LIBDYN, &ortd_compu_func_jumper);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 6, LIBDYN_COMPFN_TYPE_LIBDYN, &ortd_compu_func_memory);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 7, LIBDYN_COMPFN_TYPE_LIBDYN, &compu_func_abs);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 8, LIBDYN_COMPFN_TYPE_LIBDYN, &compu_func_extract_element);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 9, LIBDYN_COMPFN_TYPE_LIBDYN, &compu_func_constvec);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 10, LIBDYN_COMPFN_TYPE_LIBDYN, &ortd_compu_func_counter);
    // MISSING HERE: ld_shift register
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 12, LIBDYN_COMPFN_TYPE_LIBDYN, &compu_func_lookuptable);


//     shift_register


    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 50, LIBDYN_COMPFN_TYPE_LIBDYN, &ortd_compu_func_vectordiff);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 51, LIBDYN_COMPFN_TYPE_LIBDYN, &ortd_compu_func_vectorfindthr);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 52, LIBDYN_COMPFN_TYPE_LIBDYN, &ortd_compu_func_vectorabs);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 53, LIBDYN_COMPFN_TYPE_LIBDYN, &ortd_compu_func_vectorgain);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 54, LIBDYN_COMPFN_TYPE_LIBDYN, &ortd_compu_func_vectorextract);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 55, LIBDYN_COMPFN_TYPE_LIBDYN, &ortd_compu_func_vectorfindminmax);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 56, LIBDYN_COMPFN_TYPE_LIBDYN, &ortd_compu_func_vectoraddscalar);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 57, LIBDYN_COMPFN_TYPE_LIBDYN, &ortd_compu_func_vectorsum);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 58, LIBDYN_COMPFN_TYPE_LIBDYN, &ortd_compu_func_vector_muladd);


}


//} // extern "C"
