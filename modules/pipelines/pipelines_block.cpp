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







// External block comp functions
extern "C" {
    // ADJUST HERE: must match to the function name in the end of this file
    int libdyn_module_pipelines_siminit(struct dynlib_simulation_t *sim, int bid_ofs);
    
    // RingBuffer.cpp
    extern int RingBuffer_block(int flag, struct dynlib_block_t *block);
    extern int write_RingBuffer_block(int flag, struct dynlib_block_t *block);    
    extern int read_RingBuffer_block(int flag, struct dynlib_block_t *block);
}

// CHANGE HERE: Adjust this function name to match the name of your module
int libdyn_module_pipelines_siminit(struct dynlib_simulation_t *sim, int bid_ofs)
{

    // Register my blocks to the given simulation

    int blockid = 15300;  // CHANGE HERE: choose a unique id for each block
//     libdyn_compfnlist_add(sim->private_comp_func_list, blockid, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &compu_func_pipeline);

    libdyn_compfnlist_add(sim->private_comp_func_list, blockid+0, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &RingBuffer_block);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid+1, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &write_RingBuffer_block);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid+2, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &read_RingBuffer_block);

    printf("libdyn module pipelines initialised\n");

}


