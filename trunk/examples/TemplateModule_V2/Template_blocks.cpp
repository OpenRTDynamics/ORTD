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
#include "irpar.h"
}

#include "libdyn_cpp.h"






/*
   A nicer C++ interface for compuational functions
*/





/*

  This is a template for creating a new ORTD Block's computational function in C++

  Do a search of "Template" and replace it with your own name.

*/


class compu_func_Template_class {
public:
    compu_func_Template_class(struct dynlib_block_t *block) {
        this->block = block;
    }  
    ~compu_func_Template_class()
    {
        printf("Destructor was called\n");
        // free your allocated memory, ...
    }
  
    //     
    // define states
    //
    
    double z0; // a state

    //     
    // initialise your block
    //
    
    int init() {
        int *Uipar;
        double *Urpar;

        // Get the irpar parameters Uipar, Urpar
        libdyn_AutoConfigureBlock_GetUirpar(block, &Uipar, &Urpar);

        //
        // extract some structured sample parameters
        //
        int error = 0;

        //
        // get a string (not so nice by now)
        //
        struct irpar_ivec_t str_;
        char *str;
        if ( irpar_get_ivec(&str_, Uipar, Urpar, 12) < 0 ) error = -1 ;
        irpar_getstr(&str, str_.v, 0, str_.n);

        printf("str = %s\n", str);

        free(str); // do not forget to free the memory allocated by irpar_getstr


        //
        // get a vector of integers (double vectors are similar, replace ivec with rvec)
        //
        struct irpar_ivec_t vec;
        if ( irpar_get_ivec(&vec, Uipar, Urpar, 11) < 0 ) error = -1 ;
        printf("vec[0] = %d\n", vec.v[0]);


        // set the initial states
        resetStates();

        // Return -1 to indicate an error, so the simulation will be destructed
        return error;
    }

    
    void updateStates()
    {
        double *in1 = (double *) libdyn_get_input_ptr(block,0); // the first input port
        double *in2 = (double *) libdyn_get_input_ptr(block,1); // the 2nd input port
        double *output = (double*) libdyn_get_output_ptr(block, 0); // the first output port

        z0 = *in1 * 2;
    }

    
    void calcOutputs()
    {
        double *in1 = (double *) libdyn_get_input_ptr(block,0); // the first input port
        double *in2 = (double *) libdyn_get_input_ptr(block,1); // the 2nd input port
        double *output = (double*) libdyn_get_output_ptr(block, 0); // the first output port

//   printf("output calc\n");
        *output = z0;
    }

    
    void resetStates()
    {
        z0 = 0; // return to the blocks initial state
    }




    void printInfo() {
        fprintf(stderr, "I'm a Template block\n");
    }

    
    
    /*
       This ugly tail is not so important to look at    
       Maybe this should be put into a macro
    */

    // The Computational function that is called by the simulator
    // and that distributes the execution to the various functions
    // in this C++ - Class, including: init(), io(), destruct()
    static int CompFn(int flag, struct dynlib_block_t *block) {
        // This is the main C-Callback function, which forwards requests to the functions of the C++-Class

        // uncomment this if you want to know when this block is called by the simulator
//           printf("comp_func Template: flag==%d\n", flag);

        // the blocks raw parameter sets for integers and doubles
        double *rpar = libdyn_get_rpar_ptr(block);
        int *ipar = libdyn_get_ipar_ptr(block);

        switch (flag) {
        case COMPF_FLAG_CALCOUTPUTS:
        {
            compu_func_Template_class *worker = (compu_func_Template_class *) libdyn_get_work_ptr(block);
            worker->calcOutputs();
        }
        return 0;
        break;
        case COMPF_FLAG_UPDATESTATES:
        {
            compu_func_Template_class *worker = (compu_func_Template_class *) libdyn_get_work_ptr(block);
            worker->updateStates();
        }
        return 0;
        break;
        case COMPF_FLAG_RESETSTATES:
        {
            compu_func_Template_class *worker = (compu_func_Template_class *) libdyn_get_work_ptr(block);
            worker->resetStates();
        }
        return 0;
        break;
        case COMPF_FLAG_CONFIGURE:  // configure. NOTE: do not reserve memory or open devices. Do this within COMPF_FLAG_INIT instead!
        {
            return libdyn_AutoConfigureBlock(block, ipar, rpar);
        }
        return 0;
        break;
        case COMPF_FLAG_INIT:  // init
        {
            compu_func_Template_class *worker = new compu_func_Template_class(block);
            libdyn_set_work_ptr(block, (void*) worker); // remember the instance of the C++ Class

            int ret = worker->init();
            if (ret < 0)
                return -1;
        }
        return 0;
        break;
        case COMPF_FLAG_DESTUCTOR: // destroy instance
        {
            compu_func_Template_class *worker = (compu_func_Template_class *) libdyn_get_work_ptr(block);
            delete worker;

        }
        return 0;
        break;
        case COMPF_FLAG_PRINTINFO:
        {
            compu_func_Template_class *worker = (compu_func_Template_class *) libdyn_get_work_ptr(block);
            worker->printInfo();
        }
        return 0;
        break;

        }

    }
    
    struct dynlib_block_t *block;
};




//
// Export to C so the libdyn simulator finds this function
// All comp. functions for all blocks are added to a list
//
extern "C" {

// CHANGE HERE: Adjust this function name to match the name of your module
int libdyn_module_Template_siminit(struct dynlib_simulation_t *sim, int bid_ofs)
{

    // Register my blocks to the given simulation

    int blockid = 999911111;  // CHANGE HERE: choose a unique id for each block
    
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &compu_func_Template_class::CompFn);
//     libdyn_compfnlist_add(sim->private_comp_func_list, blockid+1, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &compu_func_Template2_class::CompFn); // another block

    printf("module Template initialised\n");

}


}
