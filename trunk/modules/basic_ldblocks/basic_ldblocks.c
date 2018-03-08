/*
    Copyright (C) 2010, 2011, 2012,   Christian Klauer

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
#include <stdint.h>

// extern "C" {

#include "libdyn.h"
#include "libdyn_scicos_macros.h"
#include "irpar.h"





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

int compu_func_demuxInt32(int flag, struct dynlib_block_t *block)
{
    // printf("comp_func demux: flag==%d\n", flag);
    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);

    int size = ipar[0];
    int datatype = ipar[1];
    int Nout = size;
    int Nin = 1;

    int32_t *in;


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        in = (int32_t *) libdyn_get_input_ptr(block,0);

        int i;

// 	printf("--");
        for (i = 0; i < size; ++i) {
            int32_t *out = (int32_t *) libdyn_get_output_ptr(block, i);
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
            libdyn_config_block_output(block, i, 1, DATATYPE_INT32,1 ); // in, intype,
        }

        libdyn_config_block_input(block, 0, size, DATATYPE_INT32);
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
        printf("I'm demux int32_t block\n");
        return 0;
        break;

    }
}

/*
  Multiplexer

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

int compu_func_muxInt32(int flag, struct dynlib_block_t *block)
{
    //  printf("comp_func mux: flag==%d; irparid = %d\n", flag, block->irpar_config_id);
    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);

    int size = ipar[0];
    int datatype = ipar[1];
    int Nout = 1;
    int Nin = size;

    int32_t *in;


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        int32_t *out = (int32_t *) libdyn_get_output_ptr(block,0);

        int i;

        for (i = 0; i < size; ++i) {
            int32_t *in = (int32_t *) libdyn_get_input_ptr(block, i);
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
            libdyn_config_block_input(block, i, 1, DATATYPE_INT32);
        }

        libdyn_config_block_output(block, 0, size, DATATYPE_INT32, 1);
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
        output = (double *) libdyn_get_output_ptr(block,0);

        *state = initial_state;
        *output = (state[0] > 0) ? onout : offout ;  // Added this 14.5.2012

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
	//printf( "modcounter is at %f\n", *output);

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
	
	output = (double *) libdyn_get_output_ptr(block,0);

	*state = initial_state;
	*output = state[0];
	
	//fprintf(stderr, "modcounter resetstates to %f\n", *output);

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

    double *initial_state_vec = &rpar[0];

    double *state = (void*) libdyn_get_work_ptr(block);

    double *in, *rememberin;
    double *output;
    
    int memsize = ipar[0];
    size_t datasize = sizeof(double) * memsize;



    switch (flag) {
      case COMPF_FLAG_CALCOUTPUTS:
	  in= (double *) libdyn_get_input_ptr(block,0);
	  rememberin = (double *) libdyn_get_input_ptr(block,1);
	  output = (double *) libdyn_get_output_ptr(block,0);

	  // 	    memcpy(state, in, datasize);
  //             state[0] = *in;

  //         *output = state[0];
	  if (*rememberin > 0)
	      memcpy(output, in, datasize); // immidiately feed through the input data
	  else
	      memcpy(output, state, datasize); // TODO: Ist this required?

	  return 0;
	  break;
      case COMPF_FLAG_UPDATESTATES:
	  in = (double *) libdyn_get_input_ptr(block,0);
	  rememberin = (double *) libdyn_get_input_ptr(block,1);
	  output = (double *) libdyn_get_output_ptr(block,0);

	  if (*rememberin > 0) {
	      memcpy(state, in, datasize);   // TODO: Ist this required?
	  }
  //             state[0] = *in;
  

	  return 0;
	  break;
      case COMPF_FLAG_RESETSTATES:
	  memcpy(state, initial_state_vec, datasize);
	  //*state = initial_state;

	  return 0;
	  break;
      case COMPF_FLAG_CONFIGURE:  // configure
	  libdyn_config_block(block, BLOCKTYPE_DYNAMIC, Nout, Nin, (void *) 0, 0);
	  libdyn_config_block_input(block, 0, memsize, DATATYPE_FLOAT); // in, intype,
	  libdyn_config_block_input(block, 1, 1, DATATYPE_FLOAT); // in, intype,
	  libdyn_config_block_output(block, 0, memsize, DATATYPE_FLOAT, 1);

	  return 0;
	  break;
      case COMPF_FLAG_INIT:  // init
      {
	  double *state__ = malloc(datasize);
	  libdyn_set_work_ptr(block, (void *) state__);
	  memcpy(state__, initial_state_vec, datasize);
  //         *state__ = initial_state;
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
	  printf("I'm a memory block.\n");
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
        out = (double *) libdyn_get_output_ptr(block,0);
         memcpy((void*) out, (void*) vec, veclen*sizeof(double) );

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

        if (inreset[0] > 0.5)
            state[0] = inresetto[0];

        return 0;
        break;
    case COMPF_FLAG_RESETSTATES: // reset states
        output = (double *) libdyn_get_output_ptr(block,0);

        *state = initial_state;
        *output = state[0];	  // Added this 14.5.2012

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


int compu_func_vector_lookuptable(int flag, struct dynlib_block_t *block)
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
    int vecsize = ipar[2]; 

    double lowerin = rpar[0];
    double upperin = rpar[1];
    double *table = &rpar[2];

    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        inp1 = (double *) libdyn_get_input_ptr(block,0);
        out = (double *) libdyn_get_output_ptr(block,0);

        int i;

        for (i=0; i < vecsize; ++i) {



        // normalise to 0..1
            double nin = (inp1[i] - lowerin) / (upperin - lowerin); // [0..1]

            if (interpolation == 1) {

                int outindex = floor(nin * (len-1));
                double remainder = nin*(len-1) - outindex; // Range [0..1[

                if ( outindex >= 0 && outindex < len) { // check wheter index is out of array
                    out[i] = table[outindex] * ( (1-remainder) ) +  table[outindex+1] * remainder; // linear interpolation
                } else if (outindex < 0) {
                    out[i] = table[0];
                } else if (outindex >= len) {
                    out[i] = table[len-1];
                }
                
    //              *out = remainder;

            } else if (interpolation == 0) {
                int outindex = round(nin * (len-1));
            
    //      printf("lb=%f ub=%f u=%f, nin=%f, outindex=%d\n", lowerin, upperin, *inp1, nin, outindex);

            if ( outindex >= 0 && outindex < len) { // check wheter index is out of array
                    out[i] = table[outindex]; // no interpolation
                } else if (outindex < 0) {
                    out[i] = table[0];
                } else if (outindex >= len) {
                    out[i] = table[len-1];
                }
                
    //      *out = outindex;
            }

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
        libdyn_config_block_input(block, 0, vecsize, DATATYPE_FLOAT); // in, intype,
        libdyn_config_block_output(block, 0, vecsize, DATATYPE_FLOAT, 1);

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
        printf("I'm a vector lookup table block\n");
        return 0;
        break;

    }
}


int compu_func_not(int flag, struct dynlib_block_t *block)
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

        out[0] = (*in > 0.5) ? 0 : 1;

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
        printf("I'm a not block\n");
        return 0;
        break;

    }
}

int compu_func_ld_notInt32(int flag, struct dynlib_block_t *block)
{
    //  printf("comp_func mux: flag==%d; irparid = %d\n", flag, block->irpar_config_id);
    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);

    int Nout = 1;
    int Nin = 1;


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        int32_t *out = (int32_t *) libdyn_get_output_ptr(block,0);
        int32_t *in = (int32_t *) libdyn_get_input_ptr(block, 0);

        out[0] = (*in >= 1) ? 0 : 1;

    }
    return 0;
    break;
    case COMPF_FLAG_UPDATESTATES:
        return 0;
        break;
    case COMPF_FLAG_CONFIGURE:  // configure
    {
        libdyn_config_block(block, BLOCKTYPE_STATIC, Nout, Nin, (void *) 0, 0);

        libdyn_config_block_input(block, 0, 1, DATATYPE_INT32);
        libdyn_config_block_output(block, 0, 1, DATATYPE_INT32, 1);
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
        printf("I'm a not int32 block\n");
        return 0;
        break;

    }
}

int compu_func_ld_or(int flag, struct dynlib_block_t *block)
{
    //  printf("comp_func mux: flag==%d; irparid = %d\n", flag, block->irpar_config_id);
    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);

    int Nout = 1;
    int Nin = 2;


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        double *out = (double *) libdyn_get_output_ptr(block,0);
        double *in1 = (double *) libdyn_get_input_ptr(block, 0);
        double *in2 = (double *) libdyn_get_input_ptr(block, 1);

	*out = 0;
	if ( (*in1 > 0.5) || (*in2 > 0.5) )
          out[0] = 1;

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
        libdyn_config_block_input(block, 1, 1, DATATYPE_FLOAT);
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
        printf("I'm a ld_or block\n");
        return 0;
        break;

    }
}


int compu_func_ld_iszero(int flag, struct dynlib_block_t *block)
{
    //  printf("comp_func mux: flag==%d; irparid = %d\n", flag, block->irpar_config_id);
    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);
    double eps = rpar[0];

    int Nout = 1;
    int Nin = 1;


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        double *out = (double *) libdyn_get_output_ptr(block,0);
        double *in = (double *) libdyn_get_input_ptr(block, 0);

	*out = 0;
	if ( (*in > -eps) && (*in < eps) ) 
          out[0] = 1;
	
//  	printf("out=%f in=%f eps=%f\n", *out, *in, eps);

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
        printf("I'm a iszero block\n");
        return 0;
        break;

    }
}


int ortd_compu_func_limitedcounter(int flag, struct dynlib_block_t *block)
{
    int Nin = 3; // # output ports
    int Nout = 1; // # input ports

    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);

    double initial_state = rpar[0];
    double lower_b = rpar[1];
    double upper_b = rpar[2];

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
	
	// state constraints
	if (state[0] > upper_b) {
	  state[0] = upper_b;
	} else if (state[0] < lower_b) {
	  state[0] = lower_b;
	}

//        printf("upd %f %f\n", state[0], incount[0]);


        if (inreset[0] > 0.5)
            state[0] = inresetto[0];

        return 0;
        break;
    case COMPF_FLAG_RESETSTATES: // reset states
        output = (double *) libdyn_get_output_ptr(block,0);

        *state = initial_state;
        *output = state[0];

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

int ortd_compu_func_memorydel(int flag, struct dynlib_block_t *block)
{
// printf("comp_func flipflop: flag==%d\n", flag);
    int Nout = 1;
    int Nin = 2;

    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);

    double *initial_state_vec = &rpar[0];

    double *state = (void*) libdyn_get_work_ptr(block);

    double *in, *rememberin;
    double *output;
    
    int memsize = ipar[0];
    size_t datasize = sizeof(double) * memsize;



    switch (flag) {
      case COMPF_FLAG_CALCOUTPUTS:
	  in= (double *) libdyn_get_input_ptr(block,0);
	  rememberin = (double *) libdyn_get_input_ptr(block,1);
	  output = (double *) libdyn_get_output_ptr(block,0);

	  // 	    memcpy(state, in, datasize);
  //             state[0] = *in;

  //         *output = state[0];
          memcpy(output, state, datasize);

	  return 0;
	  break;
      case COMPF_FLAG_UPDATESTATES:
	  in = (double *) libdyn_get_input_ptr(block,0);
	  rememberin = (double *) libdyn_get_input_ptr(block,1);
	  output = (double *) libdyn_get_output_ptr(block,0);

	  if (*rememberin > 0)
	      memcpy(state, in, datasize);
  //             state[0] = *in;

	  return 0;
	  break;
      case COMPF_FLAG_RESETSTATES:
	  memcpy(state, initial_state_vec, datasize);
	  //*state = initial_state;
	  // FIXME Set output to new value

	  return 0;
	  break;
      case COMPF_FLAG_CONFIGURE:  // configure
	  libdyn_config_block(block, BLOCKTYPE_DYNAMIC, Nout, Nin, (void *) 0, 0);
	  libdyn_config_block_input(block, 0, memsize, DATATYPE_FLOAT); // in, intype,
	  libdyn_config_block_input(block, 1, 1, DATATYPE_FLOAT); // in, intype,
	  libdyn_config_block_output(block, 0, memsize, DATATYPE_FLOAT, 0);

	  return 0;
	  break;
      case COMPF_FLAG_INIT:  // init
      {
	  double *state__ = malloc(datasize);
	  libdyn_set_work_ptr(block, (void *) state__);
	  memcpy(state__, initial_state_vec, datasize);
  //         *state__ = initial_state;
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
	  printf("I'm a delaying memory block.\n");
	  return 0;
	  break;
    }
}

int ortd_compu_func_steps(int flag, struct dynlib_block_t *block)
{
// printf("comp_func flipflop: flag==%d\n", flag);
    int Nout = 1;
    int Nin = 0;

    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);

    int numsteps = ipar[0];
    int *times = &ipar[1];
    double *values = &rpar[0];

    
    int *state = (void*) libdyn_get_work_ptr(block);

    double *output;



    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
        output = (double *) libdyn_get_output_ptr(block,0);

        *output = values[*state];
// 	printf("out = %f\n", *output);

        return 0;
        break;
    case COMPF_FLAG_UPDATESTATES:
        output = (double *) libdyn_get_output_ptr(block,0);

// 	printf("steps #%d state %d nexttimestep=%d\n", block->sim->stepcounter, *state, times[*state]);
	
        if (times[*state] <= block->sim->stepcounter) { 
	  if (*state < (numsteps-1)) {
// 	    printf("sw on\n");
              (*state)++;
	  }
        }


        return 0;
        break;
    case COMPF_FLAG_RESETSTATES:
        *state = 0;
	
        output = (double *) libdyn_get_output_ptr(block,0);
	*output = *state;

        return 0;
        break;
    case COMPF_FLAG_CONFIGURE:  // configure
        //printf("New flipflop Block\n");
        libdyn_config_block(block, BLOCKTYPE_DYNAMIC, Nout, Nin, (void *) 0, 0);
        libdyn_config_block_output(block, 0, 1, DATATYPE_FLOAT, 1);

        return 0;
        break;
    case COMPF_FLAG_INIT:  // init
    {
        int *state__ = malloc(sizeof(int));
        libdyn_set_work_ptr(block, (void *) state__);
        *state__ = 0;
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
        printf("I'm a steps block.\n");
        return 0;
        break;
    }
}


int compu_func_ld_cond_overwrite(int flag, struct dynlib_block_t *block)
{
    //printf("comp_func switch: flag==%d\n", flag);
    int Nout = 1;
    int Nin = 2;

    double *inp1;
    double *cond_in;
    double *out1;

    double *rpar = libdyn_get_rpar_ptr(block);
    double setto = rpar[0];

    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
        cond_in = (double *) libdyn_get_input_ptr(block,0);
        inp1 = (double *) libdyn_get_input_ptr(block,1); // signal
        out1 = (double *) libdyn_get_output_ptr(block,0);

        if (*cond_in > 0.5) {
            *out1 = setto;
        } else {
            *out1 = *inp1;
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
        printf("I'm a ld_cond_overwrite block\n");
        return 0;
        break;

    }
}


int compu_func_ld_cond_overwrite2(int flag, struct dynlib_block_t *block)
{
    //printf("comp_func switch: flag==%d\n", flag);
    int Nout = 1;
    int Nin = 2;

    double *inp1;
    uint32_t *cond_in;
    double *out1;

    double *rpar = libdyn_get_rpar_ptr(block);
    double setto = rpar[0];

    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
        cond_in = (uint32_t *) libdyn_get_input_ptr(block,0);
        inp1 = (double *) libdyn_get_input_ptr(block,1); // signal
        out1 = (double *) libdyn_get_output_ptr(block,0);
        
       // printf("Condition: %d\n", *cond_in );

        if (*cond_in > 0) {
            *out1 = setto;
        } else {
            *out1 = *inp1;
        }

        return 0;
        break;
    case COMPF_FLAG_UPDATESTATES:
        return 0;
        break;
    case COMPF_FLAG_CONFIGURE:  // configure
        //printf("New switch Block\n");
        libdyn_config_block(block, BLOCKTYPE_STATIC, Nout, Nin, (void *) 0, 0);
        libdyn_config_block_input(block, 0, 1, DATATYPE_INT32); // in, intype,
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
    case COMPF_FLAG_PRINTINFO:
        printf("I'm a ld_cond_overwrite2 block\n");
        return 0;
        break;

    }
}





int compu_func_ld_cond_overwriteInt32(int flag, struct dynlib_block_t *block)
{
    //printf("comp_func switch: flag==%d\n", flag);
    int Nout = 1;
    int Nin = 2;

    uint32_t *inp1;
    uint32_t *cond_in;
    uint32_t *out1;

    uint32_t *ipar = libdyn_get_ipar_ptr(block);
    uint32_t setto = ipar[0];

    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
        cond_in = (uint32_t *) libdyn_get_input_ptr(block,0);
        inp1 = (uint32_t *) libdyn_get_input_ptr(block,1); // signal
        out1 = (uint32_t *) libdyn_get_output_ptr(block,0);
        
       // printf("Condition: %d\n", *cond_in );

        if (*cond_in > 0) {
            *out1 = setto;
        } else {
            *out1 = *inp1;
        }

        return 0;
        break;
    case COMPF_FLAG_UPDATESTATES:
        return 0;
        break;
    case COMPF_FLAG_CONFIGURE:  // configure
        //printf("New switch Block\n");
        libdyn_config_block(block, BLOCKTYPE_STATIC, Nout, Nin, (void *) 0, 0);
        libdyn_config_block_input(block, 0, 1, DATATYPE_INT32); // in, intype,
        libdyn_config_block_input(block, 1, 1, DATATYPE_INT32); // in, intype,
        
        libdyn_config_block_output(block, 0, 1, DATATYPE_INT32, 1);

        return 0;
        break;
    case COMPF_FLAG_INIT:  // init
        return 0;
        break;
    case COMPF_FLAG_DESTUCTOR: // destroy instance
        return 0;
        break;
    case COMPF_FLAG_PRINTINFO:
        printf("I'm a ld_cond_overwriteInt32 block\n");
        return 0;
        break;

    }
}








int ortd_compu_func_ld_ramp(int flag, struct dynlib_block_t *block) // FIXME: NOT FINISHED
{
    int Nout = 1;
    int Nin = 3;

    double *increase, *start, *reset;
    double *out;

    double *rpar = libdyn_get_rpar_ptr(block);
    

    double *state = (void*) libdyn_get_work_ptr(block);



    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
        increase = (double *) libdyn_get_input_ptr(block,0); 
        start = (double *) libdyn_get_input_ptr(block,1); 
        reset = (double *) libdyn_get_input_ptr(block,2); 
        out = (double *) libdyn_get_output_ptr(block,0);

        *out = state[0];

        return 0;
        break;
    case COMPF_FLAG_UPDATESTATES:
        increase = (double *) libdyn_get_input_ptr(block,0); 
        start = (double *) libdyn_get_input_ptr(block,1); 
        reset = (double *) libdyn_get_input_ptr(block,2); 
        out = (double *) libdyn_get_output_ptr(block,0);

        if  (state[0] <= 1) 
            state[0] += *increase;
	
	if (reset[0] > 0.5)
	  state[0] = 0;

        return 0;
        break;
    case COMPF_FLAG_RESETSTATES: // reset states
        out = (double *) libdyn_get_output_ptr(block,0);

        *state = 0;
        *out = state[0];

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
        *state = 0;
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
        printf("I'm a ramp block.\n");
        return 0;
        break;
    }
}

int compu_func_ld_and(int flag, struct dynlib_block_t *block)
{
    //  printf("comp_func mux: flag==%d; irparid = %d\n", flag, block->irpar_config_id);
    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);

    int Nout = 1;
    int Nin = 2;


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        double *out = (double *) libdyn_get_output_ptr(block,0);
        double *in1 = (double *) libdyn_get_input_ptr(block, 0);
        double *in2 = (double *) libdyn_get_input_ptr(block, 1);

	*out = 0;
	if ( (*in1 > 0.5) && (*in2 > 0.5) )
          out[0] = 1;

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
        libdyn_config_block_input(block, 1, 1, DATATYPE_FLOAT);
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
        printf("I'm a ld_and block\n");
        return 0;
        break;

    }
}

int compu_func_ld_andInt32(int flag, struct dynlib_block_t *block)
{
    //  printf("comp_func mux: flag==%d; irparid = %d\n", flag, block->irpar_config_id);
    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);

    int Nout = 1;
    int Nin = 2;


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        uint32_t *out = (uint32_t *) libdyn_get_output_ptr(block,0);
        uint32_t *in1 = (uint32_t *) libdyn_get_input_ptr(block, 0);
        uint32_t *in2 = (uint32_t *) libdyn_get_input_ptr(block, 1);

        
	*out = 0;
	if ( (*in1 > 0) && (*in2 > 0) )
          out[0] = 1;

           //     printf("---- %d and %d  = %d\n", *in1, *in2, *out );

    }
    return 0;
    break;
    case COMPF_FLAG_UPDATESTATES:
        return 0;
        break;
    case COMPF_FLAG_CONFIGURE:  // configure
    {
        libdyn_config_block(block, BLOCKTYPE_STATIC, Nout, Nin, (void *) 0, 0);

        libdyn_config_block_input(block, 0, 1, DATATYPE_INT32);
        libdyn_config_block_input(block, 1, 1, DATATYPE_INT32);
        libdyn_config_block_output(block, 0, 1, DATATYPE_INT32, 1);
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
        printf("I'm a ld_andInt32 block\n");
        return 0;
        break;

    }
}


int compu_func_ld_orInt32(int flag, struct dynlib_block_t *block)
{
    //  printf("comp_func mux: flag==%d; irparid = %d\n", flag, block->irpar_config_id);
    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);

    int Nout = 1;
    int Nin = 2;


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        uint32_t *out = (uint32_t *) libdyn_get_output_ptr(block,0);
        uint32_t *in1 = (uint32_t *) libdyn_get_input_ptr(block, 0);
        uint32_t *in2 = (uint32_t *) libdyn_get_input_ptr(block, 1);

	*out = 0;
	if ( (*in1 > 0) || (*in2 > 0) )
          out[0] = 1;

    }
    return 0;
    break;
    case COMPF_FLAG_UPDATESTATES:
        return 0;
        break;
    case COMPF_FLAG_CONFIGURE:  // configure
    {
        libdyn_config_block(block, BLOCKTYPE_STATIC, Nout, Nin, (void *) 0, 0);

        libdyn_config_block_input(block, 0, 1, DATATYPE_INT32);
        libdyn_config_block_input(block, 1, 1, DATATYPE_INT32);
        libdyn_config_block_output(block, 0, 1, DATATYPE_INT32, 1);
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
        printf("I'm a ld_orInt32 block\n");
        return 0;
        break;

    }
}


int ortd_compu_func_ld_initimpuls(int flag, struct dynlib_block_t *block)
{
// printf("comp_func flipflop: flag==%d\n", flag);
    int Nout = 1;
    int Nin = 0;

    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);


    
    int *state = (void*) libdyn_get_work_ptr(block);

    double *output;



    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
        output = (double *) libdyn_get_output_ptr(block,0);

	if (*state <= 0)
          *output = 1;
	else
	  *output = -0.001;

	// 	printf("out = %f\n", *output);

        return 0;
        break;
    case COMPF_FLAG_UPDATESTATES:
        output = (double *) libdyn_get_output_ptr(block,0);

// 	printf("steps #%d state %d nexttimestep=%d\n", block->sim->stepcounter, *state, times[*state]);
	
	  if (*state == 0) {
              (*state) += 1;
	  }


        return 0;
        break;
    case COMPF_FLAG_RESETSTATES:
        *state = 0;
	
        output = (double *) libdyn_get_output_ptr(block,0);
	*output = 1;

        return 0;
        break;
    case COMPF_FLAG_CONFIGURE:  // configure
        //printf("New flipflop Block\n");
        libdyn_config_block(block, BLOCKTYPE_DYNAMIC, Nout, Nin, (void *) 0, 0);
        libdyn_config_block_output(block, 0, 1, DATATYPE_FLOAT, 0);

        return 0;
        break;
    case COMPF_FLAG_INIT:  // init
    {
        int *state = malloc(sizeof(int));
        libdyn_set_work_ptr(block, (void *) state);
        *state = 0;
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
        printf("I'm a initimpuls block.\n");
        return 0;
        break;
    }
}

int compu_func_printfstderr(int flag, struct dynlib_block_t *block)
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

    }
      return 0;
      break;
    case COMPF_FLAG_UPDATESTATES:
    {
      in = (double *) libdyn_get_input_ptr(block,0);
      char *str = (char *) block->work;

      fprintf(stderr, "%s [", str);
      int i;
      for (i = 0; i < vlen; ++i) {
	fprintf(stderr, "%f, ", in[i]);
      }
      fprintf(stderr, "].\n"); 
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
      printf("I'm a printf(stderr,...) block\n");
      return 0;
      break;
      
  }
}

int compu_func_printfstderr2(int flag, struct dynlib_block_t *block)
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

    }
      return 0;
      break;
    case COMPF_FLAG_UPDATESTATES:
    {
      in = (double *) libdyn_get_input_ptr(block,0);
      char *str = (char *) block->work;

      fprintf(stderr, str, in[1]);
//      int i;
//      for (i = 0; i < vlen; ++i) {
//	fprintf(stderr, "%f, ", in[i]);
//      }
      fprintf(stderr, "\n"); 
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
      printf("I'm a printf(stderr,...) V2 block\n");
      return 0;
      break;
      
  }
}

int compu_func_ld_printfInt32(int flag, struct dynlib_block_t *block)
{
  
//   printf("comp_func printf: flag==%d\n", flag);
  int Nout = 0;
  int Nin = 1;

  int32_t *in;

  double *rpar = libdyn_get_rpar_ptr(block);
  int *ipar = libdyn_get_ipar_ptr(block);

  int vlen = ipar[0];
  int fnamelen = ipar[1];
  int *codedfname = &ipar[2];
  
  
  switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {  

    }
      return 0;
      break;
    case COMPF_FLAG_UPDATESTATES:
    {
      in = (int32_t *) libdyn_get_input_ptr(block,0);
      char *str = (char *) block->work;

      printf( "%s [", str);
      int i;
      for (i = 0; i < vlen; ++i) {
	printf( "%d, ", in[i]);
      }
      printf( "].\n"); 
    } 
      return 0;
      break;
    case COMPF_FLAG_CONFIGURE:  // configure
    {

      // one Port of length vlen
      libdyn_config_block(block, BLOCKTYPE_DYNAMIC, Nout, Nin, (void *) 0, 0); 
      libdyn_config_block_input(block, 0, vlen, DATATYPE_INT32); 
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
      printf("I'm a printf int32 block\n");
      return 0;
      break;
      
  }
}

int compu_func_printfbar(int flag, struct dynlib_block_t *block)
{
  
//   printf("comp_func printf: flag==%d\n", flag);
  int Nout = 0;
  int Nin = 1;

  double *in;

  double *rpar = libdyn_get_rpar_ptr(block);
  int *ipar = libdyn_get_ipar_ptr(block);

  int vlen = 1;
  int fnamelen = ipar[1];
  int *codedfname = &ipar[2];
  
  
  switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {  

    }
      return 0;
      break;
    case COMPF_FLAG_UPDATESTATES:
    {
      in = (double *) libdyn_get_input_ptr(block,0);
      char *str = (char *) block->work;
      
      int steps = round(*in);
      
      if (steps <= 0)
	steps = 0;
      
      if (steps >= 400)
	steps = 400;

      printf("%s |", str);
      int i;
      for (i = 0; i < steps; ++i) {
	printf("=");
      }
      printf(">\n"); 
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
      printf("I'm a printf(stderr,...) block\n");
      return 0;
      break;
      
  }
}


// AWs are missing
int compu_func_delay(int flag, struct dynlib_block_t *block)
{
  //printf("comp_func zTF: flag==%d\n", flag);
  int err;
  
  int Nout = 1;
  int Nin = 1;

  double *inp;
  double *out;	

  int *ipar = libdyn_get_ipar_ptr(block);
  int delay_length = ipar[0];
  
  int dfeed = 0;  

  void *buffer__ = (void*) libdyn_get_work_ptr(block);
  int *buf_position_read =  (int*) buffer__;
//    int *buf_position_write = &( (int*) buffer__)[1];
  double *rot_buf = (double*) (buffer__ + sizeof(int) );
  
  
  switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
      inp = (double *) libdyn_get_input_ptr(block,0);
      out = (double *) libdyn_get_output_ptr(block,0);
      
        *out = rot_buf[*buf_position_read];
          
	
//          rot_buf[0] = *inp;
	
//       printf("out %f, in %f, posr %d \n", *out, *inp, *buf_position_read);
      return 0;
      break;
    case COMPF_FLAG_UPDATESTATES:
      inp = (double *) libdyn_get_input_ptr(block,0);
      rot_buf[*buf_position_read] = *inp;
      
      if (*buf_position_read >= delay_length-1) {
	*buf_position_read = 0;
      } else {
	(*buf_position_read)++;
      }
      
      
      

//       if (*buf_position_write >= delay_length) {
// 	*buf_position_write = 0;
//       } else {
// 	(*buf_position_write)++;
//       }

//        printf("buf pos is now %d\n", *buf_position_read);
      
      return 0;
      break;
    case COMPF_FLAG_CONFIGURE:  // configure
      libdyn_config_block(block, BLOCKTYPE_DYNAMIC, Nout, Nin, (void *) 0, 0);  // no dfeed
      libdyn_config_block_input(block, 0, 1, DATATYPE_FLOAT); // in, intype, 
      libdyn_config_block_output(block, 0, 1, DATATYPE_FLOAT, dfeed);
  
      return 0;
      break;
    case COMPF_FLAG_INIT:  // configure
    {      
      if (delay_length < 1) {
	fprintf(stderr, "ld_delay: invalid delay length\n");
	return -1;
      }
      
      unsigned int Nbytes = sizeof(double)*(delay_length) + sizeof(unsigned int);
      void *buffer = malloc(Nbytes);
      memset((void*) buffer, 0,  Nbytes );
      
       int *bpr = &( (int*) buffer)[0];

       *bpr = 0;

      libdyn_set_work_ptr(block, (void *) buffer);
      
//        printf("ptr %p\n", buffer);
    }
      return 0;
      break;
    case COMPF_FLAG_RESETSTATES: // destroy instance
    {
      unsigned int Nbytes = sizeof(double)*(delay_length) + sizeof(unsigned int);
      void *buffer = buffer__;
      memset((void*) buffer, 0,  Nbytes );
      
       int *bpr = &( (int*) buffer)[0];

       *bpr = 0;
              
       out = (double *) libdyn_get_output_ptr(block,0);
       *out = 0;

//        fprintf(stderr, "ld_delay: reseting states\n");
       
      
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
      printf("I'm a delay block len=%d: \n", delay_length);
      return 0;
      break;
  }
}

int ortd_compu_func_steps2(int flag, struct dynlib_block_t *block)
{
// printf("comp_func flipflop: flag==%d\n", flag);
    int Nout = 1;
    int Nin = 0;

    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);

    int numsteps = ipar[0];
    int *times = &ipar[1];
    double *values = &rpar[0];

    
    int *state = (void*) libdyn_get_work_ptr(block);
    int *counter = (void*) libdyn_get_work_ptr(block) + sizeof(int);
    
    double *output;



    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
        output = (double *) libdyn_get_output_ptr(block,0);

        *output = values[*state];
// 	printf("out = %f\n", *output);

        return 0;
        break;
    case COMPF_FLAG_UPDATESTATES:
        output = (double *) libdyn_get_output_ptr(block,0);

// 	printf("steps #%d state %d nexttimestep=%d\n", block->sim->stepcounter, *state, times[*state]);
	
        if (times[*state] <= *counter) { 
	  if (*state < (numsteps-1)) {
// 	    printf("sw on\n");
              (*state)++;
	  }
        }
        
        *counter = *counter + 1;


        return 0;
        break;
    case COMPF_FLAG_RESETSTATES:
        *state = 0;
	*counter = 0;
	
        output = (double *) libdyn_get_output_ptr(block,0);
	*output = *state;
	
// 	fprintf(stderr, "ld_steps2: RESETSTATES, output set to %f\n", *output);

        return 0;
        break;
    case COMPF_FLAG_CONFIGURE:  // configure
        //printf("New flipflop Block\n");
        libdyn_config_block(block, BLOCKTYPE_DYNAMIC, Nout, Nin, (void *) 0, 0);
        libdyn_config_block_output(block, 0, 1, DATATYPE_FLOAT, 1);

        return 0;
        break;
    case COMPF_FLAG_INIT:  // init
    {
        void *mem = malloc(2 * sizeof(int));
        libdyn_set_work_ptr(block, (void *) mem);
	int *state__ = mem;
	int *counter__ = mem + sizeof(int); // FIXME: CHECK also above
	
        *state__ = 0;
	*counter__ = 0;
	
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
        printf("I'm a steps2 block.\n");
        return 0;
        break;
    }
}

int compu_func_ld_getsign(int flag, struct dynlib_block_t *block)
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

        out[0] = (*in >= 0) ? 1 : -1;
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
        printf("I'm a getsign block\n");
        return 0;
        break;

    }
}



/*
  insert_element block
*/
int compu_func_insert_element(int flag, struct dynlib_block_t *block)
{
//     printf("comp_func eele: flag==%d\n", flag);
    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);

    int vecsize = ipar[0];
    int datatype = ipar[1];
    int Nout = 1;
    int Nin = 2;

    double *in, *inpointer;


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        in = (double *) libdyn_get_input_ptr(block,0);
        inpointer = (double *) libdyn_get_input_ptr(block,1);
        double *out = (double *) libdyn_get_output_ptr(block, 0);
	
	//printf("insert_element dstptr %p\n", out);

        int index = *inpointer - 1;

        // prevent out of bounds access
        if (index < 0)
            index = 0;
        if (index > vecsize-1)
            index = vecsize-1;

//         printf(" schreibe %f --> index = %d \n", *in, index);
        out[index] = *in;
    }
        return 0;
        break;
    case COMPF_FLAG_UPDATESTATES:
//     {
//         in = (double *) libdyn_get_input_ptr(block,0);
//         inpointer = (double *) libdyn_get_input_ptr(block,1);
//         double *out = (double *) libdyn_get_output_ptr(block, 0);
// 	
// 	printf("insert_element dstptr %p\n", out);
// 
//         int index = *inpointer - 1;
// 
//         // prevent out of bounds access
//         if (index < 0)
//             index = 0;
//         if (index > vecsize-1)
//             index = vecsize-1;
// 
// //         printf(" schreibe %f --> index = %d \n", *in, index);
//         out[index] = *in;
//     }
    return 0;
    break;
    case COMPF_FLAG_RESETSTATES: // reset states
    {
       double *out = (double *) libdyn_get_output_ptr(block, 0);      
       memset((void*) out, 0, sizeof(double)*vecsize  );
    }
    return 0;
    break;
    
    case COMPF_FLAG_CONFIGURE:  // configure
    {
        if (vecsize < 1) {
            printf("size cannot be smaller than 1\n");
            printf("size = %d\n", vecsize);

            return -1;
        }

        libdyn_config_block(block, BLOCKTYPE_DYNAMIC, Nout, Nin, (void *) 0, 0);

        libdyn_config_block_input(block, 0, 1, DATATYPE_FLOAT);
        libdyn_config_block_input(block, 1, 1, DATATYPE_FLOAT);

        libdyn_config_block_output(block, 0, vecsize, DATATYPE_FLOAT,0 ); // in, intype,
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
        printf("I'm a oneelementinserter block\n");
        return 0;
        break;

    }
}


int compu_func_FlagProbe(int flag, struct dynlib_block_t *block)
{
  
  printf("FLAG PROBE: flag==%d\n", flag);
  
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
      in = (double *) libdyn_get_input_ptr(block,0);
      char *str = (char *) block->work;

      fprintf(stderr, "FlagProbe: CALCOUT %s [", str);
      int i;
      for (i = 0; i < vlen; ++i) {
	fprintf(stderr, "%f, ", in[i]);
      }
      fprintf(stderr, "].\n"); 

    }
      return 0;
      break;
    case COMPF_FLAG_UPDATESTATES:
    {
      in = (double *) libdyn_get_input_ptr(block,0);
      char *str = (char *) block->work;

      fprintf(stderr, "FlagProbe: UPSTATES %s [", str);
      int i;
      for (i = 0; i < vlen; ++i) {
	fprintf(stderr, "%f, ", in[i]);
      }
      fprintf(stderr, "].\n"); 
    } 
      return 0;
      break;
    case COMPF_FLAG_RESETSTATES:
    {
      in = (double *) libdyn_get_input_ptr(block,0);
      char *str = (char *) block->work;

      fprintf(stderr, "FlagProbe: RESETSTATES %s [", str);
      int i;
      for (i = 0; i < vlen; ++i) {
	fprintf(stderr, "%f, ", in[i]);
      }
      fprintf(stderr, "].\n"); 
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
      printf("I'm a FlagProbe block\n");
      return 0;
      break;
      
  }
}


int compu_func_ld_ceilInt32(int flag, struct dynlib_block_t *block)
{
    //  printf("comp_func mux: flag==%d; irparid = %d\n", flag, block->irpar_config_id);
    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);

    int Nout = 1;
    int Nin = 1;


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        int32_t *out = (int32_t *) libdyn_get_output_ptr(block,0);
        double *in = (double *) libdyn_get_input_ptr(block, 0);
	
        *out = ceil(*in);
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
        libdyn_config_block_output(block, 0, 1, DATATYPE_INT32, 1);
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
        printf("I'm a ceilInt32 block\n");
        return 0;
        break;

    }
}


int compu_func_ld_Int32ToFloat(int flag, struct dynlib_block_t *block)
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
        int32_t *in = (int32_t *) libdyn_get_input_ptr(block, 0);
	
        *out = *in;
    }
    return 0;
    break;
    case COMPF_FLAG_UPDATESTATES:
        return 0;
        break;
    case COMPF_FLAG_CONFIGURE:  // configure
    {
        libdyn_config_block(block, BLOCKTYPE_STATIC, Nout, Nin, (void *) 0, 0);

        libdyn_config_block_input(block, 0, 1, DATATYPE_INT32);
        libdyn_config_block_output(block, 0, 1, DATATYPE_FLOAT , 1);
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
        printf("I'm a Int32ToFloat block\n");
        return 0;
        break;

    }
}


int compu_func_vector_ld_Int32ToFloat(int flag, struct dynlib_block_t *block)
{
    //  printf("comp_func mux: flag==%d; irparid = %d\n", flag, block->irpar_config_id);
    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);

    int vecsize = ipar[0];

    int Nout = 1;
    int Nin = 1;


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        int32_t *out = (int32_t *) libdyn_get_output_ptr(block,0);
        double *in = (double *) libdyn_get_input_ptr(block, 0);

        int i;

        for (i=0; i<vecsize; ++i) {
            out[i] = in[i];  // conversion from int to double
        }
	
    }
    return 0;
    break;
    case COMPF_FLAG_UPDATESTATES:
        return 0;
        break;
    case COMPF_FLAG_CONFIGURE:  // configure
    {
        libdyn_config_block(block, BLOCKTYPE_STATIC, Nout, Nin, (void *) 0, 0);

        libdyn_config_block_input(block, 0, vecsize, DATATYPE_FLOAT);
        libdyn_config_block_output(block, 0, vecsize, DATATYPE_INT32, 1);
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
        printf("I'm a floorInt32 block\n");
        return 0;
        break;

    }
}


int compu_func_ld_floorInt32(int flag, struct dynlib_block_t *block)
{
    //  printf("comp_func mux: flag==%d; irparid = %d\n", flag, block->irpar_config_id);
    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);

    int Nout = 1;
    int Nin = 1;


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        int32_t *out = (int32_t *) libdyn_get_output_ptr(block,0);
        double *in = (double *) libdyn_get_input_ptr(block, 0);
    
        *out = floor(*in);
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
        libdyn_config_block_output(block, 0, 1, DATATYPE_INT32, 1);
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
        printf("I'm a floorInt32 block\n");
        return 0;
        break;

    }
}





int compu_func_vector_ld_floorInt32(int flag, struct dynlib_block_t *block)
{
    //  printf("comp_func mux: flag==%d; irparid = %d\n", flag, block->irpar_config_id);
    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);

    int vecsize = ipar[0];

    int Nout = 1;
    int Nin = 1;


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        int32_t *out = (int32_t *) libdyn_get_output_ptr(block,0);
        double *in = (double *) libdyn_get_input_ptr(block, 0);

        int i;

        for (i=0; i<vecsize; ++i) {
            out[i] = floor( in[i] );
        }
	
    }
    return 0;
    break;
    case COMPF_FLAG_UPDATESTATES:
        return 0;
        break;
    case COMPF_FLAG_CONFIGURE:  // configure
    {
        libdyn_config_block(block, BLOCKTYPE_STATIC, Nout, Nin, (void *) 0, 0);

        libdyn_config_block_input(block, 0, vecsize, DATATYPE_FLOAT);
        libdyn_config_block_output(block, 0, vecsize, DATATYPE_INT32, 1);
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
        printf("I'm a floorInt32 block\n");
        return 0;
        break;

    }
}


int compu_func_ld_roundInt32(int flag, struct dynlib_block_t *block)
{
    //  printf("comp_func mux: flag==%d; irparid = %d\n", flag, block->irpar_config_id);
    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);

    int Nout = 1;
    int Nin = 1;


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        int32_t *out = (int32_t *) libdyn_get_output_ptr(block,0);
        double *in = (double *) libdyn_get_input_ptr(block, 0);
	
        *out = round(*in);
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
        libdyn_config_block_output(block, 0, 1, DATATYPE_INT32, 1);
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
        printf("I'm a roundInt32 block\n");
        return 0;
        break;

    }
}

int compu_func_constvecInt32(int flag, struct dynlib_block_t *block)
{
//   printf("comp_func const: flag==%d\n", flag);
    int Nout = 1;
    int Nin = 0;

    int32_t *out;

    double *rpar = libdyn_get_rpar_ptr(block);
    int *ipar = libdyn_get_ipar_ptr(block);

    int veclen = ipar[0];
    int datatype = ipar[1];

    int *vec = &ipar[2];
    int i;

    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:

        return 0;
        break;
    case COMPF_FLAG_UPDATESTATES:
        return 0;
        break;
    case COMPF_FLAG_CONFIGURE:  // configure

        // BLOCKTYPE_STATIC enabled makes sure that the output calculation is called only once, since there is no input to rely on
        libdyn_config_block(block, BLOCKTYPE_STATIC, Nout, Nin, (void *) 0, 0);
        libdyn_config_block_output(block, 0, veclen, DATATYPE_INT32, 0);

        return 0;
        break;
    case COMPF_FLAG_INIT:  // init
        // initially copy the output
        out = (int32_t *) libdyn_get_output_ptr(block,0);
//          memcpy((void*) out, (void*) vec, veclen*sizeof(uint32_t) );
	 for (i = 0; i<veclen; ++i) {
	   out[i] = vec[i];
	 }

	 return 0;
        break;
    case COMPF_FLAG_DESTUCTOR: // destroy instance
        return 0;
        break;
    }
}

int compu_func_ld_sumInt32(int flag, struct dynlib_block_t *block)
{
    //  printf("comp_func mux: flag==%d; irparid = %d\n", flag, block->irpar_config_id);
    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);

    int Nout = 1;
    int Nin = 2;


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        int32_t *out = (int32_t *) libdyn_get_output_ptr(block,0);
        int32_t *in1 = (int32_t *) libdyn_get_input_ptr(block, 0);
        int32_t *in2 = (int32_t *) libdyn_get_input_ptr(block, 1);

	*out = *in1 + *in2;
    }
    return 0;
    break;
    case COMPF_FLAG_UPDATESTATES:
        return 0;
        break;
    case COMPF_FLAG_CONFIGURE:  // configure
    {
        libdyn_config_block(block, BLOCKTYPE_STATIC, Nout, Nin, (void *) 0, 0);

        libdyn_config_block_input(block, 0, 1, DATATYPE_INT32);
        libdyn_config_block_input(block, 1, 1, DATATYPE_INT32);
        libdyn_config_block_output(block, 0, 1, DATATYPE_INT32, 1);
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
        printf("I'm a sumInt32 block\n");
        return 0;
        break;

    }
}

int compu_func_ld_getsignInt32(int flag, struct dynlib_block_t *block)
{
    //  printf("comp_func mux: flag==%d; irparid = %d\n", flag, block->irpar_config_id);
    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);

    int Nout = 1;
    int Nin = 1;


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        int32_t *out = (int32_t *) libdyn_get_output_ptr(block,0);
        int32_t *in = (int32_t *) libdyn_get_input_ptr(block, 0);

        out[0] = (*in >= 0) ? 1 : -1;
    }
    return 0;
    break;
    case COMPF_FLAG_UPDATESTATES:
        return 0;
        break;
    case COMPF_FLAG_CONFIGURE:  // configure
    {
        libdyn_config_block(block, BLOCKTYPE_STATIC, Nout, Nin, (void *) 0, 0);

        libdyn_config_block_input(block, 0, 1, DATATYPE_INT32);
        libdyn_config_block_output(block, 0, 1, DATATYPE_INT32, 1);
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
        printf("I'm a getsignInt32 block\n");
        return 0;
        break;

    }
}

int ortd_compu_func_OneStepDelInt32(int flag, struct dynlib_block_t *block)
{
// printf("comp_func flipflop: flag==%d\n", flag);
    int Nout = 1;
    int Nin = 1;

    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);

    int32_t initial_state = ipar[0];

    int32_t *state = (void*) libdyn_get_work_ptr(block);

    int32_t *in;
    int32_t *output;
    

    switch (flag) {
      case COMPF_FLAG_CALCOUTPUTS:
	  in= (int32_t *) libdyn_get_input_ptr(block,0);
	  output = (int32_t *) libdyn_get_output_ptr(block,0);

	  *output = state[0];

	  return 0;
	  break;
      case COMPF_FLAG_UPDATESTATES:
	  in = (int32_t *) libdyn_get_input_ptr(block,0);
	  output = (int32_t *) libdyn_get_output_ptr(block,0);
	  
	  state[0] = *in;
	  
	  return 0;
	  break;
      case COMPF_FLAG_RESETSTATES:
	  state[0] = initial_state;	  

	  return 0;
	  break;
      case COMPF_FLAG_CONFIGURE:  // configure
	  libdyn_config_block(block, BLOCKTYPE_DYNAMIC, Nout, Nin, (void *) 0, 0);
	  libdyn_config_block_input(block, 0, 1, DATATYPE_INT32); // in, intype,
	  libdyn_config_block_input(block, 1, 1, DATATYPE_INT32); // in, intype,
	  libdyn_config_block_output(block, 0, 1, DATATYPE_INT32, 1);

	  return 0;
	  break;
      case COMPF_FLAG_INIT:  // init
      {
	  int32_t *state__ = malloc(sizeof(int32_t) );
	  libdyn_set_work_ptr(block, (void *) state__);

	  state__[0] = initial_state;
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
	  printf("I'm a memory block.\n");
	  return 0;
	  break;
    }
}

int compu_func_ld_CompareEqInt32(int flag, struct dynlib_block_t *block)
{
    //  printf("comp_func mux: flag==%d; irparid = %d\n", flag, block->irpar_config_id);
    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);

    int Nout = 1;
    int Nin = 1;


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        int32_t *out = (int32_t *) libdyn_get_output_ptr(block,0);
        int32_t *in = (int32_t *) libdyn_get_input_ptr(block, 0);

        out[0] = ( *in == ipar[0] ) ? 1 : 0;
    }
    return 0;
    break;
    case COMPF_FLAG_UPDATESTATES:
        return 0;
        break;
    case COMPF_FLAG_CONFIGURE:  // configure
    {
        libdyn_config_block(block, BLOCKTYPE_STATIC, Nout, Nin, (void *) 0, 0);

        libdyn_config_block_input(block, 0, 1, DATATYPE_INT32);
        libdyn_config_block_output(block, 0, 1, DATATYPE_INT32, 1);
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
        printf("I'm ld_CompareEqInt32 block\n");
        return 0;
        break;

    }
}


int compu_func_ld_CompareInt32(int flag, struct dynlib_block_t *block)
{
    //  printf("comp_func mux: flag==%d; irparid = %d\n", flag, block->irpar_config_id);
    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);

    int Nout = 1;
    int Nin = 1;
    
    int32_t thr = ipar[0];


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        int32_t *out = (int32_t *) libdyn_get_output_ptr(block,0);
        int32_t *in = (int32_t *) libdyn_get_input_ptr(block, 0);

        out[0] = ( *in > thr ) ? 1 : 0;
    }
    return 0;
    break;
    case COMPF_FLAG_UPDATESTATES:
        return 0;
        break;
    case COMPF_FLAG_CONFIGURE:  // configure
    {
        libdyn_config_block(block, BLOCKTYPE_STATIC, Nout, Nin, (void *) 0, 0);

        libdyn_config_block_input(block, 0, 1, DATATYPE_INT32);
        libdyn_config_block_output(block, 0, 1, DATATYPE_INT32, 1);
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
        printf("I'm a ld_CompareInt32 block\n");
        return 0;
        break;

    }
}


int compu_func_ld_integratorInt32(int flag, struct dynlib_block_t *block)
{
    //  printf("comp_func mux: flag==%d; irparid = %d\n", flag, block->irpar_config_id);
    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);

    int Nout = 1;
    int Nin = 1;
    
    //int32_t thr = ipar[0];


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        int32_t *out = (int32_t *) libdyn_get_output_ptr(block,0);
        int32_t *in = (int32_t *) libdyn_get_input_ptr(block, 0);

        out[0] = out[0] + in[0];
    }
    return 0;
    break;
    case COMPF_FLAG_UPDATESTATES:
        return 0;
        break;
    case COMPF_FLAG_CONFIGURE:  // configure
    {
        libdyn_config_block(block, BLOCKTYPE_DYNAMIC, Nout, Nin, (void *) 0, 0);

        libdyn_config_block_input(block, 0, 1, DATATYPE_INT32);
        libdyn_config_block_output(block, 0, 1, DATATYPE_INT32, 1);
    }
    return 0;
    break;
    case COMPF_FLAG_RESETSTATES:  // 
    {
        int32_t *out = (int32_t *) libdyn_get_output_ptr(block,0);
	
	out[0] = 0;
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
        printf("I'm a ld_CompareInt32 block\n");
        return 0;
        break;

    }
}

































/*
  Vector functions
*/


int ortd_compu_func_vectordelay(int flag, struct dynlib_block_t *block)
{
  int err;
  
  int Nout = 1;
  int Nin = 1;

  int *ipar = libdyn_get_ipar_ptr(block);
  int veclen = ipar[0];
  
  int dfeed = 0;  
  int i = 0;
  
  double *out;
  double *inp;

  void *buffer__ = (void*) libdyn_get_work_ptr(block);
//   int *buf_position_read =  (int*) buffer__;

  double *stat_buf = (double*) (buffer__ + sizeof(int) );
//   int stat_buf_begin;
  
  
  switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
//       freopen( "/home/max/outfile.txt", "a", stdout );
      out = (double *) libdyn_get_output_ptr(block,0);
      
//       printf("stat_buf_ptr = %p\n", stat_buf);
      
      for(i = 0; i < veclen; i++){
// 	printf("i = %d\n", i);
// 	printf("stat_buf = %f\n", stat_buf[i]);
	out[i] = stat_buf[i];
// 	printf("out = %f\n", out[i]);
// 	stat_buf++;
// 	out++;
      }
//       freopen( "CON", "a", stdout );

      return 0;
      break;
    }
    case COMPF_FLAG_UPDATESTATES:
    {
//       freopen( "/home/max/inpfile.txt", "a", stdout );
      inp = (double *) libdyn_get_input_ptr(block,0);
      
//       stat_buf = stat_buf_begin;
//       printf("stat_buf_ptr = %p\n", stat_buf);
      
      for(i = 0; i < veclen; i++){
// 	printf("i = %d\n", i);
// 	printf("inp = %f\n", inp[i]);
// 	*stat_buf = *inp;
	stat_buf[i] = inp[i];
// 	printf("stat_buf = %f\n", stat_buf[i]);
//  	stat_buf++;
//  	inp++;
      }
//       freopen( "CON", "a", stdout );
      
      return 0;
      break;
    }
    case COMPF_FLAG_CONFIGURE:  // configure
    {
      libdyn_config_block(block, BLOCKTYPE_DYNAMIC, Nout, Nin, (void *) 0, 0);  // no dfeed
      libdyn_config_block_input(block, 0, veclen, DATATYPE_FLOAT); // in, intype, 
      libdyn_config_block_output(block, 0, veclen, DATATYPE_FLOAT, dfeed);
  
      return 0;
      break;
    }
    case COMPF_FLAG_INIT:  // configure
    {      
      if (veclen < 1) {
	fprintf(stderr, "ld_vector_delay: invalid vector length\n");
	return -1;
      }
      
      unsigned int Nbytes = sizeof(double)*(veclen) + sizeof(unsigned int);
      void *buffer = malloc(Nbytes);
      memset((void*) buffer, 0,  Nbytes );
      
       int *bpr = &( (int*) buffer)[0];

       *bpr = 0;

      libdyn_set_work_ptr(block, (void *) buffer);
    }
      return 0;
      break;
    case COMPF_FLAG_RESETSTATES: // destroy instance
    {
      unsigned int Nbytes = sizeof(double)*(veclen) + sizeof(unsigned int);
      void *buffer = buffer__;
      memset((void*) buffer, 0,  Nbytes );
      
       int *bpr = &( (int*) buffer)[0];

       *bpr = 0;
              
       out = (double *) libdyn_get_output_ptr(block,0);
       *out = 0;

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
      printf("I'm a vector-delay block.\n");
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
    int Nout = 2;
    int Nin = 1;

    double *in;


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        in = (double *) libdyn_get_input_ptr(block,0);
        double *minmax_index = (double *) libdyn_get_output_ptr(block, 0);
	double *minmax_value = (double *) libdyn_get_output_ptr(block, 1);

        int i;
        double potential_minmax;
	double index = 1.0;

        

        if (findmax > 0) {
            
            
            i=0;
            do {
                if (!isnan(in[i])) {
                    
                    potential_minmax = in[i];
                    index = i + 1;
                    
                    ++i;
                    
                    break;
                }
            } while(1);
            

            for ( ; i < size; ++i) {
                if (!isnan(in[i])) {
                    if (potential_minmax < in[i]) {
                        potential_minmax = in[i];
                        index = i + 1; // +1 to match the c-way of counting indices
                    }
                }
            }

        } else {
            
            i=0;
            do {
                if (!isnan(in[i])) {
                    
                    potential_minmax = in[i];
                    index = i + 1;
                    
                    ++i;
                    
                    break;
                }
            } while(1);
            
            for ( ; i < size; ++i) {
                if (!isnan(in[i])) {
                    if (potential_minmax > in[i]) {
                        potential_minmax = in[i];
                        index = i + 1; // +1 to match the c-way of counting indices
                    }
                }
            }

        }

        minmax_index[0] = index;
	minmax_value[0] = potential_minmax;

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
        libdyn_config_block_output(block, 1, 1, DATATYPE_FLOAT,1 );
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


int ortd_compu_func_ld_vectorFindSpike(int flag, struct dynlib_block_t *block)
{
        printf("ortd_compu_func_ld_vectorFindSpike: flag==%d\n", flag);

    // printf("comp_func demux: flag==%d\n", flag);
    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);

    int size = ipar[0];
    int NskipLeft = ipar[1];
    int NskipRight = ipar[2];
    
    double significanceFactor = rpar[0]; // typically 6
    
    int Nout = 6;
    int Nin = 1;

    double *in;


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        in = (double *) libdyn_get_input_ptr(block,0);
	
        int32_t *index = (int32_t *) libdyn_get_output_ptr(block, 0);
        int32_t *FoundSpike = (int32_t *) libdyn_get_output_ptr(block, 1);
	
	double *Mean = (double *) libdyn_get_output_ptr(block, 2);
	double *sigma = (double *) libdyn_get_output_ptr(block, 3);
	double *Distance = (double *) libdyn_get_output_ptr(block, 4);
	double *Val = (double *) libdyn_get_output_ptr(block, 5);

        int i;
        double potential_max = 0;
	double potential_min = 0;
	int32_t indexmin = 1;
	int32_t indexmax = 1;
	
	int32_t potentialSpikeIndex;
	double SpikeVal;
	double Dis;

	#define abs(a) ( (a) > 0 ? (a) : (-a) )
	
            // Find the maximum which potentially is a spike
            i=0;
            do {
                if (!isnan(in[i])) {
                    
                    potential_max = in[i];
		    potential_min = in[i];
                    indexmin = i + 1;
		    indexmax = i + 1;
                    
                    ++i;
                    
                    break;
                }
            } while(1);
            

            for ( ; i < size; ++i) {
                if (!isnan(in[i])) {
		  
                    if (potential_max < in[i]) {
                        potential_max = in[i];
                        indexmax = i + 1; // +1 to match the c-way of counting indices
                    }
                  
                    if (potential_min > in[i]) {
                        potential_min = in[i];
                        indexmin = i + 1; // +1 to match the c-way of counting indices
                    }
                    
                    
                }
            }
            
//             printf("min=%f, max=%f\n", potential_min, potential_max);
// 	    printf("index min = %d, index max = %d\n", indexmin, indexmax);
            
            
            if ( abs(potential_max) > abs(potential_min) ) {
	      potentialSpikeIndex = indexmax;
	      SpikeVal = potential_max;
	    } else {
	      potentialSpikeIndex = indexmin;
	      SpikeVal = potential_min;
	    }
            
            // DEBUG
//             printf("index = %d, Val = %f\n", potentialSpikeIndex, SpikeVal);
            
            
            
            
            // calc mean and variance of data in the vector without nan values and without a range of size 2*Nskip+1 around the center of the potential spike
	    double Cummlative = 0;
	    int Nsamples = 0;
	    
            for (i = 0; i < size; ++i) {
// 	       printf(" I%d - v%f ", i, in[i] );
	       
	      if ( isnan( in[i] ) ) {
// 		printf("(no) ");
		continue;
	      }
	      
	      if ( (i > potentialSpikeIndex-NskipLeft) && ( i < potentialSpikeIndex+NskipRight ) ) {
// 		printf("(no) ");
		continue;
	      }

	     
	      
	      Cummlative += in[i];
	      ++Nsamples;
	    }
// 	    printf("\n");
	    
	    double EstMean = Cummlative / Nsamples;
	    
// 	    printf("EstMean = %f\n", EstMean);
            
	    // variance
	    Cummlative = 0;
	    //Nsamples = 0;
	    
            for (i = 0; i < size; ++i) {
	      if ( isnan( in[i] ) )
		continue;
	      
	      if ( (i > potentialSpikeIndex-NskipLeft) && ( i < potentialSpikeIndex+NskipRight ) )
		continue;

	      Cummlative +=  (in[i] - EstMean) * (in[i] - EstMean); 
	      //++Nsamples;
	    }
	    
	    double EstVariance = Cummlative / Nsamples;  // AKA sigma^2

// 	    	    printf("EstVariance = %f\n", EstVariance);

	    // calc distance of the spike to the mean val
	    Dis = abs(SpikeVal - EstMean);
	    
// 	    printf("Distance_ = %f = (%f - %f)\n", Dis, SpikeVal, EstMean );
	    
	    
	    // check for significance
	    double sigm = sqrt(EstVariance);
	    
// 	    printf("ld_vectorFindSpike; Distance = %f, sigma=%f, index=%d, Mean=%f, Variance=%f, Val=%f\n", Dis, sigma, potentialSpikeIndex, EstMean, EstVariance, SpikeVal );
	    
	    if (Dis > significanceFactor*sigm) {
	      // Yes it is a spike!
	      *FoundSpike = 1;
// 	      printf("ld_vectorFindSpike: Found spike\n");
	    } else {
	      // Probaply not a spike
	      *FoundSpike = 0;
	      
	    }
	    
	    // Further outputs
	    *index = potentialSpikeIndex;
	    *sigma = sigm;
	    *Mean = EstMean;
	    *Distance = Dis;
	    *Val = SpikeVal;
	    
	    

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

        libdyn_config_block_output(block, 0, 1, DATATYPE_INT32,1 ); // in, intype,
        libdyn_config_block_output(block, 1, 1, DATATYPE_INT32,1 );
       libdyn_config_block_output(block, 2, 1, DATATYPE_FLOAT,1 );
       libdyn_config_block_output(block, 3, 1, DATATYPE_FLOAT,1 );
       libdyn_config_block_output(block, 4, 1, DATATYPE_FLOAT,1 );
       libdyn_config_block_output(block, 5, 1, DATATYPE_FLOAT,1 );
	
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
        printf("I'm a ld_vectorFindSpike block\n");
        return 0;
        break;

    }
}



int ortd_compu_func_vectorglue(int flag, struct dynlib_block_t *block)
{
    // printf("comp_func demux: flag==%d\n", flag);
    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);

    int veclen = ipar[0];
    int Nout = 2;
    int Nin = 6;

    int outlen = 2*veclen;
    int k = 0;

    double *in1;
    double *in2;
    double *fromind1;
    double *fromind2;
    double *toind1;
    double *toind2;

    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        in1 = (double *) libdyn_get_input_ptr(block,0);
	fromind1 = (double *) libdyn_get_input_ptr(block,1);
	toind1 = (double *) libdyn_get_input_ptr(block,2);
        in2 = (double *) libdyn_get_input_ptr(block,3);
        fromind2 = (double *) libdyn_get_input_ptr(block,4);
	toind2 = (double *) libdyn_get_input_ptr(block,5);
        double *out = (double *) libdyn_get_output_ptr(block, 0);
	double *valnum = (double *) libdyn_get_output_ptr(block, 1);
	
	valnum[0] = 0.0;

	int ifromind1 = fromind1[0];
	ifromind1 = ifromind1 - 1; // to match the c-way of counting indices
	int itoind1 = toind1[0];
	itoind1 = itoind1 - 1; // to match the c-way of counting indices
        int ifromind2 = fromind2[0];
	ifromind2 = ifromind2 - 1; // to match the c-way of counting indices
	int itoind2 = toind2[0];
	itoind2 = itoind2 - 1; // to match the c-way of counting indices

	if(ifromind1 < 0)
	  goto glue_error;
	
	if(itoind1 >= veclen)
	  goto glue_error;

	if(itoind1 < ifromind1)
	  goto glue_error;
	
	if(ifromind2 < 0)
	  goto glue_error;
	
	if(itoind2 >= veclen)
	  goto glue_error;
	
	if(itoind2 < ifromind2)
	  goto glue_error;

	int cpylen1 = itoind1 - ifromind1;
	int cpylen2 = itoind2 - ifromind2;

        memcpy(out, (in1 + ifromind1), sizeof(double) * cpylen1);
	memcpy((out + cpylen1), (in2 + ifromind2), sizeof(double) * cpylen2);
	
	valnum[0] = (double)(cpylen1 + cpylen2);
        
	  for(k = (cpylen1 + cpylen2 + 1); k < outlen; k++){
	    out[k] = 0.0;
	  }
	  //  fprintf(stderr, "Good call of vectorglue block.\n");
	  
	  return 0;
	  
	  glue_error:
	   // fprintf(stderr, "Inconsistent call of vectorglue block.\n");
	    return -1;
    }
    return 0;
    break;
    case COMPF_FLAG_UPDATESTATES:
        return 0;
        break;
    case COMPF_FLAG_CONFIGURE:  // configure
    {
        libdyn_config_block(block, BLOCKTYPE_STATIC, Nout, Nin, (void *) 0, 0);

        libdyn_config_block_output(block, 0, outlen, DATATYPE_FLOAT,1 );
	libdyn_config_block_output(block, 1, 1, DATATYPE_FLOAT,1 );
        libdyn_config_block_input(block, 0, veclen, DATATYPE_FLOAT);
        libdyn_config_block_input(block, 1, 1, DATATYPE_FLOAT);
	libdyn_config_block_input(block, 2, 1, DATATYPE_FLOAT);
        libdyn_config_block_input(block, 3, veclen, DATATYPE_FLOAT);
        libdyn_config_block_input(block, 4, 1, DATATYPE_FLOAT);
	libdyn_config_block_input(block, 5, 1, DATATYPE_FLOAT);
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
        printf("I'm a vectorglue block\n");
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



int ortd_compu_func_vectoradd(int flag, struct dynlib_block_t *block)
{
    // printf("comp_func demux: flag==%d\n", flag);
    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);

    int size = ipar[0];
    int Nout = 1;
    int Nin = 2;

    double *in1;
    double *in2;


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        in1 = (double *) libdyn_get_input_ptr(block,0);
	in2 = (double *) libdyn_get_input_ptr(block,1);

        double *out = (double *) libdyn_get_output_ptr(block, 0);

        int i;
        for (i=0; i < size; ++i) {
            out[i] = ( in1[ i ] + in2[ i ]  );
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
        printf("I'm vectoradd block\n");
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

int ortd_compu_func_vectorabssum(int flag, struct dynlib_block_t *block)
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

#define abs(a) ( (a) > 0 ? (a) : (-a) )
	
        for (i=0; i < size; ++i) {
            sum += ( abs( in[i] ) );
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

int ortd_compu_func_vectorsqsum(int flag, struct dynlib_block_t *block)
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
            sum += (  in[i]*in[i]  );
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
        printf("I'm vectorsqsum block\n");
        return 0;
        break;

    }
}

int ortd_compu_func_vectorextractandsum(int flag, struct dynlib_block_t *block)
{
    // printf("comp_func demux: flag==%d\n", flag);
    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);

    int size = ipar[0];
    int Nout = 1;
    int Nin = 3;

    double *in;


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        in = (double *) libdyn_get_input_ptr(block,0);
        double *from = (double *) libdyn_get_input_ptr(block, 1);
        double *to = (double *) libdyn_get_input_ptr(block, 2);

        double *out = (double *) libdyn_get_output_ptr(block, 0);

        int from_ = *from; // round to integer
        int to_ = *to;
	
        if (from_ < 1) from_ = 1;
        if (from_ > size) from_ = size;

        if (to_ < 1) to_ = 1;
        if (to_ > size) to_ = size;

        int i;

	// sum-up
	out[i] = 0;
        for (i=from_; i <= to_; ++i) {
            out[i] += ( in[ i - 1  ]  );
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
        libdyn_config_block_input(block, 1, 1, DATATYPE_FLOAT); // from input
        libdyn_config_block_input(block, 2, 1, DATATYPE_FLOAT); // to input
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
        printf("I'm extract and sum-up block\n");
        return 0;
        break;

    }
}

int ortd_compu_func_simplecovar(int flag, struct dynlib_block_t *block)
{
    // printf("comp_func demux: flag==%d\n", flag);
    int *ipar = libdyn_get_ipar_ptr(block);
    double *shape = libdyn_get_rpar_ptr(block); // reference-vector

    int size = ipar[0]; // len input vector
    int shape_len = ipar[1]; // len shape sample

    int Nout = 1;
    int Nin = 1;

    double *in;

    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        in = (double *) libdyn_get_input_ptr(block,0);
	double *out = (double *) libdyn_get_output_ptr(block, 0);

	int i,j;
	double sum;
	for (i = 1; i <= size-shape_len+1; ++i) { // shift the windown within which the shape is compared through the input vector

          // apply the sample "shape" to the current position i in the input vector
	  sum = 0;
	  for (j = 1; j <= shape_len; ++j) {
	    sum = sum + ( in[ i+j-2 ] * shape[ j-1 ] );  // -1 and -2 is to match the C-way of counting indices
	  }
	  
	  out[ i-1 ] = sum;
	}
    }
    return 0;
    break;
    case COMPF_FLAG_UPDATESTATES:
        return 0;
        break;
    case COMPF_FLAG_CONFIGURE:  // configure
    {
        if (size < shape_len) {
            printf("size cannot be smaller than shape_len\n");
            printf("size = %d\n", size);

            return -1;
        }
        if (shape_len < 1) {
            printf("shape_len cannot be smaller than 1\n");
            printf("shape_len = %d\n", size);

            return -1;
        }
        if (shape_len > size) {
            printf("shape_len > size !\n");

            return -1;
        }

        libdyn_config_block(block, BLOCKTYPE_STATIC, Nout, Nin, (void *) 0, 0);

        libdyn_config_block_output(block, 0, size-shape_len+1, DATATYPE_FLOAT,1 ); // in, intype,
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
        printf("I'm covar block\n");
        return 0;
        break;

    }
}


int ortd_compu_func_ld_vectorFindShape(int flag, struct dynlib_block_t *block)
{
          printf("ortd_compu_func_ld_vectorFindShape: flag==%d\n", flag);

    // printf("comp_func demux: flag==%d\n", flag);
    int *ipar = libdyn_get_ipar_ptr(block);
    double *shape = libdyn_get_rpar_ptr(block); // reference-vector

    int size = ipar[0]; // len input vector
    int shape_len = ipar[1]; // len shape sample
    int windowSize = shape_len + 2; // + 2 because of the signal ofs detection prior and after the sub-window for shape comparison 

    int Nout = 1;
    int Nin = 1;

    double *in;

    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        in = (double *) libdyn_get_input_ptr(block,0);
	double *out = (double *) libdyn_get_output_ptr(block, 0);

	int i,j;
	double sum;
	
	
	for (i = 1; i <= size-windowSize+1; ++i) { // shift the windown within which the shape is compared through the input vector

	  
	  // calc signal ofs around the shape window
	  double ofs = ( in[i - 1] + in[i-1 + shape_len + 1] ) / 2;
	  
	//  printf("FS: i=%d, ofs= mean( in(%d)=%f , in(%d)=%f ) =  %f\n", i, i-1, in[i-1],   i-1 + shape_len + 1, in[i-1 + shape_len + 1], ofs);
	  
          // apply the sample "shape" to the current position i in the input vector
	  sum = 0;
	  for (j = 2; j <= shape_len+1; ++j) { // j=2:(shape_len+1)
	    sum = sum + ( (in[ i+j-2  ] - ofs) * shape[ j-1-1 ] );  // -1 and -2 is to match the C-way of counting indices; +1 because of ofs estimation
	    
	//    printf("  FS: j=%d, apply shape to in(%d)=%f\n", j, i+j-2, in[i+j-2] );
	  }
	  
	 // printf("FS: sum=%f\n", sum);
	  
	  out[ i-1 ] = sum;
	}
    }
    return 0;
    break;
    case COMPF_FLAG_UPDATESTATES:
        return 0;
        break;
    case COMPF_FLAG_CONFIGURE:  // configure
    {
        if (size < windowSize) {
            printf("size cannot be smaller than windowSize\n");
            printf("size = %d\n", size);

            return -1;
        }
        if (shape_len < 1) {
            printf("shape_len cannot be smaller than 1\n");
            printf("shape_len = %d\n", size);

            return -1;
        }
        if (windowSize > size) {
            printf("windowSize > size !\n");

            return -1;
        }

        libdyn_config_block(block, BLOCKTYPE_STATIC, Nout, Nin, (void *) 0, 0);

        libdyn_config_block_output(block, 0, size-windowSize+1, DATATYPE_FLOAT,1 ); // in, intype,
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
        printf("I'm a ld_vectorFindShape block\n");
        return 0;
        break;

    }
}


int ortd_compu_func_vectormute(int flag, struct dynlib_block_t *block)
{
    // printf("comp_func demux: flag==%d\n", flag);
    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);

    int size = ipar[0];
    int Nout = 1;
    int Nin = 4;

    double *in;


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        in = (double *) libdyn_get_input_ptr(block,0);
        double *from = (double *) libdyn_get_input_ptr(block, 1);
        double *len  = (double *) libdyn_get_input_ptr(block, 2);
        double *setto = (double *) libdyn_get_input_ptr(block, 3);

        double *out = (double *) libdyn_get_output_ptr(block, 0);

        int from_ = *from; // round to integer
        int len_ = *len;
	
	
	int to_ = from_ + len_ - 1;
// 	printf("to_ %d\n", to_);
	
        if (from_ < 1) from_ = 1;
	if (from_ > size) from_ = size;
	if (to_ > size) to_ = size;
	if (to_ < 1) to_ = 1;

//  	printf("to_ %d from_ %d\n", to_, from_);
	
        int i;

	// copy 1. part
	for (i=1; i <= from_-1; ++i) {
// 	  printf("copy p1 %d\n", i);
	  out[i-1] = in[i-1];
	}
	
	// copy 2. part
	for (i=to_; i <= size; ++i) {
// 	  printf("copy p2 %d\n", i);
	  out[i-1] = in[i-1];
	} 
	
	// mute
        for (i=from_; i <= to_; ++i) {
// 	  printf("mute %d\n", i);
            out[ i - 1  ] = *setto;
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
	
        libdyn_config_block_input(block, 1, 1, DATATYPE_FLOAT); // from
        libdyn_config_block_input(block, 2, 1, DATATYPE_FLOAT); // len
        libdyn_config_block_input(block, 3, 1, DATATYPE_FLOAT); // setto
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
        printf("I'm vectormute block\n");
        return 0;
        break;

    }
}


int ortd_compu_func_NaNToVal(int flag, struct dynlib_block_t *block)
{
    // printf("comp_func demux: flag==%d\n", flag);
    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);

    int size = ipar[0];
    double Val = rpar[0];
    int Nout = 1;
    int Nin = 1;

    double *in;


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        in = (double *) libdyn_get_input_ptr(block,0);
        double *out = (double *) libdyn_get_output_ptr(block, 0);
        int i;
	
// 	#define isNaN(a) ( (a) == (a) ? 0 : 1 )
	
        for (i=0; i < size; ++i) {
	  if (isnan(in[i])) {	    
            out[i] = Val;
	  } else {
	    out[i] = in[i];
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
        printf("I'm a nan to val block\n");
        return 0;
        break;

    }
}


int ortd_compu_func_eventDemux(int flag, struct dynlib_block_t *block)
{
    // printf("comp_func demux: flag==%d\n", flag);
    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);

    int NrEvents = ipar[0];
    int KeepOutputLevel = ipar[1];
    int Nout = NrEvents;
    int Nin = 1;

    double *in;


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        in = (double *) libdyn_get_input_ptr(block,0);
        double *out;
	
        int i;
        for (i=0; i < NrEvents; ++i) {
	  out = (double *) libdyn_get_output_ptr(block, i);
	  
	  int roundedinput = round(*in);
	  if (roundedinput == i+1) {
	    *out = 1;
// 	    printf("event %d is true\n", i+1);
	  } else {
	    *out = 0;
// 	    printf("event %d is false\n", i+1);
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
        libdyn_config_block(block, BLOCKTYPE_STATIC, Nout, Nin, (void *) 0, 0);

        libdyn_config_block_input(block, 0, 1, DATATYPE_FLOAT);
	
	int i;
	for (i=0; i<NrEvents; ++i) {
          libdyn_config_block_output(block, i, 1, DATATYPE_FLOAT,1 ); // in, intype,
	}
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
        printf("I'm a eventDemux block\n");
        return 0;
        break;

    }
}


int ortd_compu_func_TrigSwitch1toN(int flag, struct dynlib_block_t *block)
{
    // printf("comp_func demux: flag==%d\n", flag);
    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);

    int N = ipar[0];
    int InitialState = ipar[1];
    int Nout = 1;
    int Nin = 1+N;

    int *state = (void*) libdyn_get_work_ptr(block);
    
    double *Event;
    double *SwitchInput;

    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        Event = (double *) libdyn_get_input_ptr(block,0);
	
	if (*Event > 0.5) { // update state
	  int E = round(*Event);
	  if (E > N) {
	    // error --> do not change the state
	  } else {
	    *state = E-1;
	  }
	}
	
	SwitchInput = (double *) libdyn_get_input_ptr(block, *state+1); // use input #state

	double *out = (double *) libdyn_get_output_ptr(block,0);

	// copy the input
	*out = *SwitchInput;
    }
    return 0;
    break;
    case COMPF_FLAG_UPDATESTATES:
        return 0;
        break;
    case COMPF_FLAG_RESETSTATES:
    {
        double *out = (double *) libdyn_get_output_ptr(block,0);
// 	if (InitialState <= N && InitialState >= 1) {	  
          *state = InitialState-1;
// 	} else {
// 	  *state = 0; // invalid initialisation
// 	}
	*out = 0;
//         *output = (state[0] > 0) ? onout : offout ;  // Added this 14.5.2012
    }
        return 0;
        break;	
    case COMPF_FLAG_CONFIGURE:  // configure
    {
        libdyn_config_block(block, BLOCKTYPE_STATIC, Nout, Nin, (void *) 0, 0);

        libdyn_config_block_input(block, 0, 1, DATATYPE_FLOAT); // Event input
	
	int i;
	for (i=0; i<N; ++i) {
  	  libdyn_config_block_input(block, i+1, 1, DATATYPE_FLOAT);
	}

        libdyn_config_block_output(block, 0, 1, DATATYPE_FLOAT,1 ); // in, intype,
    }
    return 0;
    break;
    case COMPF_FLAG_INIT:  // init
{
        int *state = malloc(sizeof(int));
        libdyn_set_work_ptr(block, (void *) state);
	
	if (InitialState <= N && InitialState >= 1) {	  
          *state = InitialState-1;
	} else {
	  fprintf(stderr, "ortd_compu_func_TrigSwitch1toN: WARNING: wrong initialisation for InitialState!\n");
	  *state = 0; // invalid initialisation
	  free(state);
	  return -1;
	}
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
        printf("I'm a TrigSwitch1toN block\n");
        return 0;
        break;

    }
}




int ortd_compu_func_vectorConcatenate(int flag, struct dynlib_block_t *block)
{
    // printf("comp_func demux: flag==%d\n", flag);
    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);

    int size1 = ipar[0];
    int size2 = ipar[1];
    int Nout = 1;
    int Nin = 2;

    double *in1, *in2;


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        in1 = (double *) libdyn_get_input_ptr(block,0);
        in2 = (double *) libdyn_get_input_ptr(block,1);

        double *out = (double *) libdyn_get_output_ptr(block, 0);

        int i,j;
	j=0;
	
        for (i=0; i < size1; ++i) {
            out[j] = in1[i];
	    j++;
        }
        
        for (i=0; i < size2; ++i) {
            out[j] = in2[i];
	    j++;
        }
        

    }
    return 0;
    break;
    case COMPF_FLAG_UPDATESTATES:
        return 0;
        break;
    case COMPF_FLAG_CONFIGURE:  // configure
    {
        if (size1 < 1 || size2 < 1) {
            return -1;
        }

        libdyn_config_block(block, BLOCKTYPE_STATIC, Nout, Nin, (void *) 0, 0);

        libdyn_config_block_output(block, 0, size1+size2, DATATYPE_FLOAT,1 ); // in, intype,
        libdyn_config_block_input(block, 0, size1, DATATYPE_FLOAT);
        libdyn_config_block_input(block, 1, size2, DATATYPE_FLOAT); // from input
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
        printf("I'm vectorcat block\n");
        return 0;
        break;

    }
}


int ortd_compu_func_vectormultscalar(int flag, struct dynlib_block_t *block)
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
        double *tomult = (double *) libdyn_get_input_ptr(block, 1);

        double *out = (double *) libdyn_get_output_ptr(block, 0);

        int i;
        for (i=0; i < size; ++i) {
            out[i] = ( in[ i ] * *tomult  );
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
        printf("I'm a vectormultscalar block\n");
        return 0;
        break;

    }
}



int ortd_compu_func_collectValues(int flag, struct dynlib_block_t *block)
{
//       printf("ortd_compu_func_collectValues: flag==%d\n", flag);

  int err;
  
  int Nout = 1;
  int Nin = 2;

  int *ipar = libdyn_get_ipar_ptr(block);
  int memorysize = ipar[0];
  int inVecsize = ipar[1];
  
  int *rpar = libdyn_get_rpar_ptr(block);
  double DefaultVal = rpar[0];
  

  
  int dfeed = 0;  
  int i = 0;
  
  double *out;
  double *in;
  int32_t *WriteIndex;
  

  
  switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
     
       // printf("-- CALCOUT --\n");
                 out = (double *) libdyn_get_output_ptr(block,0);
      in = (double *) libdyn_get_input_ptr(block,0);
      WriteIndex = (int32_t *) libdyn_get_input_ptr(block,1);
      
  

      if ( *WriteIndex < 1 ) { 
          fprintf(stderr, "ld_collectValues: Failed: WriteIndex < 1\n");
          return 0;
          break;
      } 
      
      if ( *WriteIndex-1 + inVecsize > memorysize ) { 
          fprintf(stderr, "ld_collectValues: Failed: Trying to write data bejond the buffer: WriteIndex-1 + inVecsize > memorysize\n");
          return 0;
          break;
      } 

        for(i = 0; i < inVecsize; i++){
            // copy values
           // printf("write %f to %d \n", in[i], *WriteIndex-1+i ); 
            
            out[ *WriteIndex-1+i ] = in[i];
        }
      
      return 0;
      break;
    }
    case COMPF_FLAG_UPDATESTATES:
    {
      return 0;
      break;
    }
    case COMPF_FLAG_CONFIGURE:  // configure
    {
      libdyn_config_block(block, BLOCKTYPE_DYNAMIC, Nout, Nin, (void *) 0, 0);  // no dfeed
      libdyn_config_block_input(block, 0, inVecsize, DATATYPE_FLOAT); // in, intype, 
      libdyn_config_block_input(block, 1, 1, DATATYPE_INT32); // in, intype, 
      libdyn_config_block_output(block, 0, memorysize, DATATYPE_FLOAT, 1);
  
      return 0;
      break;
    }
    case COMPF_FLAG_INIT:  // configure
    {      
               out = (double *) libdyn_get_output_ptr(block,0);
      in = (double *) libdyn_get_input_ptr(block,0);
      WriteIndex = (int32_t *) libdyn_get_input_ptr(block,1);
      
  
      if (inVecsize < 1) {
	fprintf(stderr, "ld_collectValues: invalid inVecsize\n");
	return -1;
      }
      
      if (memorysize < 1) {
	fprintf(stderr, "ld_collectValues: invalid memorysize\n");
	return -1;
      }
      
        for (i=0; i<memorysize; ++i) {
            out[i] = DefaultVal;
        }
        
    }
      return 0;
      break;
    case COMPF_FLAG_RESETSTATES: // destroy instance
    {
                 out = (double *) libdyn_get_output_ptr(block,0);
      in = (double *) libdyn_get_input_ptr(block,0);
      WriteIndex = (int32_t *) libdyn_get_input_ptr(block,1);
      
  
        for (i=0; i<memorysize; ++i) {
            out[i] = DefaultVal;
        }
        

// 
     }
      return 0;
      break;      
    case COMPF_FLAG_DESTUCTOR: // destroy instance
    {
//       void *buffer = (void*) libdyn_get_work_ptr(block);
//       free(buffer);
    }
      return 0;
      break;
      
    case COMPF_FLAG_PRINTINFO:
      printf("I'm a ld_collectValues block.\n");
      return 0;
      break;
  }
}







int ortd_compu_func_ld_HistogramInt32(int flag, struct dynlib_block_t *block)
{
//     printf("ortd_compu_func_ld_HistogramInt32: flag==%d\n", flag);

  
  int err;
  
  int Nout = 1;
  int Nin = 2;

  int *ipar = libdyn_get_ipar_ptr(block);
  int from = ipar[0];
  int to = ipar[1];
  
  int *rpar = libdyn_get_rpar_ptr(block);
  double DefaultVal = rpar[0];
  
  
  int dfeed = 0;  
  int i = 0;
  
  int32_t *out;
  int32_t *Val;
  int32_t *Weight;


  
  switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
             out = (int32_t *) libdyn_get_output_ptr(block,0);
      Val = (int32_t *) libdyn_get_input_ptr(block,0);
      Weight = (int32_t *) libdyn_get_input_ptr(block,1);
      
       // printf("-- CALCOUT --\n");
        

      if ( *Val < from ) { 
          //fprintf(stderr, "ld_collectValues: Failed: WriteIndex < 1\n");
          return 0;
          break;
      } 
      
      if ( *Val > to ) { 
          //fprintf(stderr, "ld_collectValues: Failed: WriteIndex < 1\n");
          return 0;
          break;
      } 

	// Accumulate Weight
            out[ *Val - from ] += *Weight;
        
      
      return 0;
      break;
    }
    case COMPF_FLAG_UPDATESTATES:
    {
      return 0;
      break;
    }
    case COMPF_FLAG_CONFIGURE:  // configure
    {
      libdyn_config_block(block, BLOCKTYPE_DYNAMIC, Nout, Nin, (void *) 0, 0);  // no dfeed
      libdyn_config_block_input(block, 0, 1, DATATYPE_INT32); // in, intype, 
      libdyn_config_block_input(block, 1, 1, DATATYPE_INT32); // in, intype, 
      libdyn_config_block_output(block, 0, to-from+1, DATATYPE_INT32, 1);
  
      return 0;
      break;
    }
    case COMPF_FLAG_INIT:  // configure
    {      
                   out = (int32_t *) libdyn_get_output_ptr(block,0);
      Val = (int32_t *) libdyn_get_input_ptr(block,0);
      Weight = (int32_t *) libdyn_get_input_ptr(block,1);
      
      if (from > to ) {
	fprintf(stderr, "ld_HistogramInt32: from > to\n");
	return -1;
      }
      
      // set output to zero
      for (i=0; i< (to-from+1); ++i) {
	out[i]=0;
      }

    }
      return 0;
      break;
    case COMPF_FLAG_RESETSTATES: // destroy instance
    {
                    out = (int32_t *) libdyn_get_output_ptr(block,0);
      Val = (int32_t *) libdyn_get_input_ptr(block,0);
      Weight = (int32_t *) libdyn_get_input_ptr(block,1);
      
      // set output to zero
      for (i=0; i< (to-from+1); ++i) {
	out[i]=0;
      }

     }
      return 0;
      break;      
    case COMPF_FLAG_DESTUCTOR: // destroy instance
    {
//       void *buffer = (void*) libdyn_get_work_ptr(block);
//       free(buffer);
    }
      return 0;
      break;
      
    case COMPF_FLAG_PRINTINFO:
      printf("I'm a ld_HistogramInt32 block.\n");
      return 0;
      break;
  }
}




// [sim, TimerActive, Counter] = ld_Timer(sim, 0, Trigger=AbnormalityDetected, Count=length(par_.CorrModel) )

int ortd_compu_func_ld_Timer(int flag, struct dynlib_block_t *block)
{
//   printf("ortd_compu_func_ld_Timer: flag==%d\n", flag);
  
  int err;
  
  int Nout = 2;
  int Nin = 1;

  int *ipar = libdyn_get_ipar_ptr(block);
  int Count = ipar[0];

  
  int *rpar = libdyn_get_rpar_ptr(block);
  
  int i = 0;
  
  int32_t *TimerActive;
  int32_t *Counter;
  int32_t *Trigger;


  
  switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
           TimerActive = (int32_t *) libdyn_get_output_ptr(block,0);
      Counter = (int32_t *) libdyn_get_output_ptr(block,1);
      Trigger = (int32_t *) libdyn_get_input_ptr(block,0);
      
       // printf("-- CALCOUT --\n");
        

	if (*TimerActive == 1) {
	  *Counter -= 1;
	  
	  if (*Counter == 0) {
	    *TimerActive = 0;
	  }
	}
      
	if ( *Trigger > 0 ) { 
	    
	    if (*TimerActive == 0) {
	      *Counter = Count; // reset counter
	      *TimerActive = 1;
	    }
	}

      return 0;
      break;
    }
    case COMPF_FLAG_UPDATESTATES:
    {
      return 0;
      break;
    }
    case COMPF_FLAG_CONFIGURE:  // configure
    {
      libdyn_config_block(block, BLOCKTYPE_DYNAMIC, Nout, Nin, (void *) 0, 0);  // no dfeed
      libdyn_config_block_input(block, 0, 1, DATATYPE_INT32); // in, intype, 
      libdyn_config_block_output(block, 0, 1, DATATYPE_INT32, 1);
      libdyn_config_block_output(block, 1, 1, DATATYPE_INT32, 1);
  
      return 0;
      break;
    }
    case COMPF_FLAG_INIT:  // configure
    {      
      TimerActive = (int32_t *) libdyn_get_output_ptr(block,0);
      Counter = (int32_t *) libdyn_get_output_ptr(block,1);
      Trigger = (int32_t *) libdyn_get_input_ptr(block,0);
      
	      *Counter = 0; // reset counter
	      *TimerActive = 0;
    }
      return 0;
      break;
    case COMPF_FLAG_RESETSTATES: // destroy instance
    {
      TimerActive = (int32_t *) libdyn_get_output_ptr(block,0);
      Counter = (int32_t *) libdyn_get_output_ptr(block,1);
      Trigger = (int32_t *) libdyn_get_input_ptr(block,0);
      
	      *Counter = 0; // reset counter
	      *TimerActive = 0;
     }
      return 0;
      break;      
    case COMPF_FLAG_DESTUCTOR: // destroy instance
    {
//       void *buffer = (void*) libdyn_get_work_ptr(block);
//       free(buffer);
    }
      return 0;
      break;
      
    case COMPF_FLAG_PRINTINFO:
      printf("I'm a ld_Timer block.\n");
      return 0;
      break;
  }
}


int compu_func_ld_add_ofsInt32(int flag, struct dynlib_block_t *block)
{
    //  printf("comp_func mux: flag==%d; irparid = %d\n", flag, block->irpar_config_id);
    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);

    int Nout = 1;
    int Nin = 1;
    
    int ofs=ipar[0];

    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        int32_t *out = (int32_t *) libdyn_get_output_ptr(block,0);
        int32_t *in = (int32_t *) libdyn_get_input_ptr(block, 0);
    
        *out = (*in) + ofs;
    }
    return 0;
    break;
    case COMPF_FLAG_UPDATESTATES:
        return 0;
        break;
    case COMPF_FLAG_CONFIGURE:  // configure
    {
        libdyn_config_block(block, BLOCKTYPE_STATIC, Nout, Nin, (void *) 0, 0);

        libdyn_config_block_input(block, 0, 1, DATATYPE_INT32);
        libdyn_config_block_output(block, 0, 1, DATATYPE_INT32, 1);
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
        printf("I'm a ld_add_ofsInt32 block\n");
        return 0;
        break;

    }
}


int compu_func_ld_gainInt32(int flag, struct dynlib_block_t *block)
{
    //  printf("comp_func mux: flag==%d; irparid = %d\n", flag, block->irpar_config_id);
    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);

    int Nout = 1;
    int Nin = 1;
    
    int fac=ipar[0];

    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        int32_t *out = (int32_t *) libdyn_get_output_ptr(block,0);
        int32_t *in = (int32_t *) libdyn_get_input_ptr(block, 0);
    
        *out = (*in) * fac;
    }
    return 0;
    break;
    case COMPF_FLAG_UPDATESTATES:
        return 0;
        break;
    case COMPF_FLAG_CONFIGURE:  // configure
    {
        libdyn_config_block(block, BLOCKTYPE_STATIC, Nout, Nin, (void *) 0, 0);

        libdyn_config_block_input(block, 0, 1, DATATYPE_INT32);
        libdyn_config_block_output(block, 0, 1, DATATYPE_INT32, 1);
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
        printf("I'm a ld_gainInt32 block\n");
        return 0;
        break;

    }
}

/*

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
        output = (double *) libdyn_get_output_ptr(block,0);

        *state = initial_state;
        *output = (state[0] > 0) ? onout : offout ;  // Added this 14.5.2012

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
}*/



/*
   Special blocks
*/

int compu_func_interface2(int flag, struct dynlib_block_t *block)
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
      
/*      int i;
      for (i=0; i<len; ++i) // copy in to out FIXME: DO this by setting outptr to inptr
        out[i] = in[i];*/
      
      memcpy((void*) out, (void*) in, sizeof(double) * len);
      
      return 0;
      break;
    case COMPF_FLAG_UPDATESTATES:
      return 0;
      break;
    case COMPF_FLAG_RESETSTATES:
      out = (double *) libdyn_get_output_ptr(block,0);
      
      // set output to zero
      memset((void*) out, 0, sizeof(double) * len );
      
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


// } // extern "C" 


int libdyn_module_basic_ldblocks_siminit_Cpp(struct dynlib_simulation_t *sim, int bid_ofs);


int libdyn_module_basic_ldblocks_siminit(struct dynlib_simulation_t *sim, int bid_ofs)
// extern "C" int libdyn_module_basic_ldblocks_siminit(struct dynlib_simulation_t *sim, int bid_ofs)
{
    // printf("libdyn module siminit function called\n");

    // Register my blocks to the given simulation

    int blockid_ofs = 60001;

#ifdef DEBUG
    fprintf(stderr, "Adding basic ld_blocks module\n");
#endif

    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs, LIBDYN_COMPFN_TYPE_LIBDYN,  (void*) &compu_func_switch2to1);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 1, LIBDYN_COMPFN_TYPE_LIBDYN,  (void*) &compu_func_demux);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 2, LIBDYN_COMPFN_TYPE_LIBDYN,  (void*) &compu_func_mux);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 3, LIBDYN_COMPFN_TYPE_LIBDYN,  (void*) &ortd_compu_func_hysteresis);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 4, LIBDYN_COMPFN_TYPE_LIBDYN,  (void*) &ortd_compu_func_modcounter);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 5, LIBDYN_COMPFN_TYPE_LIBDYN,  (void*) &ortd_compu_func_jumper);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 6, LIBDYN_COMPFN_TYPE_LIBDYN,  (void*) &ortd_compu_func_memory);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 7, LIBDYN_COMPFN_TYPE_LIBDYN,  (void*) &compu_func_abs);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 8, LIBDYN_COMPFN_TYPE_LIBDYN,  (void*) &compu_func_extract_element);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 9, LIBDYN_COMPFN_TYPE_LIBDYN,  (void*) &compu_func_constvec);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 10, LIBDYN_COMPFN_TYPE_LIBDYN,  (void*) &ortd_compu_func_counter);
    // MISSING HERE: ld_shift register
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 12, LIBDYN_COMPFN_TYPE_LIBDYN,  (void*) &compu_func_lookuptable);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 13, LIBDYN_COMPFN_TYPE_LIBDYN,  (void*) &compu_func_not);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 14, LIBDYN_COMPFN_TYPE_LIBDYN,  (void*) &compu_func_ld_or);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 15, LIBDYN_COMPFN_TYPE_LIBDYN,  (void*) &compu_func_ld_iszero);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 16, LIBDYN_COMPFN_TYPE_LIBDYN,  (void*) &ortd_compu_func_limitedcounter);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 17, LIBDYN_COMPFN_TYPE_LIBDYN,  (void*) &ortd_compu_func_memorydel);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 18, LIBDYN_COMPFN_TYPE_LIBDYN,  (void*)  &ortd_compu_func_steps);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 19, LIBDYN_COMPFN_TYPE_LIBDYN,   (void*) &compu_func_ld_cond_overwrite);
    // MISSING HERE
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 21, LIBDYN_COMPFN_TYPE_LIBDYN,   (void*) &compu_func_ld_and);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 22, LIBDYN_COMPFN_TYPE_LIBDYN,   (void*) &ortd_compu_func_ld_initimpuls);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 23, LIBDYN_COMPFN_TYPE_LIBDYN,   (void*) &compu_func_printfstderr);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 24, LIBDYN_COMPFN_TYPE_LIBDYN,   (void*) &compu_func_delay);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 25, LIBDYN_COMPFN_TYPE_LIBDYN,   (void*) &ortd_compu_func_steps2);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 26, LIBDYN_COMPFN_TYPE_LIBDYN,   (void*) &compu_func_ld_getsign);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 27, LIBDYN_COMPFN_TYPE_LIBDYN,   (void*) &compu_func_insert_element);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 28, LIBDYN_COMPFN_TYPE_LIBDYN,   (void*) &compu_func_FlagProbe);    
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 29, LIBDYN_COMPFN_TYPE_LIBDYN,   (void*) &compu_func_printfbar);

    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 30, LIBDYN_COMPFN_TYPE_LIBDYN,   (void*) &compu_func_ld_ceilInt32);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 31, LIBDYN_COMPFN_TYPE_LIBDYN,   (void*) &compu_func_ld_Int32ToFloat);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 32, LIBDYN_COMPFN_TYPE_LIBDYN,   (void*) &compu_func_ld_floorInt32);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 33, LIBDYN_COMPFN_TYPE_LIBDYN,   (void*) &compu_func_ld_roundInt32);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 34, LIBDYN_COMPFN_TYPE_LIBDYN,   (void*) &compu_func_constvecInt32);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 35, LIBDYN_COMPFN_TYPE_LIBDYN,   (void*) &compu_func_ld_sumInt32);
    
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 36, LIBDYN_COMPFN_TYPE_LIBDYN,   (void*) &compu_func_printfstderr2);
    
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 37, LIBDYN_COMPFN_TYPE_LIBDYN,  (void*) &compu_func_demuxInt32);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 38, LIBDYN_COMPFN_TYPE_LIBDYN,  (void*) &compu_func_muxInt32);

    
    
//     TO INSERT
    
    
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 42, LIBDYN_COMPFN_TYPE_LIBDYN,   (void*) &compu_func_ld_CompareEqInt32);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 43, LIBDYN_COMPFN_TYPE_LIBDYN,   (void*) &compu_func_ld_cond_overwrite2);

    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 44, LIBDYN_COMPFN_TYPE_LIBDYN,   (void*) &compu_func_ld_andInt32);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 45, LIBDYN_COMPFN_TYPE_LIBDYN,   (void*) &compu_func_ld_orInt32);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 46, LIBDYN_COMPFN_TYPE_LIBDYN,   (void*) &compu_func_ld_notInt32);

    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 47, LIBDYN_COMPFN_TYPE_LIBDYN,   (void*) &compu_func_ld_CompareInt32);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 48, LIBDYN_COMPFN_TYPE_LIBDYN,   (void*) &compu_func_ld_integratorInt32);
    
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 49, LIBDYN_COMPFN_TYPE_LIBDYN,   (void*) &compu_func_ld_cond_overwriteInt32);
    
    
    
    

//     shift_register


    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 50, LIBDYN_COMPFN_TYPE_LIBDYN,  (void*)  &ortd_compu_func_vectordiff);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 51, LIBDYN_COMPFN_TYPE_LIBDYN,   (void*) &ortd_compu_func_vectorfindthr);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 52, LIBDYN_COMPFN_TYPE_LIBDYN,   (void*) &ortd_compu_func_vectorabs);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 53, LIBDYN_COMPFN_TYPE_LIBDYN,   (void*) &ortd_compu_func_vectorgain);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 54, LIBDYN_COMPFN_TYPE_LIBDYN,  (void*)  &ortd_compu_func_vectorextract);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 55, LIBDYN_COMPFN_TYPE_LIBDYN,  (void*)  &ortd_compu_func_vectorfindminmax);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 56, LIBDYN_COMPFN_TYPE_LIBDYN,  (void*)  &ortd_compu_func_vectoraddscalar);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 57, LIBDYN_COMPFN_TYPE_LIBDYN,  (void*)  &ortd_compu_func_vectorsum);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 58, LIBDYN_COMPFN_TYPE_LIBDYN,   (void*) &ortd_compu_func_vector_muladd);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 59, LIBDYN_COMPFN_TYPE_LIBDYN,   (void*) &ortd_compu_func_vectorabssum);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 60, LIBDYN_COMPFN_TYPE_LIBDYN,   (void*) &ortd_compu_func_vectorsqsum);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 61, LIBDYN_COMPFN_TYPE_LIBDYN,  (void*)  &ortd_compu_func_vectorextractandsum);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 62, LIBDYN_COMPFN_TYPE_LIBDYN,  (void*)  &ortd_compu_func_simplecovar);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 63, LIBDYN_COMPFN_TYPE_LIBDYN,   (void*) &ortd_compu_func_vectormute);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 64, LIBDYN_COMPFN_TYPE_LIBDYN,   (void*) &ortd_compu_func_vectorglue);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 65, LIBDYN_COMPFN_TYPE_LIBDYN,   (void*) &ortd_compu_func_vectordelay);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 66, LIBDYN_COMPFN_TYPE_LIBDYN,   (void*) &ortd_compu_func_vectoradd);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 67, LIBDYN_COMPFN_TYPE_LIBDYN,   (void*) &ortd_compu_func_NaNToVal);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 68, LIBDYN_COMPFN_TYPE_LIBDYN,  (void*)  &ortd_compu_func_eventDemux);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 69, LIBDYN_COMPFN_TYPE_LIBDYN,  (void*)  &ortd_compu_func_TrigSwitch1toN);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 70, LIBDYN_COMPFN_TYPE_LIBDYN,  (void*)  &ortd_compu_func_vectorConcatenate);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 71, LIBDYN_COMPFN_TYPE_LIBDYN,  (void*)  &ortd_compu_func_vectormultscalar);
    
    
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 72, LIBDYN_COMPFN_TYPE_LIBDYN,  (void*)  &compu_func_vector_lookuptable );
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 73, LIBDYN_COMPFN_TYPE_LIBDYN,   (void*) &compu_func_vector_ld_floorInt32);
    
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 74, LIBDYN_COMPFN_TYPE_LIBDYN,   (void*) &ortd_compu_func_collectValues);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 75, LIBDYN_COMPFN_TYPE_LIBDYN,   (void*) &compu_func_ld_add_ofsInt32);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 76, LIBDYN_COMPFN_TYPE_LIBDYN,   (void*) &compu_func_ld_gainInt32);
    
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 77, LIBDYN_COMPFN_TYPE_LIBDYN,   (void*) &compu_func_vector_ld_Int32ToFloat);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 78, LIBDYN_COMPFN_TYPE_LIBDYN,   (void*) &compu_func_ld_printfInt32 );
    
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 79, LIBDYN_COMPFN_TYPE_LIBDYN,   (void*) &ortd_compu_func_ld_vectorFindShape );

    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 80, LIBDYN_COMPFN_TYPE_LIBDYN,   (void*) &ortd_compu_func_ld_vectorFindSpike );
    
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 81, LIBDYN_COMPFN_TYPE_LIBDYN,   (void*) &ortd_compu_func_ld_HistogramInt32 );
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 82, LIBDYN_COMPFN_TYPE_LIBDYN,   (void*) &ortd_compu_func_ld_Timer );
    
    
    
    

    
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 1000, LIBDYN_COMPFN_TYPE_LIBDYN,   (void*) &compu_func_interface2);
    
    
    
    
    // Blocks in the new C++ Template style
    libdyn_module_basic_ldblocks_siminit_Cpp(sim,0);
    
    
    
}


//} // extern "C"
