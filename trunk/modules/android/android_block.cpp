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
#include <stdio.h>

extern "C" {
#include "libdyn.h"
#include "libdyn_scicos_macros.h"

}








// class compu_func_android_class {
// public:
//     compu_func_android_class(struct dynlib_block_t *block);
//     void destruct();
//     void io(int update_states);
//     int init();
// private:
//    struct dynlib_block_t *block;
// 
// };
// 
// compu_func_android_class::compu_func_android_class(dynlib_block_t* block)
// {
//     this->block = block;
// }
// 
// int compu_func_android_class::init()
// {
//     double *rpar = libdyn_get_rpar_ptr(block);
//     int *ipar = libdyn_get_ipar_ptr(block);
// 
//     int Nin = ipar[0];
//     int Nout = ipar[1];
// 
// 
//     // Return -1 to indicate an error, so the simulation will be destructed
//   
//     return 0;
// }
// 
// 
// void compu_func_android_class::io(int update_states)
// {
//     if (update_states==0) {
//         double *output = (double*) libdyn_get_output_ptr(block, 0);
// 	
// 	*output = 1;
//     }
// }
// 
// void compu_func_android_class::destruct()
// {
//     
// }
// 
// 
// // This is the main C-Callback function, which forwards requests to the C++-Class above
// int compu_func_android(int flag, struct dynlib_block_t *block)
// {
// 
// //     printf("comp_func android: flag==%d\n", flag);
// 
//     double *in;
//     double *rpar = libdyn_get_rpar_ptr(block);
//     int *ipar = libdyn_get_ipar_ptr(block);
// 
//     // The number of in- and output ports
//     int Nin = ipar[0];
//     int Nout = ipar[1];
// 
// 
//     switch (flag) {
//     case COMPF_FLAG_CALCOUTPUTS:
//     {
//         in = (double *) libdyn_get_input_ptr(block,0);
//         compu_func_android_class *worker = (compu_func_android_class *) libdyn_get_work_ptr(block);
// 
//         worker->io(0);
// 
//     }
//     return 0;
//     break;
//     case COMPF_FLAG_UPDATESTATES:
//     {
//         in = (double *) libdyn_get_input_ptr(block,0);
//         compu_func_android_class *worker = (compu_func_android_class *) libdyn_get_work_ptr(block);
// 
//         worker->io(1);
// 
//     }
//     return 0;
//     break;
//     case COMPF_FLAG_CONFIGURE:  // configure. NOTE: do not reserve memory or open devices. Do this while init instead!
//     {
//         int i;
//         libdyn_config_block(block, BLOCKTYPE_STATIC, Nout, Nin, (void *) 0, 0);
// 
// 	// configure each in and output port i
//         for (i = 0; i < Nin; ++i)
//             libdyn_config_block_input(block, i, 1, DATATYPE_FLOAT);
// 
//         for (i = 0; i < Nin; ++i)
//             libdyn_config_block_output(block, i, 1, DATATYPE_FLOAT, 1);
// 
// 
//     }
//     return 0;
//     break;
//     case COMPF_FLAG_INIT:  // init
//     {
//         compu_func_android_class *worker = new compu_func_android_class(block);
//         libdyn_set_work_ptr(block, (void*) worker);
// 
//         int ret = worker->init();
//         if (ret < 0)
//             return -1;
//     }
//     return 0;
//     break;
//     case COMPF_FLAG_DESTUCTOR: // destroy instance
//     {
//         compu_func_android_class *worker = (compu_func_android_class *) libdyn_get_work_ptr(block);
// 
//         worker->destruct();
// 	delete worker;
// 
//     }
//     return 0;
//     break;
//     case COMPF_FLAG_PRINTINFO:
//         printf("I'm a android block\n");
//         return 0;
//         break;
// 
//     }
// }

// Export to C so the libdyn simulator finds this function
extern "C" {
    // ADJUST HERE: must match to the function name in the end of this file
    int libdyn_module_android_siminit(struct dynlib_simulation_t *sim, int bid_ofs);

//// Compile only on Android /////
  #ifdef __ORTD_TARGET_ANDROID
//////////////////////////////////

    
    int compu_func_AndroidSensor(int flag, struct dynlib_block_t *block);

//// Compile only on Android /////
  #endif
//////////////////////////////////
    
}

// CHANGE HERE: Adjust this function name to match the name of your module
int libdyn_module_android_siminit(struct dynlib_simulation_t *sim, int bid_ofs)
{
//// Compile only on Android /////
  #ifdef __ORTD_TARGET_ANDROID
//////////////////////////////////

    // Register my blocks to the given simulation

    int blockid = 15500;  // CHANGE HERE: choose a unique id for each block
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid+0, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &compu_func_AndroidSensor);

    
    printf("libdyn module android initialised\n");
    
//// Compile only on Android /////
  #endif
//////////////////////////////////


}


