/*
    Copyright (C) 2010, 2011, 2012, 2013  Christian Klauer

    This file is part of OpenRTDynamics, the Real-Time Dynamics Framework

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

#include "libdyn_cpp.h"






class compu_func_EDF_class {
public:
    compu_func_EDF_class(struct dynlib_block_t *block);
    void destruct();
    void io(int update_states);
    int init();
private:
   struct dynlib_block_t *block;
   
//    irpar *param;

};

compu_func_EDF_class::compu_func_EDF_class(dynlib_block_t* block)
{
    this->block = block;
}

int compu_func_EDF_class::init()
{
    // initialise your block
    
    int *Uipar;
    double *Urpar;
        
    // Get the irpar parameters Uipar, Urpar
    libdyn_AutoConfigureBlock_GetUirpar(block, &Uipar, &Urpar);

    
    // extract structured parameters
    int error = 0; 

    // get a string (not so nice by now)
    struct irpar_ivec_t str_;
    char *str;
    if ( irpar_get_ivec(&str_, Uipar, Urpar, 12) < 0 ) error = -1 ;    
    irpar_getstr(&str, str_.v, 0, str_.n);

    printf("str = %s\n", str);
    
    free(str); // do not forget to free the memory allocated by irpar_getstr
    
    // get a vector of integers (double vectors are similar, replace ivec with rvec)
    struct irpar_ivec_t vec;   
    if ( irpar_get_ivec(&vec, Uipar, Urpar, 11) < 0 ) error = -1 ;            
    printf("vec[0] = %d\n", vec.v[0]);
    


    
    // Return -1 to indicate an error, so the simulation will be destructed  
    return error;
}


void compu_func_EDF_class::io(int update_states)
{
    if (update_states==0) { // calculate outputs
//         double *in = (double *) libdyn_get_input_ptr(block,0); // the first input port

// Write to the first output port      
//         double *output = (double*) libdyn_get_output_ptr(block, 0); // the first output port
// 	*output = 1;
    }
}

void compu_func_EDF_class::destruct()
{
    // free your memory, ...
}


// This is the main C-Callback function, which forwards requests to the C++-Class above
int compu_func_EDFWrite(int flag, struct dynlib_block_t *block)
{

     printf("comp_func EDF: flag==%d\n", flag);

    double *rpar = libdyn_get_rpar_ptr(block);
    int *ipar = libdyn_get_ipar_ptr(block);



    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
//         in = (double *) libdyn_get_input_ptr(block,0);
        compu_func_EDF_class *worker = (compu_func_EDF_class *) libdyn_get_work_ptr(block);

        worker->io(0);

    }
    return 0;
    break;
    case COMPF_FLAG_UPDATESTATES:
    {
//         in = (double *) libdyn_get_input_ptr(block,0);
        compu_func_EDF_class *worker = (compu_func_EDF_class *) libdyn_get_work_ptr(block);

        worker->io(1);

    }
    return 0;
    break;
    case COMPF_FLAG_CONFIGURE:  // configure. NOTE: do not reserve memory or open devices. Do this while init instead!
    {
        int i;
	
	return libdyn_AutoConfigureBlock(block, ipar, rpar);

	
// 	// get parameters
//      struct irpar_ivec_t insizes_irp, outsizes_irp, intypes_irp, outtypes_irp, dfeed_irp;
// 
//     int error = 0; // FIXME Länge prüfen
//     if ( irpar_get_ivec(&insizes_irp, ipar, rpar, 10) < 0 ) error = 1 ;
//     if ( irpar_get_ivec(&outsizes_irp, ipar, rpar, 11) < 0 ) error = 1 ;
//     if ( irpar_get_ivec(&intypes_irp, ipar, rpar, 12) < 0 ) error = 1 ;
//     if ( irpar_get_ivec(&outtypes_irp, ipar, rpar, 13) < 0 ) error = 1 ;
//     if ( irpar_get_ivec(&dfeed_irp, ipar, rpar, 14) < 0 ) error = 1 ;
// 
//      struct irpar_ivec_t Uipar;
//      struct irpar_rvec_t Urpar;
//     
//     if ( irpar_get_ivec(&Uipar, ipar, rpar, 20) < 0 ) error = 1 ;
//     if ( irpar_get_rvec(&Urpar, ipar, rpar, 21) < 0 ) error = 1 ;
// 
// 
// 
// 	 // The number of in- and output ports
//         int Nin = insizes_irp.n;
//         int Nout = outsizes_irp.n; //ipar[1];
// 
// 	
// 	libdyn_config_block(block, BLOCKTYPE_DYNAMIC, Nout, Nin, (void *) 0, 0);
// 
// 	// configure each in and output port i
//         for (i = 0; i < Nin; ++i) {
// 	    printf("%d - \n", insizes_irp.v[i] );
//             libdyn_config_block_input(block, i, insizes_irp.v[i], intypes_irp.v[i]);
// 	}
// 
//         for (i = 0; i < Nout; ++i) {
// 	    printf("%d - df=%d \n", outsizes_irp.v[i], dfeed_irp.v[i] );
//             libdyn_config_block_output(block, i, insizes_irp.v[i], outtypes_irp.v[i], dfeed_irp.v[i] );
// 	}
// //         for (i = 0; i < Nin; ++i)
// //             libdyn_config_block_output(block, i, 1, DATATYPE_FLOAT, 1);
/*
	    
	 return error;*/

    }
    return 0;
    break;
    case COMPF_FLAG_INIT:  // init
    {
        compu_func_EDF_class *worker = new compu_func_EDF_class(block);
        libdyn_set_work_ptr(block, (void*) worker);
	
        int ret = worker->init();
        if (ret < 0)
            return -1;
    }
    return 0;
    break;
    case COMPF_FLAG_DESTUCTOR: // destroy instance
    {
        compu_func_EDF_class *worker = (compu_func_EDF_class *) libdyn_get_work_ptr(block);

        worker->destruct();
	delete worker;

    }
    return 0;
    break;
    case COMPF_FLAG_PRINTINFO:
        printf("I'm an EDF block\n");
        return 0;
        break;

    }
}

// Export to C so the libdyn simulator finds this function
extern "C" {
    // ADJUST HERE: must match to the function name in the end of this file
    int libdyn_module_EDF_siminit(struct dynlib_simulation_t *sim, int bid_ofs);
}

// CHANGE HERE: Adjust this function name to match the name of your module
int libdyn_module_EDF_siminit(struct dynlib_simulation_t *sim, int bid_ofs)
{

    // Register my blocks to the given simulation

    int blockid = 15800;  // CHANGE HERE: choose a unique id for each block
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &compu_func_EDFWrite);

    printf("module EDF initialised\n");

}


