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

#include "scilab.h"


extern "C" {
#include "libdyn.h"
#include "libdyn_scicos_macros.h"
#include "irpar.h"
}


#include "io.h"



extern "C" {
    int libdyn_module_scilab_siminit(struct dynlib_simulation_t *sim, int bid_ofs);
}

class compu_func_scilab_class {
public:
    compu_func_scilab_class(struct dynlib_block_t *block);
    void destruct();
    void io(int update_states);
    int init();
private:
    struct dynlib_block_t *block;
    char *retstr;
    scilab_calculation *scilab_calc;

    // strings that are parameters to the block
    char *init_cmd, *calc_cmd, *destruct_cmd, *scilab_path;

};

compu_func_scilab_class::compu_func_scilab_class(dynlib_block_t* block)
{
    this->block = block;
}

extern char _binary_scilab_ORTDToolbox_sce_start[];
extern uint _binary_scilab_ORTDToolbox_sce_size;
extern char _binary_scilab_ORTDToolbox_sce_end[];

int compu_func_scilab_class::init()
{
    int *ipar = libdyn_get_ipar_ptr(block);

    int start_init_param = 11;

    int length_init_cmd = ipar[start_init_param];
    int length_calc_cmd = ipar[start_init_param + 1];
    int length_destruct_cmd = ipar[start_init_param + 2];
    int length_scilab_path = ipar[start_init_param + 3];

    int start_init_cmd = start_init_param + 4;
    int start_calc_cmd = start_init_cmd + length_init_cmd;
    int start_destruct_cmd = start_calc_cmd + length_calc_cmd;
    int start_scilab_path = start_destruct_cmd + length_destruct_cmd;



    irpar_getstr(&init_cmd, ipar, start_init_cmd, length_init_cmd);
    irpar_getstr(&calc_cmd, ipar, start_calc_cmd, length_calc_cmd);
    irpar_getstr(&destruct_cmd, ipar, start_destruct_cmd, length_destruct_cmd);
    irpar_getstr(&scilab_path, ipar, start_scilab_path, length_scilab_path);

    fprintf(stderr, "Scilab executable is %s\n", scilab_path);
    ortd_io::PutString(block->sim, "Scilab ::\n");

    scilab_calc = new scilab_calculation(block, scilab_path, init_cmd, destruct_cmd, calc_cmd);

    // Load the ORTD-Scilab Toolbox that was compiled into the ortd executables into the Scilab instance
//     scilab_calc->send_buffer( _binary_scilab_ORTDToolbox_sce_start, _binary_scilab_ORTDToolbox_sce_size );
    scilab_calc->send_buffer( _binary_scilab_ORTDToolbox_sce_start, _binary_scilab_ORTDToolbox_sce_end - _binary_scilab_ORTDToolbox_sce_start );

    scilab_calc->init(); // send init_cmd to scilab

    return 0;
}


void compu_func_scilab_class::io(int update_states)
{
    if (update_states==0) {
        double *output = (double*) libdyn_get_output_ptr(block, 0);
        double *in = (double *) libdyn_get_input_ptr(block,0);
        int *ipar = libdyn_get_ipar_ptr(block);
        int insize = ipar[0];
        int outsize = ipar[1];
        int invec_no = ipar[2];
        int outvec_no = ipar[3];
        int i;

#ifdef DEBUG
        fprintf(stderr, "scilab: Start send_vector_to_scilab\n");
#endif

	if (scilab_calc->send_vector_to_scilab(invec_no, in, insize))
        {
	  
#ifdef DEBUG
            fprintf(stderr, "scilab: Start calculate\n");
#endif

	    if (scilab_calc->calculate(invec_no, outvec_no, insize, outsize)) // send calc_cmd to scilab
            {
	      
#ifdef DEBUG
                fprintf(stderr, "scilab: Start read_vector_from_scilab\n");
#endif

		scilab_calc->read_vector_from_scilab(outvec_no, output, outsize);
            }
        }

#ifdef DEBUG
        fprintf(stderr, "scilab: computation finished\n");
#endif

    }
}

void compu_func_scilab_class::destruct()
{
    delete scilab_calc; // destruct_cmd and "exit" to scilab

    free(init_cmd);
    free(calc_cmd);
    free(destruct_cmd);
    free(scilab_path);
}


int compu_func_scilab(int flag, struct dynlib_block_t *block)
{

    //printf("comp_func scilab: flag==%d\n", flag);

    int *ipar = libdyn_get_ipar_ptr(block);

    int insize = ipar[0];
    int outsize = ipar[1];

    int Nin = 1;
    int Nout = 1;


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {

        compu_func_scilab_class *worker = (compu_func_scilab_class *) libdyn_get_work_ptr(block);

        worker->io(0);


    }
    return 0;
    break;
    case COMPF_FLAG_UPDATESTATES:
    {
        compu_func_scilab_class *worker = (compu_func_scilab_class *) libdyn_get_work_ptr(block);


        worker->io(1);

    }
    return 0;
    break;
    case COMPF_FLAG_CONFIGURE:  // configure. NOTE: do not reserve memory or open devices. Do this while init instead!
    {
        int i;

        if ((insize < 1) || (outsize < 1)) {
            printf("size cannot be smaller than 1\n");
            printf("insize = %d\n", insize);
            printf("outsize = %d\n", outsize);

            return -1;
        }



        libdyn_config_block(block, BLOCKTYPE_STATIC, Nout, Nin, (void *) 0, 0);

        for (i = 0; i < Nin; ++i)
            libdyn_config_block_input(block, i, insize, DATATYPE_FLOAT);

        for (i = 0; i < Nout; ++i)
            libdyn_config_block_output(block, i, outsize, DATATYPE_FLOAT, 1);


    }
    return 0;
    break;
    case COMPF_FLAG_INIT:  // init
    {
        compu_func_scilab_class *worker = new compu_func_scilab_class(block);
        libdyn_set_work_ptr(block, (void*) worker);

        int ret = worker->init();

        if (ret < 0)
            return -1;
    }
    return 0;
    break;
    case COMPF_FLAG_DESTUCTOR: // destroy instance
    {
        compu_func_scilab_class *worker = (compu_func_scilab_class *) libdyn_get_work_ptr(block);

        worker->destruct();
        delete worker;

    }
    return 0;
    break;
    case COMPF_FLAG_PRINTINFO:
        printf("I'm a scilab block\n");
        return 0;
        break;

    }
}

//#include "block_lookup.h"

int libdyn_module_scilab_siminit(struct dynlib_simulation_t *sim, int bid_ofs)
{

    // Register my blocks to the given simulation

    int blockid = 22000;
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &compu_func_scilab);

#ifdef DEBUG
    printf("libdyn module scilab initialised\n");
#endif
}


//} // extern "C"
