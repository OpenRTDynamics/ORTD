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
#include "edflib.h"





class compu_func_EDF_class {
public:
    compu_func_EDF_class(struct dynlib_block_t *block);
    void destruct();
    void io(int update_states);
    int init();
private:
    struct dynlib_block_t *block;

    int EDFhdl;
    int chns, SMP_FREQ;

    double **buffers;
    int BufferCounter, bufferSize;

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
    int i;

    // Get the irpar parameters Uipar, Urpar
    libdyn_AutoConfigureBlock_GetUirpar(block, &Uipar, &Urpar);


    // extract structured parameters
    int error = 0;



    // get a vector of integers (double vectors are similar, replace ivec with rvec)
    struct irpar_ivec_t vec;
    if ( irpar_get_ivec(&vec, Uipar, Urpar, 11) < 0 ) error = -1 ;

    chns = vec.v[0];
    SMP_FREQ = vec.v[1];

    printf("chns = %d, SMP_FREQ = %d\n", chns, SMP_FREQ);



    // open EDF+
    struct irpar_ivec_t fname_;
    char *fname;
    if ( irpar_get_ivec(&fname_, Uipar, Urpar, 12) < 0 ) error = -1 ;
    irpar_getstr(&fname, fname_.v, 0, fname_.n);
    printf("fname = %s\n", fname);

    EDFhdl = edfopen_file_writeonly(fname, EDFLIB_FILETYPE_BDFPLUS, chns);
    free(fname);
    if(EDFhdl<0)    {
        fprintf(stderr, "error: edfopen_file_writeonly()\n");
        goto error;
    }

    // set sampling freq.
    for(i=0; i<chns; i++)
    {
        if(edf_set_samplefrequency(EDFhdl, i, SMP_FREQ))      {
            printf("error: edf_set_samplefrequency()\n");
            goto error_EDF;
        }
    }

    // Apply PhyMax
    {
        struct irpar_rvec_t PhyMax_;
        if ( irpar_get_rvec(&PhyMax_, Uipar, Urpar, 15) < 0 ) error = -1 ;

        for(i=0; i<chns; i++)  {
            if(edf_set_physical_maximum(EDFhdl, i, PhyMax_.v[i] ))    {
                printf("error: edf_set_physical_maximum()\n");
                goto error_EDF;
            }
        }
    }

    // Apply PhyMin
    {
        struct irpar_rvec_t PhyMin_;
        if ( irpar_get_rvec(&PhyMin_, Uipar, Urpar, 16) < 0 ) error = -1 ;

        for(i=0; i<chns; i++)  {
            if(edf_set_physical_minimum(EDFhdl, i, PhyMin_.v[i] ))    {
                printf("error: edf_set_physical_minimum()\n");
                goto error_EDF;
            }
        }
    }

    // Apply DigMax
    {
        struct irpar_ivec_t DigMax_;
        if ( irpar_get_ivec(&DigMax_, Uipar, Urpar, 13) < 0 ) error = -1 ;

        for(i=0; i<chns; i++)  {
            if(edf_set_digital_maximum(EDFhdl, i, DigMax_.v[i] ))    {
                printf("error: edf_set_digital_maximum()\n");
                goto error_EDF;
            }
        }
    }

    // Apply DigMin
    {
        struct irpar_ivec_t DigMin_;
        if ( irpar_get_ivec(&DigMin_, Uipar, Urpar, 14) < 0 ) error = -1 ;

        for(i=0; i<chns; i++)  {
            if(edf_set_digital_minimum(EDFhdl, i, DigMin_.v[i] ))    {
                printf("error: edf_set_digital_minimum()\n");
                goto error_EDF;
            }
        }
    }

    // Set labels
    {
        for(i=0; i<chns; i++)  {
            struct irpar_ivec_t str_;
            char *str;
            if ( irpar_get_ivec(&str_, Uipar, Urpar, 100+i ) < 0 ) error = -1 ;
            irpar_getstr(&str, str_.v, 0, str_.n);
            printf("label[%d] = %s\n", i, str);


            if(edf_set_label(EDFhdl, i, str))   {
                printf("error: edf_set_label()\n");
		free(str);

                goto error_EDF;
            }
            free(str);
        }
    }

    // allocate buffers
    {
        bufferSize = SMP_FREQ;
        BufferCounter = 0;
        buffers = (double**) malloc(sizeof(double*) * chns );
        for(i=0; i<chns; i++)  {
            buffers[i] = (double*) malloc( sizeof(double)*bufferSize );
        }
    }
    
    return 0;

    
    
error_EDF:
    fprintf(stderr, "Error while initialising EDF\n");
    // close EDF
    edfclose_file(EDFhdl);
    return -1;

error:
    fprintf(stderr, "Error before initialising EDF\n");

    // Return -1 to indicate an error, so the simulation will be destructed
    return error;
}


void compu_func_EDF_class::io(int update_states)
{
    if (update_states==1) { // calculate outputs
// Write to the first output port
//         double *output = (double*) libdyn_get_output_ptr(block, 0); // the first output port
// 	*output = 1;

        int i;

// Buffer the samples
        for(i=0; i<chns; i++)  {
            double *ChannelBuffer = buffers[i];

            double *in = (double *) libdyn_get_input_ptr(block,i);

            ChannelBuffer[BufferCounter] = *in; // copy sample into the buffer


        }

        // increase BufferCounter
        BufferCounter++;

        // Write the buffered data every bufferSize'th sample
        if (BufferCounter == bufferSize) {
            BufferCounter = 0; // reset

            // write buffered data for all channels
            for(i=0; i<chns; i++)  {
                double *ChannelBuffer = buffers[i];

	      if(edfwrite_physical_samples(EDFhdl, ChannelBuffer))    {
		printf("error: edfwrite_physical_samples()\n");      
	      }
            }

        }




    }
}

void compu_func_EDF_class::destruct()
{
    // free your memory, ...
    edfclose_file(EDFhdl);

    // deallocate buffers
    {


        int i;
        for(i=0; i<chns; i++)  {
            free(buffers[i]);
        }
        free(buffers);


    }

}


// This is the main C-Callback function, which forwards requests to the C++-Class above
int compu_func_EDFWrite(int flag, struct dynlib_block_t *block)
{

//     printf("comp_func EDF: flag==%d\n", flag);

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


