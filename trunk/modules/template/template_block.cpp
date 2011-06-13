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

extern "C" {
#include "libdyn.h"
#include "libdyn_scicos_macros.h"

}






extern "C" {
    int libdyn_module_template_siminit(struct dynlib_simulation_t *sim, int bid_ofs);
}

class compu_func_template_class {
public:
    compu_func_template_class(struct dynlib_block_t *block);
    void destruct();
    void io(int update_states);
    int init();
private:
   struct dynlib_block_t *block;

};

compu_func_template_class::compu_func_template_class(dynlib_block_t* block)
{
    this->block = block;
}

int compu_func_template_class::init()
{
    double *rpar = libdyn_get_rpar_ptr(block);
    int *ipar = libdyn_get_ipar_ptr(block);

    double Nin = ipar[0];
    double Nout = ipar[1];



  
    return 0;
}


void compu_func_template_class::io(int update_states)
{
    if (update_states==0) {
        double *output = (double*) libdyn_get_output_ptr(block, 0);
	
	*output = 1;
    }
}

void compu_func_template_class::destruct()
{
    
}


int compu_func_template(int flag, struct dynlib_block_t *block)
{

//     printf("comp_func template: flag==%d\n", flag);

    double *in;
    double *rpar = libdyn_get_rpar_ptr(block);
    int *ipar = libdyn_get_ipar_ptr(block);

    int Nin = ipar[0];
    int Nout = ipar[1];


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        in = (double *) libdyn_get_input_ptr(block,0);
        compu_func_template_class *worker = (compu_func_template_class *) libdyn_get_work_ptr(block);

        worker->io(0);

    }
    return 0;
    break;
    case COMPF_FLAG_UPDATESTATES:
    {
        in = (double *) libdyn_get_input_ptr(block,0);
        compu_func_template_class *worker = (compu_func_template_class *) libdyn_get_work_ptr(block);

        worker->io(1);

    }
    return 0;
    break;
    case COMPF_FLAG_CONFIGURE:  // configure. NOTE: do not reserve memory or open devices. Do this while init instead!
    {
        int i;
        libdyn_config_block(block, BLOCKTYPE_STATIC, Nout, Nin, (void *) 0, 0);

        for (i = 0; i < Nin; ++i)
            libdyn_config_block_input(block, i, 1, DATATYPE_FLOAT);

        for (i = 0; i < Nin; ++i)
            libdyn_config_block_output(block, i, 1, DATATYPE_FLOAT, 1);


    }
    return 0;
    break;
    case COMPF_FLAG_INIT:  // init
    {
        compu_func_template_class *worker = new compu_func_template_class(block);
        libdyn_set_work_ptr(block, (void*) worker);

        int ret = worker->init();
        if (ret < 0)
            return -1;
    }
    return 0;
    break;
    case COMPF_FLAG_DESTUCTOR: // destroy instance
    {
        compu_func_template_class *worker = (compu_func_template_class *) libdyn_get_work_ptr(block);

        worker->destruct();

    }
    return 0;
    break;
    case COMPF_FLAG_PRINTINFO:
        printf("I'm a template block\n");
        return 0;
        break;

    }
}

//#include "block_lookup.h"

int libdyn_module_template_siminit(struct dynlib_simulation_t *sim, int bid_ofs)
{

    // Register my blocks to the given simulation

    int blockid = 990001;
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &compu_func_template);

    printf("libdyn module template initialised\n");

}


//} // extern "C"
