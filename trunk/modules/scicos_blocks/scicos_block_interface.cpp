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
#include "irpar.h"

}

#include "ScicosWrapper.h"






class compu_func_ScicosBlockWrapper_class {
public:
    compu_func_ScicosBlockWrapper_class(struct dynlib_block_t *block);
    void destruct();
    void io(int update_states);
    int init();
private:
   struct dynlib_block_t *block;
   
   ScicosWrapper cos;
};

compu_func_ScicosBlockWrapper_class::compu_func_ScicosBlockWrapper_class(dynlib_block_t* block)
{
    this->block = block;
}

int compu_func_ScicosBlockWrapper_class::init()
{
    double *rpar = libdyn_get_rpar_ptr(block);
    int *ipar = libdyn_get_ipar_ptr(block);

    int Nin = ipar[1];
    int Nout = ipar[2];
    
    int len_identstr = ipar[3];
    char *identstr;
    
    irpar_getstr(&identstr, ipar, 4, len_identstr);
    
     int (*compfn)(scicos_block * block, int flag);
     
    printf("New scicos interface using identifier %s\n", identstr);
    
    free(identstr);
    

    
    
   double rpar2[] = {1,-1,1,1,0};
   int ipar2[] = {1,-1,1,1,1};
   
   int Nz = 7;
   double z[] = { 0,0,0, 0,0,0, 0 };
    
    
    cos.initStructure(compfn, sizeof(rpar2), sizeof(rpar2), ipar2, rpar2, Nin, Nout, Nz, z);
    
    int i;
    for (i = 0; i < Nin; ++i) {
      cos.setInSize(i, 1);
      
      double *p = (double*) libdyn_get_input_ptr(block, i);
      cos.setInPtr(i, p);
    }
    for (i = 0; i < Nout; ++i) {
      cos.setOutSize(i, 1);
      
      double *p = (double*) libdyn_get_output_ptr(block, i);
      cos.setOutPtr(i, p);
    }
    

    cos.Cinit();

    // Return -1 to indicate an error, so the simulation will be destructed
  
    return 0;
}


void compu_func_ScicosBlockWrapper_class::io(int update_states)
{
    if (update_states==0) {
        double *output = (double*) libdyn_get_output_ptr(block, 0);
	
	//*output = 1;
	
	cos.CCalcOutputs();
	
    } else {
      cos.CStateUpd(); 
    }
}

void compu_func_ScicosBlockWrapper_class::destruct()
{
  cos.Cdestruct();
  
  cos.freeStructure();
}


// This is the main C-Callback function, which forwards requests to the C++-Class above
int compu_func_scicosinterface(int flag, struct dynlib_block_t *block)
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
        compu_func_ScicosBlockWrapper_class *worker = (compu_func_ScicosBlockWrapper_class *) libdyn_get_work_ptr(block);

        worker->io(0);

    }
    return 0;
    break;
    case COMPF_FLAG_UPDATESTATES:
    {
        in = (double *) libdyn_get_input_ptr(block,0);
        compu_func_ScicosBlockWrapper_class *worker = (compu_func_ScicosBlockWrapper_class *) libdyn_get_work_ptr(block);

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
        compu_func_ScicosBlockWrapper_class *worker = new compu_func_ScicosBlockWrapper_class(block);
        libdyn_set_work_ptr(block, (void*) worker);

        int ret = worker->init();
        if (ret < 0)
            return -1;
    }
    return 0;
    break;
    case COMPF_FLAG_DESTUCTOR: // destroy instance
    {
        compu_func_ScicosBlockWrapper_class *worker = (compu_func_ScicosBlockWrapper_class *) libdyn_get_work_ptr(block);

        worker->destruct();
	delete worker;

    }
    return 0;
    break;
    case COMPF_FLAG_PRINTINFO:
        printf("I'm a ScicosBlockWrapper block\n");
        return 0;
        break;

    }
}

// Export to C so the libdyn simulator finds this function
extern "C" {
    // ADJUST HERE: must match to the function name in the end of this file
    int libdyn_module_scicos_blocks_siminit(struct dynlib_simulation_t *sim, int bid_ofs);
}

// CHANGE HERE: Adjust this function name to match the name of your module
int libdyn_module_scicos_blocks_siminit(struct dynlib_simulation_t *sim, int bid_ofs)
{

    // Register my blocks to the given simulation

    int blockid = 15200 + 0;  // CHANGE HERE: choose a unique id for each block
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &compu_func_scicosinterface);

    printf("libdyn module scicos_blocks initialised\n");

}


