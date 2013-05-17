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

  This is a template for creating a new ORTD Block's computational function in C++

  Do a search of "Template" and replace it with your own name.

*/



class TemplateBlock {
public:
    TemplateBlock(struct dynlib_block_t *block) {
        this->block = block;    // no nothing more here. The real initialisation take place in init()
    }
    ~TemplateBlock()
    {
        // free your allocated memory, ...
    }

    //
    // define states or other variables
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


    inline void updateStates()
    {
        double *in1 = (double *) libdyn_get_input_ptr(block,0); // the first input port
        double *in2 = (double *) libdyn_get_input_ptr(block,1); // the 2nd input port
        double *output = (double*) libdyn_get_output_ptr(block, 0); // the first output port

        z0 = *in1 * 2;
    }


    inline void calcOutputs()
    {
        double *in1 = (double *) libdyn_get_input_ptr(block,0); // the first input port
        double *in2 = (double *) libdyn_get_input_ptr(block,1); // the 2nd input port
        double *output = (double*) libdyn_get_output_ptr(block, 0); // the first output port

        //   printf("output calc\n");
        *output = z0;
    }


    inline void resetStates()
    {
        z0 = 0; // return to the blocks initial state
    }



    void printInfo() {
        fprintf(stderr, "I'm a Template block\n");
    }


    // The Computational function that is called by the simulator
    // and that distributes the execution to the various functions
    // in this C++ - Class, including: init(), io(), resetStates() and the destructor
    static int CompFn(int flag, struct dynlib_block_t *block) {
        return LibdynCompFnTempate<TemplateBlock>( flag, block ); // this expands a template for a C-comp fn
    }

    // The data for this block managed by the simulator
    struct dynlib_block_t *block;
};




//
// Export to C so the libdyn simulator finds this function
//
extern "C" {

// CHANGE HERE: Adjust this function name to match the name of your module
    int libdyn_module_TemplateModule_V2_siminit(struct dynlib_simulation_t *sim, int bid_ofs)
    {
        //
        // Register all blocks to the given simulation "sim", bid_ofs has no meaning by now.
        // All comp. functions for all blocks are added to a list
        //

        int blockid = 999911111;  // CHANGE HERE: choose a unique id for each block

        libdyn_compfnlist_add(sim->private_comp_func_list, blockid, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &TemplateBlock::CompFn);
//     libdyn_compfnlist_add(sim->private_comp_func_list, blockid+1, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &compu_func_Template2_class::CompFn); // another block

        printf("module Template initialised\n");

    }


}
