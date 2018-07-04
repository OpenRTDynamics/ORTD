/*
    Copyright (C) 2010, 2011, 2012, 2013, 2014, 2015  Christian Klauer

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


#include <RandomLib/Random.hpp>
#include <RandomLib/NormalDistribution.hpp>
#include <RandomLib/RandomSelect.hpp>



class RandomBlock {
    // The simplest form of a block
public:
    RandomBlock(struct dynlib_block_t *block) {
        this->block = block;    // no nothing more here. The real initialisation take place in init()
    }
    ~RandomBlock()
    {
        destruct();
    }

    // variables that point to allocated memot
    irpar_ivec *Array;
    
    RandomLib::Random r;          // Create r
    RandomLib::NormalDistribution<> NormalDistribution;

    void destruct()
    {
        // free your during init() allocated memory, ...

        if (Array!=NULL) delete Array;
    }

    int init() {
        int *Uipar;
        double *Urpar;

        try {
            // Get the irpar parameters Uipar, Urpar
            libdyn_AutoConfigureBlock_GetUirpar(block, &Uipar, &Urpar);

            // init all pointers with NULL
            Array = NULL;


            // c++ version (nicer), an exception is thrown in case something goes wrong

            Array = new irpar_ivec(Uipar, Urpar, 11); // then use:  veccpp.n; veccpp.v;
            printf("veccpp[0] = %d\n", Array->v[0]); // print the first element
            // of the vector that is of size veccpp.n
	    
	      r.Reseed();                   // and give it a unique seed
//   std::cout << "Using " << r.Name() << "\n"
//             << "with seed " << r.SeedString() << "\n";

            // set the initial states
            resetStates();
        } catch(int e) { // check if initialisation went fine
            // deallocate all previously allocated memeory in case something went wrong
            fprintf(stderr, "RandomBlock: something went wrong. Exception = %d\n", e);
            destruct(); // free all memeory allocated by now.
            return -1; // indicate an error
        }

        // Return 0 to indicate that there was no error
        return 0;
    }

    inline void updateStates()
    {
    }

    inline void calcOutputs()
    {
        double *output = (double*) libdyn_get_output_ptr(block, 0); // the first output port
	
	*output = NormalDistribution(r);
// 	printf("Random: %f\n", *output );      
    }


    inline void resetStates()
    {
    }


    void printInfo() {
        fprintf(stderr, "I'm a Random block\n");
    }

    // uncommonly used flags
    void PrepareReset() {}
    void HigherLevelResetStates() {}
    void PostInit() {}


    // The Computational function that is called by the simulator
    // and that distributes the execution to the various functions
    // in this C++ - Class, including: init(), io(), resetStates() and the destructor
    static int CompFn(int flag, struct dynlib_block_t *block) {
        return LibdynCompFnTempate<RandomBlock>( flag, block ); // this expands a template for a C-comp fn
    }

    // The data for this block managed by the simulator
    struct dynlib_block_t *block;
};









//
// Export to C so the libdyn simulator finds this function
// fn = "Random" is the folder name of the module
// and the C- function called by the simulator for requesting
// blocks is then "libdyn_module_<fn>_siminit".
// If you're compiling to a module, make sure to also adapt the file
// pluginLoader.cpp
//
extern "C" {

// CHANGE HERE: Adjust this function name to match the name of your module
//              The string "Random" must be exchanged
    int libdyn_module_Random_siminit(struct dynlib_simulation_t *sim, int bid_ofs)
    {
        //
        // Register all blocks to the given simulation "sim", bid_ofs has no meaning by now.
        // All comp. functions for all blocks are added to a list
        //


        // Read the configuration file for the block ids
        int blockid =
#include "blockid_start.conf.tmp"
            ;

        // simple block
        libdyn_compfnlist_add(sim->private_comp_func_list, blockid, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &RandomBlock::CompFn);


#ifdef DEBUG
        printf("module Random is initialised\n");
#endif  

    }


}
