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




//
// Export to C so the libdyn simulator finds this function
// fn = "TemplateModule_V2" is the folder name of the module
// and the C- function called by the simulator for requesting
// blocks is then "libdyn_module_<fn>_siminit".
// If you're compiling to a module, make sure to also adapt the file
// pluginLoader.cpp
//
extern "C" {

// CHANGE HERE: Adjust this function name to match the name of your module
    int libdyn_module_Automation_siminit(struct dynlib_simulation_t *sim, int bid_ofs)
    {
        //
        // Register all blocks to the given simulation "sim", bid_ofs has no meaning by now.
        // All comp. functions for all blocks are added to a list
        //

//         int blockid = 999911111;  // CHANGE HERE: choose a unique id for each block FIXME: Need a list of free id's
// 
//         libdyn_compfnlist_add(sim->private_comp_func_list, blockid, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &TemplateBlock::CompFn);
	
	
//         printf("module Template is initialised\n");

    }


}
