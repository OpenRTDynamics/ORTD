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

#include <malloc.h>

#include "libdyn_cpp.h"

extern "C" {
// #include "libdyn.h"
#include "libdyn_scicos_macros.h"

}

// #include "parameter_manager.h"





extern "C" {
    int libdyn_module_rt_server_siminit(struct dynlib_simulation_t *sim, int bid_ofs);
}

class compu_func_rt_server_param_class {
public:
    compu_func_rt_server_param_class(struct dynlib_block_t *block);
    void destruct();
    void io(int update_states);
    int init();
private:
   struct dynlib_block_t *block;

   char *str;
   
   libdyn_master *master;
   parameter * par;
   
   bool deactivated;
};

compu_func_rt_server_param_class::compu_func_rt_server_param_class(dynlib_block_t* block)
{
    this->block = block;
}

int compu_func_rt_server_param_class::init()
{
    deactivated = true;
  
    double *rpar = libdyn_get_rpar_ptr(block);
    int *ipar = libdyn_get_ipar_ptr(block);

    

    master = (libdyn_master *) block->sim->master;
    if (master == NULL) {
      fprintf(stderr, "WARNING: libdyn: parameter block requires a libdyn master\n");
      return 0;
    }
    
    if (master->pmgr == NULL) {
      fprintf(stderr, "WARNING: libdyn: parameter block requires a set-up parameter manager\nTurn on rmeote control\n");
      return 0;
    }

    deactivated = false;

    int outsize = ipar[1];
//     int parlen = ipar[2];
    double *initial_par = &rpar[0];
    
    int exprlen = ipar[2];
    int *codedexpr = &ipar[3];

    str = (char *) malloc(exprlen+1);

    // Decode filename
    int i;
    for (i = 0; i < exprlen; ++i)
        str[i] = codedexpr[i];

    str[i] = 0; // String termination

    printf("New parameter named %s\n", str);

    par = master->pmgr->new_parameter(str, DATATYPE_FLOAT , outsize);
    
    // copy initial parameters
    par->atomic_buffer_copy_e2d(initial_par);
  
    return 0;
}


void compu_func_rt_server_param_class::io(int update_states)
{
  double *rpar = libdyn_get_rpar_ptr(block);
  int *ipar = libdyn_get_ipar_ptr(block);
  
  double *output = (double*) libdyn_get_output_ptr(block, 0);

  if (!deactivated) {
    if (update_states==0) {
	
	par->atomic_buffer_copy( (void*) output ); // FIXME Maybe do this only when the parameter was actually updated
// 	printf("parameter output = %f\n", output[0]);
    }
  } else {
    double *initial_par = &rpar[0];
    int outsize = ipar[1];
    
    memcpy((void*) output, initial_par, sizeof(double) * outsize );
  }
}

void compu_func_rt_server_param_class::destruct()
{
  if (!deactivated) {
    par->destruct();
    delete par;
  
    free(str);
  }
}


int compu_func_rt_server_param(int flag, struct dynlib_block_t *block)
{

//      printf("comp_func rt_server_param: flag==%d\n", flag);

    double *rpar = libdyn_get_rpar_ptr(block);
    int *ipar = libdyn_get_ipar_ptr(block);

    int block_version = ipar[0];
    int outsize = ipar[1];


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        compu_func_rt_server_param_class *worker = (compu_func_rt_server_param_class *) libdyn_get_work_ptr(block);

        worker->io(0);

    }
    return 0;
    break;
    case COMPF_FLAG_UPDATESTATES:
    {
        compu_func_rt_server_param_class *worker = (compu_func_rt_server_param_class *) libdyn_get_work_ptr(block);

        worker->io(1);

    }
    return 0;
    break;
    case COMPF_FLAG_CONFIGURE:  // configure
    {
        int i;
        libdyn_config_block(block, BLOCKTYPE_DYNAMIC, 1, 0, (void *) 0, 0);
        libdyn_config_block_output(block, 0, outsize, DATATYPE_FLOAT, 0);



    }
    return 0;
    break;
    case COMPF_FLAG_INIT:  // init
    {
        compu_func_rt_server_param_class *worker = new compu_func_rt_server_param_class(block);
        libdyn_set_work_ptr(block, (void*) worker);

        int ret = worker->init();
        if (ret < 0)
            return -1;
    }
    return 0;
    break;
    case COMPF_FLAG_DESTUCTOR: // destroy instance
    {
        compu_func_rt_server_param_class *worker = (compu_func_rt_server_param_class *) libdyn_get_work_ptr(block);

        worker->destruct();

    }
    return 0;
    break;
    case COMPF_FLAG_PRINTINFO:
        printf("I'm a parameter block\n");
        return 0;
        break;

    }
}

//#include "block_lookup.h"

int libdyn_module_rt_server_siminit(struct dynlib_simulation_t *sim, int bid_ofs)
{

    // Register my blocks to the given simulation

    int blockid = 14001;
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &compu_func_rt_server_param);

    printf("libdyn module rt_server initialised\n");

}


//} // extern "C"
