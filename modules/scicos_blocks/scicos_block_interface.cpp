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
#include "scicos_compfn_list.h"





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


    int insize = ipar[1];
    int outsize = ipar[2];
    
    int len_identstr = ipar[3];
    char *identstr;
    
    irpar_getstr(&identstr, ipar, 4, len_identstr);
    
    
     int (*compfn)(scicos_block * block, int flag);
     
    printf("New scicos interface using identifier %s\n", identstr);
    
    compfn = ( int (*)(scicos_block*, int) ) scicos_compfn_list_find(identstr);
    
    free(identstr);
    

    
    
   double rpar2[] = {1,-1,1,1,0};
   int ipar2[] = {1,-1,1,1,1};
   
   int Nz = 7;
   double z[] = { 0,0,0, 0,  0,0, 0,0     ,0,0,0,0,0 };
    

   /* def real parameters */
 double RPAR1[ ] = {
/* Routine name of block: cstblk4
 * Gui name of block: CONST_m
 * Compiled structure index: 1
 * Exprs: 1
 * rpar=
 */
1,

/* Routine name of block: dsslti4
 * Gui name of block: DLR
 * Compiled structure index: 2
 * Exprs: 1
 * rpar=
 */
1,1,1,0,

};

/* def integer parameters */
int IPAR1[ ] = {
/* Routine name of block: summation
 * Gui name of block: SUMMATION
 * Compiled structure index: 3
 * Exprs: [1;-1]
 * ipar= {1,-1};
 */
1,-1,

/* Routine name of block: actionneur1
 * Gui name of block: OUTPUTPORTEVTS
 * Compiled structure index: 4
 * Exprs: 1
 * ipar= {1};
 */
1,

/* Routine name of block: bidon
 * Gui name of block: EVTGEN_f
 * Compiled structure index: 5
 * Exprs: 1
 * ipar= {1};
 */
1,

/* Routine name of block: capteur1
 * Gui name of block: INPUTPORTEVTS
 * Compiled structure index: 6
 * Exprs: 1
 * ipar= {1};
 */
1,

/* Routine name of block: summation
 * Gui name of block: SUMMATION
 * Compiled structure index: 7
 * Exprs: [1;-1]
 * ipar= {1,-1};
 */
1,-1,

};


   
   
     int Nin = 1; // only one in- and outport possible
    int Nout = 1;

    cos.initStructure(compfn, sizeof(IPAR1), sizeof(RPAR1), IPAR1, RPAR1, Nin, Nout, Nz, z);
    
    int i;
    for (i = 0; i < Nin; ++i) {
      cos.setInSize(i, insize);  // make insize depending on i for multiple ports
      
      double *p = (double*) libdyn_get_input_ptr(block, i);
      cos.setInPtr(i, p);
    }
    for (i = 0; i < Nout; ++i) {
      cos.setOutSize(i, outsize);  // make outsize depending on i for multiple ports
      
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

    int Nin = 1;
    int Nout = 1;

    int insize = ipar[1];
    int outsize = ipar[2];


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
        libdyn_config_block(block, BLOCKTYPE_DYNAMIC, Nout, Nin, (void *) 0, 0);

        
            libdyn_config_block_input(block, 0, insize, DATATYPE_FLOAT);
            libdyn_config_block_output(block, 0, outsize, DATATYPE_FLOAT, 1);


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


