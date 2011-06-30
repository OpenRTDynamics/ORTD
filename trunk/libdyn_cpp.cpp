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

extern "C" {
  #include "irpar.h"
}

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define libdyn_get_typesize(type) (sizeof(double))

#define idiots_check




irpar::irpar()
{
  ipar = NULL;
  rpar = NULL;
  Nrpar = 0;
  Nipar = 0;
}

bool irpar::load_from_afile(char *fname_i, char* fname_r)
{
    int err; // variable for errors

    err = irpar_load_from_afile(&ipar, &rpar, &Nipar, &Nrpar, fname_i, fname_r);
    if (err == -1) {
      printf("Error loading irpar files\n");
      return false;      
    }
    
    return true;
}

bool irpar::load_from_afile(char* fname)
{
	strcpy(fname_ipar, fname);
	strcat(fname_ipar, ".ipar");
	strcpy(fname_rpar, fname);
	strcat(fname_rpar, ".rpar");

	printf("fnames ipar = %s\n", fname_ipar);
	printf("fnames rpar = %s\n", fname_rpar);
	
	return this->load_from_afile(fname_ipar, fname_rpar);

}


void irpar::destruct()
{
  if (ipar != NULL)
    free(ipar);
  if (rpar != NULL)
    free(rpar);
}



//
// class libdyn_nested
//

bool libdyn_nested::internal_init(int Nin, const int* insizes_, const int* intypes, int Nout, const int* outsizes_, const int* outtypes)
{
  int sum, tmp, i;
  double *p;

  iocfg.provided_outcaches = 0;
  
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
  
  // Initially there is no master
  this->ld_master = NULL;
  
  // Initially there is no simulation
  this->current_sim = NULL;
  
  
  return true;
}

void libdyn_nested::set_buffer_inptrs()
{
  int i;
  char *ptr = (char*) InputBuffer;

  // share the allocated_inbuffer accoss all inputs
  
  i = 0;  iocfg.inptr[i] = (double *) ptr; //sim->cfg_inptr(i, (double *) ptr);
  
  for (i = 1; i < iocfg.inports; ++i) {
    int element_len = sizeof(double); // libdyn_get_typesize(iocfg.intypes[i-1]); // FIXME
    int vlen = iocfg.insizes[i-1];
    ptr += element_len*vlen;
    
  //  sim->cfg_inptr(i, (double*) ptr);
    iocfg.inptr[i] = (double *) ptr;

  }

}


bool libdyn_nested::allocate_inbuffers()
{
  int i;
  
  int nBytes_for_input = 0;
  
  for (i = 0; i < iocfg.inports; ++i) {
    int element_len = sizeof(double); // libdyn_get_typesize(iocfg.intypes[i]); // FIXME
    int vlen = iocfg.insizes[i];
  
    nBytes_for_input = nBytes_for_input + element_len * vlen;
  }
  
  InputBuffer = (void*) malloc(nBytes_for_input);
  
  this->set_buffer_inptrs();
  
  return true;
}



libdyn_nested::libdyn_nested(int Nin, const int* insizes_, const int* intypes, int Nout, const int* outsizes_, const int* outtypes)
{
  this->sim_slots = NULL;
  this->internal_init(Nin, insizes_, intypes, Nout, outsizes_, outtypes);
}

libdyn_nested::libdyn_nested(int Nin, const int* insizes_, const int* intypes, int Nout, const int* outsizes_, const int* outtypes, bool use_buffered_input)
{
  this->sim_slots = NULL; // initially slots are not used
  this->use_buffered_input = use_buffered_input;
   
  this->internal_init(Nin, insizes_, intypes, Nout, outsizes_, outtypes);

  if (use_buffered_input)
    this->allocate_inbuffers();

  
  //this->libdyn_nested(Nin, insizes_, intypes, Nout, outsizes_, outtypes);
  
}

void libdyn_nested::destruct()
{
  
  // LAter: list management
  if (current_sim != NULL) {
    current_sim->destruct();
    delete current_sim;
  }

  free_slots();
  
}


void libdyn_nested::allocate_slots(int n)
{
  this->Nslots = n;
  this->sim_slots = (libdyn **) malloc(n * sizeof( libdyn * ));
  
  slot_addsim_pointer = 0;
  current_slot = 0;
}

void libdyn_nested::free_slots()
{
  if (this->sim_slots != NULL)
    free(sim_slots);
}

int libdyn_nested::slots_available()
{
  return Nslots - slot_addsim_pointer;
}


void libdyn_nested::set_master(libdyn_master* master)
{
  this->ld_master = master;
}


int libdyn_nested::add_simulation(irpar* param, int boxid)
{
  if (sim_slots != NULL)
    if (slots_available() <= 0)
      return -1;
  
  return this->add_simulation(param->ipar, param->rpar, boxid);
}



// Add a simulation based on provided irpar set
int libdyn_nested::add_simulation(int* ipar, double* rpar, int boxid)
{
  if (sim_slots != NULL)
    if (slots_available() <= 0)
      return -1;


  libdyn *sim;
  
  sim = new libdyn(&iocfg);
  sim->set_master(ld_master);
  
  
  int err;
  err = sim->irpar_setup(ipar, rpar, boxid); // compilation of schematic

  if (err == -1) {
      // There may be some problems during compilation. 
      // Errors are reported on stdout    
    printf("Error in libdyn\n");
    
    return err;
  }

  if (this->add_simulation(sim) < 0)
    return -1;
  
  
  return err;
}

int libdyn_nested::add_simulation(libdyn* sim)
{
  if (sim_slots != NULL)
    if (slots_available() <= 0)
      return -1;

  // later: list management
  current_sim = sim;
  
  
  // add simulation to the next slot
  if (sim_slots != NULL) {
    // slots should be used
    sim_slots[ slot_addsim_pointer ] = sim;
    slot_addsim_pointer++;
  }
  
  return 1;

}

bool libdyn_nested::load_simulations(int* ipar, double* rpar, int start_boxid, int NSimulations)
{
  int irparid;
  
  // add all simulations
  for (irparid = start_boxid; irparid < start_boxid + NSimulations; ++irparid) {
    this->add_simulation(ipar, rpar, irparid);
  }
}



bool libdyn_nested::set_current_simulation(int nSim)
{
  if (sim_slots != NULL) {
    current_sim = this->sim_slots[nSim];
  } else {
    fprintf(stderr, "libdyn_nested: slots are not configured\n"); 
  }
}



void libdyn_nested::copy_outport_vec(int nPort, void* dest)
{
  void *src = current_sim->get_vec_out(nPort);
  int len = this->iocfg.outsizes[nPort];
  int datasize = sizeof(double); // FIXME
  
  memcpy(dest, src, len*datasize);
}

// makes only sense of use_buffered_input == true
void libdyn_nested::copy_inport_vec(int nPort, void* src)
{
  if (use_buffered_input == false) {
    fprintf(stderr, "Hmmm\n");
    return;
  }
  
  int element_len = sizeof(double); // libdyn_get_typesize(iocfg.intypes[i]); // FIXME
  int vlen = iocfg.insizes[nPort];

  
  memcpy((void*) iocfg.inptr[nPort], src, element_len * vlen);
}


void libdyn_nested::simulation_step(int update_states)
{
  current_sim->simulation_step(update_states);
}

void libdyn_nested::event_trigger_mask(int mask)
{
  current_sim->event_trigger_mask(mask);
}





//
// class libdyn
//

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

  // Make master available, if any
  sim->master = (void*) ld_master; // anonymous copy for "C"-only Code

  // Call INIT FLag of all blocks
  error = libdyn_simulation_init(sim);
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

libdyn::libdyn(int Nin, const int* insizes_, int Nout, const int* outsizes_)
{
  error = 0;
  
  int sum, tmp, i;
  double *p;

  iocfg.provided_outcaches = 0;
  
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
  
  // Initially there is no master
  this->ld_master = NULL;
}

libdyn::libdyn(struct libdyn_io_config_t * iocfg)
{
  error = 0;
  
//   printf("iocfg: %d %d\n", iocfg->inports, iocfg->outports);
  
  memcpy((void*) &this->iocfg, (void*) iocfg, sizeof(struct libdyn_io_config_t));
  
//   printf("iocfg cpy: %d %d\n", this->iocfg.inports, this->iocfg.outports);
  
  // Initially there is no master
  this->ld_master = NULL;
}

void libdyn::set_master(libdyn_master* ld_master)
{
  this->ld_master = ld_master;
//   printf("Master was set\n");
}

int libdyn::prepare_replacement_sim(int* ipar, double* rpar, int boxid)
{
  // TODO
}

void libdyn::switch_to_replacement_sim()
{
  // TODO
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
  libdyn_del_simulation(sim);

  free(iocfg.insizes);
  free(iocfg.outsizes);
  free(iocfg.inptr);
  free(iocfg.outptr);
}

//
// Master
//


libdyn_master::libdyn_master()
{
  this->realtime_environment = RTENV_UNDECIDED;
  
  global_comp_func_list = libdyn_new_compfnlist();
  printf("Created new libdyn master\n");
  
#ifdef REMOTE
  // Initial subsystems (not available)
  dtree = NULL;
  pmgr = NULL;
  stream_mgr = NULL;
  rts_mgr = NULL;
  
   init_communication(10000);  // FIXME remove as by default no remote interface is desired
#endif
}

// remote_control_tcpport - if 0 the no remote control will be set-up
libdyn_master::libdyn_master(int realtime_env, int remote_control_tcpport)
{
  this->realtime_environment = realtime_env;
  
  global_comp_func_list = libdyn_new_compfnlist();
  printf("Created new libdyn master; tcpport = %d \n", remote_control_tcpport);
  
#ifdef REMOTE
  dtree = NULL;
  pmgr = NULL;
  stream_mgr = NULL;
  rts_mgr = NULL;

  if (remote_control_tcpport != 0) {
    init_communication(remote_control_tcpport);   // FIXME: Handle return value 
  }
#endif
}


#ifdef REMOTE
	  
int libdyn_master::init_communication(int tcpport)
{
  // init communication_server
  
    printf("*Initialising remote control interface on port %d\n", tcpport);

    this->rts_mgr = new rt_server_threads_manager();

//     printf("rts_mgr = %p\n", rts_mgr);
//     rts_mgr->command_map.clear();
    
    int cret = rts_mgr->init_tcp(tcpport);

    if (cret < 0) {
      fprintf(stderr , "Initialisation of communication server failed\n");
      rts_mgr->destruct();
      delete this->rts_mgr;  // FIXME INCLUDE THIS
      return cret;
    }

    
    printf("Creating directory\n");    
    dtree = new directory_tree(this->rts_mgr);
  
   pmgr = new parameter_manager( rts_mgr, dtree );
   stream_mgr = new ortd_stream_manager(rts_mgr, dtree );
  
    rts_mgr->start_main_loop_thread();
    
    return 1;
  
}

void libdyn_master::close_communication()
{
    
  if (pmgr != NULL) {
  pmgr->destruct();
  delete pmgr;
  }

  if (stream_mgr != NULL) {
    stream_mgr->destruct();
    delete stream_mgr;
  }
  
  if (dtree != NULL) {
  dtree->destruct();
  delete dtree;

  if (rts_mgr != NULL) {
  rts_mgr->destruct();
  delete rts_mgr;
  }
  
  
  }
}

#endif

int libdyn_master::init_blocklist()
{
  // call modules
}

  
#ifdef REMOTE
rt_server_threads_manager* libdyn_master::get_communication_server()
{
  return this->rts_mgr;
}
#endif


void libdyn_master::destruct()
{
  libdyn_del_compfnlist(global_comp_func_list);
  
  #ifdef REMOTE
  close_communication();
  #endif
  
}

