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


//#include "block_lookup.h"

int libdyn_module_basic_ldblocks_siminit(struct dynlib_simulation_t *sim, int bid_ofs)
{
  // printf("libdyn module siminit function called\n"); 
  
  // Register my blocks to the given simulation
  
  int blockid = 60001;
  libdyn_compfnlist_add(sim->private_comp_func_list, blockid, LIBDYN_COMPFN_TYPE_LIBDYN, &compu_func_switch2to1);
}


//} // extern "C"
