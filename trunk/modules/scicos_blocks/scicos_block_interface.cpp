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

#define DEBUG




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


//     int insize = ipar[1];
//     int outsize = ipar[2];
    
        

int error = 0;
    
    struct irpar_ivec_t insizes, outsizes, intypes, outtypes, param;

        if ( irpar_get_ivec(&insizes, ipar, rpar, 10) < 0 ) error = 1 ;
        if ( irpar_get_ivec(&outsizes, ipar, rpar, 11) < 0 ) error = 1 ;
        if ( irpar_get_ivec(&intypes, ipar, rpar, 12) < 0 ) error = 1 ;
        if ( irpar_get_ivec(&outtypes, ipar, rpar, 13) < 0 ) error = 1 ;
        if ( irpar_get_ivec(&param, ipar, rpar, 18) <  0 ) error = 1 ;
        
        int dfeed = param.v[1];


        // N io ports
        int Ndatainports = insizes.n; 
        int Ndataoutports = outsizes.n;

    struct irpar_ivec_t identstr__, block_ipar;
    struct irpar_rvec_t block_rpar, dstate;

    
    if ( irpar_get_ivec(&identstr__, ipar, rpar, 20) < 0 ) error = 1 ;
    if ( irpar_get_ivec(&block_ipar, ipar, rpar, 21) < 0 ) error = 1 ;
    if ( irpar_get_rvec(&block_rpar, ipar, rpar, 22) < 0 ) error = 1 ;
    if ( irpar_get_rvec(&dstate, ipar, rpar, 23) < 0 ) error = 1 ;
    
    
    
    if (error == 1) {
        printf("scicos_block: could not get all parameters from irpar set\n");
        return -1;
    }
    
    
//     int len_identstr = identstr__.n;
    char *identstr;        
    irpar_getstr(&identstr, identstr__.v, 0, identstr__.n);    
    
    printf("New scicos interface using identifier %s\n", identstr);
    
    // The compfn
     int (*compfn)(scicos_block * block, int flag);
     compfn = ( int (*)(scicos_block*, int) ) scicos_compfn_list_find(identstr);
    
    free(identstr);
    
    if (compfn == NULL) {
      fprintf(stderr, "Can not find a Scicos computational function\n");
      return -1;
    }
    

   double *rpar2 = block_rpar.v;   int n_rpar2 = block_rpar.n;
   int *ipar2 = block_ipar.v;      int n_ipar2 = block_ipar.n;   
   double *z = dstate.v;           int Nz = dstate.n;

#ifdef DEBUG
   printf("rpar ptr %p\n", (void*) rpar2);
   printf("ipar ptr %p\n", (void*) ipar2);
   int lpc;
   for( lpc=0; lpc < n_rpar2; ++lpc) {
     printf("rpar %d\n", rpar2[lpc]); 
   }
   for( lpc=0; lpc < n_ipar2; ++lpc) {
     printf("ipar %d\n", ipar2[lpc]); 
   }
#endif
   
   
   // Set.up IO of the scicos block
     int Nin = Ndatainports; // only one in- and outport possible
    int Nout = Ndataoutports;

//     cos.initStructure(compfn, sizeof(IPAR1), sizeof(RPAR1), IPAR1, RPAR1, Nin, Nout, Nz, z);
    cos.initStructure(compfn, sizeof(n_ipar2), sizeof(n_rpar2), ipar2, rpar2, Nin, Nout, Nz, z);
    
//     printf("rpar ptr %p\n", (void*) cos.cosblock.rpar);
    
    int i;
    for (i = 0; i < Nin; ++i) {
      cos.setInSize(i, insizes.v[i]);  // make insize depending on i for multiple ports
      
      double *p = (double*) libdyn_get_input_ptr(block, i);
      cos.setInPtr(i, p);
      
#ifdef DEBUG
       printf("insize port %d is %d\n", i, insizes.v[i]);
#endif;
    }
    for (i = 0; i < Nout; ++i) {
      cos.setOutSize(i, outsizes.v[i]);  // make outsize depending on i for multiple ports
      
      double *p = (double*) libdyn_get_output_ptr(block, i);
      cos.setOutPtr(i, p);
      
#ifdef DEBUG
       printf("outsize port %d is %d\n", i, outsizes.v[i]);
#endif;
    }
    
//     printf("rpar ptr %p\n", (void*) cos.cosblock.rpar);
// printf("block ptr %p\n", (void*) &cos.cosblock);
//     printf("cos ptr %p\n", (void*) &cos);
    
    cos.Cinit();
#ifdef DEBUG
       printf("Block init was called.\n");
#endif;

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

#ifdef DEBUG
     printf("comp_func template: flag==%d\n", flag);
#endif;

    double *in;
    double *rpar = libdyn_get_rpar_ptr(block);
    int *ipar = libdyn_get_ipar_ptr(block);

    int Nin = 1;
    int Nout = 1;

//     int insize = ipar[1];
//     int outsize = ipar[2];


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        in = (double *) libdyn_get_input_ptr(block,0);
        compu_func_ScicosBlockWrapper_class *worker = (compu_func_ScicosBlockWrapper_class *) libdyn_get_work_ptr(block);

        worker->io(0);

#ifdef DEBUG
       printf("calclout OK\n", flag);
#endif;

    }
    return 0;
    break;
    case COMPF_FLAG_UPDATESTATES:
    {
        in = (double *) libdyn_get_input_ptr(block,0);
        compu_func_ScicosBlockWrapper_class *worker = (compu_func_ScicosBlockWrapper_class *) libdyn_get_work_ptr(block);

        worker->io(1);

#ifdef DEBUG
       printf("sup OK\n", flag);
#endif;

    }
    return 0;
    break;
    case COMPF_FLAG_CONFIGURE:  // configure. NOTE: do not reserve memory or open devices. Do this while init instead!
    {
//         int i;
//         libdyn_config_block(block, BLOCKTYPE_DYNAMIC, Nout, Nin, (void *) 0, 0);
// 
//         
//             libdyn_config_block_input(block, 0, insize, DATATYPE_FLOAT);
//             libdyn_config_block_output(block, 0, outsize, DATATYPE_FLOAT, 1);

	    
	    
	    
// 	      parlist = new_irparam_elemet_ivec(parlist, cosblk.in, 10);
//   parlist = new_irparam_elemet_ivec(parlist, cosblk.out, 11);
//   parlist = new_irparam_elemet_ivec(parlist, ORTD.DATATYPE_FLOAT*ones(cosblk.in) , 12); // only float is supported by now
//   parlist = new_irparam_elemet_ivec(parlist, ORTD.DATATYPE_FLOAT*ones(cosblk.out) , 13); // only float is supported by now
//   parlist = new_irparam_elemet_ivec(parlist, parameters, 18);

	    
// 	    printf("ipar = %d %d %d %d %d\n", ipar[0], ipar[1], ipar[2], ipar[3], ipar[4]);
	    

	    int error = 0;
        struct irpar_ivec_t insizes, outsizes, intypes, outtypes, param;
        if ( irpar_get_ivec(&insizes, ipar, rpar, 10) < 0 ) error = 1 ;
        if ( irpar_get_ivec(&outsizes, ipar, rpar, 11) < 0 ) error = 1 ;
        if ( irpar_get_ivec(&intypes, ipar, rpar, 12) < 0 ) error = 1 ;
        if ( irpar_get_ivec(&outtypes, ipar, rpar, 13) < 0 ) error = 1 ;
        if ( irpar_get_ivec(&param, ipar, rpar, 18) <  0 ) error = 1 ;
    
    if (error == 1) {
        printf("scicos_block: could not get all parameters from irpar set\n");
        return -1;
    }

        int dfeed = param.v[1];

        // IO of the OUTER block
        int Ndatainports = insizes.n; 
        int Ndataoutports = outsizes.n;


#ifdef DEBUG
         printf("ScicosBlock; Ndatainports = %d, Ndataoutports = %d\n", Ndatainports, Ndataoutports);
#endif;

        int i;
        libdyn_config_block(block, BLOCKTYPE_DYNAMIC, Ndataoutports, Ndatainports, (void *) 0, 0);

        for (i = 0; i < Ndatainports; ++i) {
            libdyn_config_block_input(block, i, insizes.v[i], intypes.v[i]);
#ifdef DEBUG
           printf("insize port %d is %d\n", i, insizes.v[i]);
#endif;
        }

        for (i = 0; i < Ndataoutports; ++i) {
            libdyn_config_block_output(block, i, outsizes.v[i], outtypes.v[i], dfeed);
#ifdef DEBUG
           printf("outsize port %d is %d\n", i, insizes.v[i]);
#endif;
        }


	    
	    
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
	
#ifdef DEBUG
       printf("init OK\n", flag);
#endif;
	
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

#ifdef DEBUG
    printf("libdyn module scicos_blocks initialised\n");
#endif  
    

}


