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
#include <unistd.h>
#include <stdlib.h>

#include <time.h>
#include <sched.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>
#include <pthread.h>
#include <sys/io.h>


extern "C" {
#include "libdyn.h"
#include "libdyn_scicos_macros.h"


}








class compu_func_synctimer_class {
public:
    compu_func_synctimer_class(struct dynlib_block_t *block);
    void destruct();
    void io(int update_states);
    int init();
    
    static int sync_callback(void *obj) {
      compu_func_synctimer_class *p = (compu_func_synctimer_class *) obj;
      return p->real_sync_callback();
    }
   
   
private:
   struct dynlib_block_t *block;
   
   int real_sync_callback();
   
   void initclock();
   void wait(double Tsamp);

   
   struct timespec t, interval, curtime, T0;
   double T; 

  
};

#define NSEC_PER_SEC    1000000000
#define USEC_PER_SEC	1000000

static inline void tsnorm(struct timespec *ts)
{
    while (ts->tv_nsec >= NSEC_PER_SEC) {
        ts->tv_nsec -= NSEC_PER_SEC;
        ts->tv_sec++;
    }
}
static inline double calcdiff(struct timespec t1, struct timespec t2)
{
    long diff;
    diff = USEC_PER_SEC * ((int) t1.tv_sec - (int) t2.tv_sec);
    diff += ((int) t1.tv_nsec - (int) t2.tv_nsec) / 1000;
    return (1e-6*diff);
}

void compu_func_synctimer_class::initclock()
{
    double Tsamp = 0.0;
  
    interval.tv_sec =  0L;
    interval.tv_nsec = (long)1e9*Tsamp;
    tsnorm(&interval);

    /* get current time */
    clock_gettime(CLOCK_MONOTONIC,&t);

    /* start after one Tsamp */
    t.tv_sec+=interval.tv_sec;
    t.tv_nsec+=interval.tv_nsec;
    tsnorm(&t);

    T0 = t;
    T = 0.0;

}


void compu_func_synctimer_class::wait(double Tsamp)
{
//   double Tsamp;
  
    interval.tv_sec =  0L;
    interval.tv_nsec = (long)1e9*Tsamp;
    tsnorm(&interval);

//         /* calculate next shot */
//     t.tv_sec+=interval.tv_sec;
//     t.tv_nsec+=interval.tv_nsec;
//     tsnorm(&t);
//     
//      clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);

  // OR
     
    
     clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &interval, NULL);
     
	
	
	
	
//         /* Task time T */
//         clock_gettime(CLOCK_MONOTONIC,&curtime);
//         T = calcdiff(curtime,T0);

        /* periodic task */
        //  NAME(MODEL,_isr)(T);


//         t.tv_sec+=interval.tv_sec;
//         t.tv_nsec+=interval.tv_nsec;
//         tsnorm(&t);

}


compu_func_synctimer_class::compu_func_synctimer_class(dynlib_block_t* block)
{
    this->block = block;
}

int compu_func_synctimer_class::init()
{
    double *rpar = libdyn_get_rpar_ptr(block);
    int *ipar = libdyn_get_ipar_ptr(block);

    double Nin = ipar[0];
    double Nout = ipar[1];
    
    initclock();
    
    libdyn_simulation_setSyncCallback(block->sim, &compu_func_synctimer_class::sync_callback , this);

      // libdyn_simulation_setSyncCallback(struct dynlib_simulation_t *simulation, void (*sync_func)( void *userdat ), void *userdat)

    // Return -1 to indicate an error, so the simulation will be destructed
  
    return 0;
}


int compu_func_synctimer_class::real_sync_callback()
{
 /*  
  * This function is called before any of the output or state-update flags
  * are called. 
  * If 0 is returned, the simulation will continue to run
  * If 1 is returned, the simulation will pause and has to be re-triggered externally.
  * e.g. by the trigger_computation input of the async nested_block.
  */
 
  double *T_pause = (double*) libdyn_get_input_ptr(block, 0);
 
  fprintf(stderr, "Sync callback was called\n");
  
  if (*T_pause < 0) {
    fprintf(stderr, "simulation is idle now\n");
    
    return 1;
  }
  
  fprintf(stderr, "Pausing simulation for %f\n", *T_pause);
  wait(*T_pause);
//   sleep(1);
//   wait();
 
  return 0;
}


void compu_func_synctimer_class::io(int update_states)
{
    if (update_states==0) {
        double *output = (double*) libdyn_get_output_ptr(block, 0);
	
	*output = 1;
    }
}

void compu_func_synctimer_class::destruct()
{
    
}


// This is the main C-Callback function, which forwards requests to the C++-Class above
int compu_func_synctimer(int flag, struct dynlib_block_t *block)
{

//     printf("comp_func template: flag==%d\n", flag);

    double *in;
    double *rpar = libdyn_get_rpar_ptr(block);
    int *ipar = libdyn_get_ipar_ptr(block);

    int Nin = 1;
    int Nout = 1;


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        in = (double *) libdyn_get_input_ptr(block,0);
        compu_func_synctimer_class *worker = (compu_func_synctimer_class *) libdyn_get_work_ptr(block);

        worker->io(0);

    }
    return 0;
    break;
    case COMPF_FLAG_UPDATESTATES:
    {
        in = (double *) libdyn_get_input_ptr(block,0);
        compu_func_synctimer_class *worker = (compu_func_synctimer_class *) libdyn_get_work_ptr(block);

        worker->io(1);

    }
    return 0;
    break;
    case COMPF_FLAG_CONFIGURE:  // configure. NOTE: do not reserve memory or open devices. Do this while init instead!
    {
        int i;
        libdyn_config_block(block, BLOCKTYPE_STATIC, Nout, Nin, (void *) 0, 0);

        
            libdyn_config_block_input(block, 0, 1, DATATYPE_FLOAT);
            libdyn_config_block_output(block, 0, 1, DATATYPE_FLOAT, 0);


    }
    return 0;
    break;
    case COMPF_FLAG_INIT:  // init
    {
        compu_func_synctimer_class *worker = new compu_func_synctimer_class(block);
        libdyn_set_work_ptr(block, (void*) worker);

        int ret = worker->init();
        if (ret < 0)
            return -1;
    }
    return 0;
    break;
    case COMPF_FLAG_DESTUCTOR: // destroy instance
    {
        compu_func_synctimer_class *worker = (compu_func_synctimer_class *) libdyn_get_work_ptr(block);

        worker->destruct();
	delete worker;

    }
    return 0;
    break;
    case COMPF_FLAG_PRINTINFO:
        printf("I'm a compu_func_synctimer block\n");
        return 0;
        break;

    }
}

// Export to C so the libdyn simulator finds this function
extern "C" {
    // ADJUST HERE: must match to the function name in the end of this file
    int libdyn_module_synchronisation_siminit(struct dynlib_simulation_t *sim, int bid_ofs);
}

// CHANGE HERE: Adjust this function name to match the name of your module
int libdyn_module_synchronisation_siminit(struct dynlib_simulation_t *sim, int bid_ofs)
{

    // Register my blocks to the given simulation

    int blockid = 15100;  // CHANGE HERE: choose a unique id for each block
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &compu_func_synctimer);

    printf("libdyn module sync initialised\n");

}

