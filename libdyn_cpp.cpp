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


// Wrapper around libdyn.c 's simulation setup code

#include "libdyn_cpp.h"
#include <stdlib.h>
#include <stdio.h>


#define idiots_check

void libdyn::event_trigger_mask(int mask)
{
  libdyn_event_trigger_mask(sim, mask);
}

void libdyn::simulation_step(int update_states)
{

  #ifdef idiots_check
  
  if (error == -1) {
    fprintf(stderr, "libdyn_cpp: You had an error while setting up your simulation\n");
    return;
  } 
  
  #endif
  
  libdyn_simulation_step(sim, update_states);    
}

int libdyn::irpar_setup(int* ipar, double* rpar, int boxid)
{
  error = libdyn_irpar_setup(ipar, rpar, boxid, &sim, &iocfg);
  if (error < 0)
    return error;
  
  error = libdyn_simulation_init(sim); // Call INIT FLag of all blocks
  if (error < 0)
    return error;
}


bool libdyn::cfg_inptr(int in, double* inptr)
{
  if (in > iocfg.inports) {
    fprintf(stderr, "libdyn::cfg_inptr: in out of range\n");
    
    return false;
  }
  
  // set input pointer 
  iocfg.inptr[in] = inptr;

  return true;
}

double libdyn::get_skalar_out(int out)
{
  if (out > iocfg.outports) {
    fprintf(stderr, "libdyn::get_skalar_out: out out of range\n");
    
    return false;
  }
  
  return iocfg.outptr[out][0];
}

double * libdyn::get_vec_out(int out)
{
  if (out > iocfg.outports) {
    fprintf(stderr, "libdyn::get_vec_out: out out of range\n");
    
    return false;
  }
  
  return iocfg.outptr[out];
}

libdyn::libdyn(int Nin, int* insizes_, int Nout, int* outsizes_)
{
  error = 0;
  
  int sum, tmp, i;
  double *p;

  iocfg.inports = Nin;
  iocfg.outports = Nout;
  
  int *insizes = (int*) malloc(sizeof(int) * iocfg.inports);
  int *outsizes = (int*) malloc(sizeof(int) * iocfg.outports);
  
  for (i = 0; i < iocfg.inports; ++i)
    insizes[i] = insizes_[i];
  for (i = 0; i < iocfg.outports; ++i)
    outsizes[i] = outsizes_[i];
  
  iocfg.insizes = insizes;
  iocfg.outsizes = outsizes;
  
  
  // List of Pointers to in vectors
  iocfg.inptr = (double **) malloc(sizeof(double *) * iocfg.inports);
  
  
  //
  // Alloc list of pointers for outvalues comming from libdyn
  // These pointers will be set by irpar_get_libdynconnlist
  //
  
  iocfg.outptr = (double **) malloc(sizeof(double *) * iocfg.outports);

  for (i=0; i<iocfg.outports; ++i)
    iocfg.outptr[i] = (double*) 0x0;

}


bool libdyn::add_libdyn_block(int blockid, void* comp_fn)
{
  return libdyn_compfnlist_add(sim->private_comp_func_list, blockid, LIBDYN_COMPFN_TYPE_LIBDYN, comp_fn);
}


void libdyn::dump_all_blocks()
{
  //
  // Dump all Blocks
  //
 
  libdyn_dump_all_blocks(sim);
}


void libdyn::destruct()
{
  free(iocfg.insizes);
  free(iocfg.outsizes);
  free(iocfg.inptr);
  free(iocfg.outptr);
  
  libdyn_del_simulation(sim);
}


