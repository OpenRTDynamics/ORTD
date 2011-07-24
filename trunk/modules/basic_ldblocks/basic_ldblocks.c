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




//#include "block_lookup.h"

int libdyn_module_basic_ldblocks_siminit(struct dynlib_simulation_t *sim, int bid_ofs)
{
    // printf("libdyn module siminit function called\n");

    // Register my blocks to the given simulation

    int blockid_ofs = 60001;
    
    printf("Adding basic ld_blocks module\n", sim->private_comp_func_list);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs, LIBDYN_COMPFN_TYPE_LIBDYN, &compu_func_switch2to1);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 1, LIBDYN_COMPFN_TYPE_LIBDYN, &compu_func_demux);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 2, LIBDYN_COMPFN_TYPE_LIBDYN, &compu_func_mux);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 3, LIBDYN_COMPFN_TYPE_LIBDYN, &ortd_compu_func_hysteresis);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 4, LIBDYN_COMPFN_TYPE_LIBDYN, &ortd_compu_func_modcounter);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs + 5, LIBDYN_COMPFN_TYPE_LIBDYN, &ortd_compu_func_jumper);
    
    
    
}


//} // extern "C"
