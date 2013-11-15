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

// #define DEBUG

#include <malloc.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <time.h>
#include <sys/time.h>
#include <sched.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>
#include <pthread.h>
//#include <sys/io.h>


extern "C" {
#include "libdyn.h"
#include "libdyn_scicos_macros.h"


#ifdef __ORTD_TARGET_ANDROID
// Declaration is missing in time.h in android NDK for some targets levels
    extern int clock_settime(clockid_t, const struct timespec *);
    extern int clock_nanosleep(clockid_t, int, const struct timespec *, struct timespec *);
#endif


}


#include "libdyn_cpp.h"
#include "global_shared_object.h"






class compu_func_synctimer_class {
public:
    compu_func_synctimer_class(struct dynlib_block_t *block);
    void destruct();
    void io(int update_states);
    int init();

    static int sync_callback(struct dynlib_simulation_t * sim) {
        void * obj = sim->sync_callback.userdat;
        compu_func_synctimer_class *p = (compu_func_synctimer_class *) obj;
        return p->real_sync_callback(sim);
    }


private:
    int magic;

    struct dynlib_block_t *block;

    int real_sync_callback(struct dynlib_simulation_t * sim );

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

    if (magic != 18454) {
        printf("wrong place\n");
        exit(-1);
    }

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


// 	sleep(1);


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

    magic = 18454;
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


int compu_func_synctimer_class::real_sync_callback( struct dynlib_simulation_t * sim )
{
    /*
     * This function is called before any of the output or state-update flags
     * are called.
     * If 0 is returned, the simulation will continue to run
     * If 1 is returned, the simulation will pause and has to be re-triggered externally.
     * e.g. by the trigger_computation input of the async nested_block.


    */

    if (true) { // Loop with absolute times

        // time for execution t and the interval
        struct timespec t, interval;

        // measure the current time
        clock_gettime(CLOCK_MONOTONIC,&t);



        do { // Main loop is now here

            // run the simulation
            // run one simulation step

            // Use C-functions to simulation one timestep
            libdyn_event_trigger_mask(sim, 1);
            libdyn_simulation_step(sim, 0);
            libdyn_simulation_step(sim, 1);

            // The simulation tells how long to wait
            double *T_pause = (double*) libdyn_get_input_ptr(block, 0);

#ifdef DEBUG
            fprintf(stderr, "Sync callback was called\n");
#endif

            if (*T_pause < 0) {
#ifdef DEBUG
                fprintf(stderr, "simulation is idle now\n");
#endif

                return 1; // return 1 exits the loop calling this callback
            }

#ifdef DEBUG
            fprintf(stderr, "Pausing simulation for %f\n", *T_pause);
#endif

            // calc time to wait
            interval.tv_sec =  0L;
            interval.tv_nsec = (long)1e9* (*T_pause);
            tsnorm(&interval);



//            // Task time T
//            clock_gettime(CLOCK_MONOTONIC,&curtime);
//            T = calcdiff(curtime,T0);


            // calculate time for the next execution
            t.tv_sec+=interval.tv_sec;
            t.tv_nsec+=interval.tv_nsec;
            tsnorm(&t);

            // wait
            clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);


            // Wait for the next simulation step
//   wait(*T_pause);

        } while (true);

    }


    if (false) {

        // run the simulation
        // run one simulation step

        // Use C-functions to simulation one timestep
        libdyn_event_trigger_mask(sim, 1);
        libdyn_simulation_step(sim, 0);
        libdyn_simulation_step(sim, 1);


        double *T_pause = (double*) libdyn_get_input_ptr(block, 0);

#ifdef DEBUG
        fprintf(stderr, "Sync callback was called\n");
#endif

        if (*T_pause < 0) {
#ifdef DEBUG
            fprintf(stderr, "simulation is idle now\n");
#endif

            return 1; // return 1 exits the loop calling this callback
        }

#ifdef DEBUG
        fprintf(stderr, "Pausing simulation for %f\n", *T_pause);
#endif


        // Wait for the next simulation step
        wait(*T_pause);

    }


    return 0; // return zero means that this callback is called again
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

/*
     New version as of 20.7.2013. Take this also as an example
*/


class ClockSyncBlock {
public:
    ClockSyncBlock(struct dynlib_block_t *block) {
        this->block = block;    // no nothing more here. The real initialisation take place in init()
    }
    ~ClockSyncBlock()
    {
        // free your allocated memory, ...
    }

    //
    // define states or other variables
    //

    bool ExitLoop;

    void initclock();
    void wait(double Tsamp);

    struct timespec t, interval, curtime, T0;
    double T;

    //
    // initialise your block
    //

    int init() {
        int *Uipar;
        double *Urpar;

        // Get the irpar parameters Uipar, Urpar
        libdyn_AutoConfigureBlock_GetUirpar(block, &Uipar, &Urpar);


        // set the initial states
        resetStates();

        // register the callback function to the simulator that shall trigger the simulation while running in a loop
        libdyn_simulation_setSyncCallback(block->sim, &syncCallback_ , this);
        libdyn_simulation_setSyncCallbackDestructor(block->sim, &syncCallbackDestructor_ , this);

        ExitLoop = false;

        // Return -1 to indicate an error, so the simulation will be destructed
        return 0;
    }


    inline void updateStates()
    {
        double *in1 = (double *) libdyn_get_input_ptr(block,0); // the first input port
        double *output = (double*) libdyn_get_output_ptr(block, 0); // the first output port
    }


    inline void calcOutputs()
    {
        double *in1 = (double *) libdyn_get_input_ptr(block,0); // the first input port
        double *output = (double*) libdyn_get_output_ptr(block, 0); // the first output port
    }


    inline void resetStates()
    {
    }


    int SyncCallback( struct dynlib_simulation_t * sim )
    {
        /*
         *		***	MAIN FUNCTION	***
         *
         * This function is called before any of the output or state-update flags
         * are called.
         * If 0 is returned, the simulation will continue to run
         * If 1 is returned, the simulation will pause and has to be re-triggered externally.
         * e.g. by the trigger_computation input of the async nested_block.
        */

#ifdef DEBUG
        printf("ClockSyncBlock: Threaded simulation started execution\n");
#endif

//     // wait until the callback function  real_syncDestructor_callback is called
//     pthread_mutex_lock(&ExitMutex);
//
        // time for execution t and the interval
        struct timespec t, interval;

        // measure the current time
        clock_gettime(CLOCK_MONOTONIC,&t);



        // This is the main loop of the new simulation
        while (!ExitLoop) {

            // run one step of the ortd simulator
            libdyn_event_trigger_mask(sim, 1);
            libdyn_simulation_step(sim, 0);
            libdyn_simulation_step(sim, 1);


            // The simulation tells how long to wait
            double *T_pause = (double*) libdyn_get_input_ptr(block, 0);

#ifdef DEBUG
            fprintf(stderr, "STEP\n");
#endif

            if (*T_pause < 0) {
#ifdef DEBUG
                fprintf(stderr, "simulation is idle now\n");
#endif
                return 1; // return 1 exits the loop calling this callback
            }

#ifdef DEBUG
            fprintf(stderr, "Pausing simulation for %f\n", *T_pause);
#endif

            // calc time to wait
            interval.tv_sec =  0L;
            interval.tv_nsec = (long)1e9* (*T_pause);
            tsnorm(&interval);

            // calculate time for the next execution
            t.tv_sec+=interval.tv_sec;
            t.tv_nsec+=interval.tv_nsec;
            tsnorm(&t);

            // wait
            clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);

        }


        return 1; // 1 - this shall not be executed again, directly after returning from this function!
    }

    int SyncDestructorCallback( struct dynlib_simulation_t * sim )
    {
#ifdef DEBUG
        printf("ClockSyncBlock: The Block containing simulation is about to be destructed\n");
#endif

        // Trigger termination of the the main loop
        ExitLoop = true;
//     pthread_mutex_unlock(&ExitMutex);

    }


    void printInfo() {
        fprintf(stderr, "I'm a ClockSyncBlock block\n");
    }

    // uncommonly used flags
    void PrepareReset() {}
    void HigherLevelResetStates() {}
    void PostInit() {}


    // The Computational function that is called by the simulator
    // and that distributes the execution to the various functions
    // in this C++ - Class, including: init(), io(), resetStates() and the destructor
    static int CompFn(int flag, struct dynlib_block_t *block) {
        return LibdynCompFnTempate<ClockSyncBlock>( flag, block ); // this expands a template for a C-comp fn
    }
    static int syncCallback_(struct dynlib_simulation_t * sim) {
        void * obj = sim->sync_callback.userdat;
        ClockSyncBlock *p = (ClockSyncBlock *) obj;
        return p->SyncCallback(sim);
    }
    static int syncCallbackDestructor_(struct dynlib_simulation_t * sim) {
        void * obj = sim->sync_callback.userdatDestructor;
        ClockSyncBlock *p = (ClockSyncBlock *) obj;
        return p->SyncDestructorCallback(sim);
    }

    // The data for this block managed by the simulator
    struct dynlib_block_t *block;
};





/*

*/


int ortd_compu_func_clock(int flag, struct dynlib_block_t *block)
{
// printf("comp_func flipflop: flag==%d\n", flag);
    int Nout = 1;
    int Nin = 0;

    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);

    int tmp = ipar[0];



    double *output;



    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        output = (double *) libdyn_get_output_ptr(block,0);

        struct timeval mytime;
        struct timezone myzone;

        gettimeofday(&mytime, &myzone);
        double usTos = 1/1000000.0;
        double time = (mytime.tv_sec+mytime.tv_usec * usTos );

        *output = time;
    }

    return 0;
    break;
    case COMPF_FLAG_UPDATESTATES:
        output = (double *) libdyn_get_output_ptr(block,0);

        return 0;
        break;
    case COMPF_FLAG_RESETSTATES:

        return 0;
        break;
    case COMPF_FLAG_CONFIGURE:  // configure
        //printf("New flipflop Block\n");
        libdyn_config_block(block, BLOCKTYPE_DYNAMIC, Nout, Nin, (void *) 0, 0);
        libdyn_config_block_output(block, 0, 1, DATATYPE_FLOAT, 1);

        return 0;
        break;
    case COMPF_FLAG_INIT:  // init
    {
        libdyn_set_work_ptr(block, (void *) NULL);
    }
    return 0;
    break;
    case COMPF_FLAG_DESTUCTOR: // destroy instance
    {
    }
    return 0;
    break;
    case COMPF_FLAG_PRINTINFO:
        printf("I'm a clock readout block.\n");
        return 0;
        break;
    }
}







//
// Blocks for thread notifications (condition variables)
// A shared object
//



class ThreadNotify_SharedObject : public ortd_global_shared_object {
public:
    ThreadNotify_SharedObject(const char* identName, dynlib_block_t *block, int Visibility) : ortd_global_shared_object(identName, block->sim, Visibility) {
        this->block = block;

        // Get the irpar parameters Uipar, Urpar
        libdyn_AutoConfigureBlock_GetUirpar(block, &Uipar, &Urpar);
    }

    ~ThreadNotify_SharedObject() {
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&condition);
    }

    int init() {
//         printf("Init of ThreadNotify_SharedObject\n");

        pthread_mutex_init(&mutex, NULL);
        pthread_cond_init(&condition, NULL);

        return 0; // init was ok
    }

    int WaitForSignal() {
//       printf("waiting for a notification\n");
      
        pthread_mutex_lock(&mutex);

        while (signal == 0) {
            pthread_cond_wait(&condition, &mutex);
        }
        int sig = signal;
        signal = 0;

        pthread_mutex_unlock(&mutex);

//         printf("Thread received a signal %d\n", sig);

        return sig;
    }

    void notify(int32_t sig) {
//          printf("notify Thread\n");

        pthread_mutex_lock(&mutex);
        signal = sig;
        pthread_mutex_unlock(&mutex);

        pthread_cond_signal(&condition);
    }

    int *Uipar;
    double *Urpar;
    dynlib_block_t *block;


    /*
        Some data of the shared object
    */
private:

    pthread_mutex_t mutex;
    pthread_cond_t condition;
    int32_t signal;


};




//
// An example block for accessing the shared object
//

class RecvNotificationsBlock {
public:
    RecvNotificationsBlock(struct dynlib_block_t *block) {
        this->block = block;    // no nothing more here. The real initialisation take place in init()
    }
    ~RecvNotificationsBlock()
    {
        // free your allocated memory, ...
    }

    //
    // define states or other variables
    //

    uint32_t signal;
    bool ExitLoop;
    ThreadNotify_SharedObject *IShObj; // instance of the shared object

    //
    // initialise your block
    //

    int init() {
//               printf("******** INIT simulation synchronised to notifications \n");
	      
        int *Uipar;
        double *Urpar;

        // Get the irpar parameters Uipar, Urpar
        libdyn_AutoConfigureBlock_GetUirpar(block, &Uipar, &Urpar);


        // register the callback function to the simulator that shall trigger the simulation while running in a loop
// 	printf("Try to sync this simulation (%p) to %p\n", block->sim, &syncCallback_);
        libdyn_simulation_setSyncCallback(block->sim, &syncCallback_ , this);
        libdyn_simulation_setSyncCallbackDestructor(block->sim, &syncCallbackDestructor_ , this);

        ExitLoop = false;


        // Get the condition management class
        IShObj = NULL;
        if ( ortd_GetSharedObj<ThreadNotify_SharedObject>(block, &IShObj) < 0 ) {
            return -1;
        }

//         printf("******** simulation synchronised to notifications \n");
        
        // Return -1 to indicate an error, so the simulation will be destructed
        return 0;
    }


    inline void updateStates()
    {
//         double *output = (double*) libdyn_get_output_ptr(block, 0); // the first output port

    }


    inline void calcOutputs()
    {
        int32_t *output = (int32_t*) libdyn_get_output_ptr(block, 0); // the first output port

        output[0] = signal;
    }


    inline void resetStates()
    {
    }



    int SyncCallback( struct dynlib_simulation_t * sim )
    {
        /*
         *		***	MAIN FUNCTION	***
         *
         * This function is called before any of the output or state-update flags
         * are called.
         * If 0 is returned, the simulation will continue to run
         * If 1 is returned, the simulation will pause and has to be re-triggered externally.
         * e.g. by the trigger_computation input of the async nested_block.
        */

         printf("Threaded simulation synchronised to notifications started execution\n");

//     // wait until the callback function  real_syncDestructor_callback is called
//     pthread_mutex_lock(&ExitMutex);
//

        double *output = (double*) libdyn_get_output_ptr(block, 0); // the first output port

        // This is the main loop of the new simulation
        while (!ExitLoop) {
//           printf("wait for UDP Data to receive\n");

            signal = IShObj->WaitForSignal();

            // run one step of the ortd simulator
            libdyn_event_trigger_mask(sim, 1);
            libdyn_simulation_step(sim, 0);
            libdyn_simulation_step(sim, 1);
        }


        return 1; // 1 - this shall not be executed again, directly after returning from this function!
    }

    int SyncDestructorCallback( struct dynlib_simulation_t * sim )
    {
        printf("The Block containing simulation is about to be destructed\n");

        // Trigger termination of the the main loop
        ExitLoop = true;
	IShObj->notify(-1);
//     pthread_mutex_unlock(&ExitMutex);

    }


    void printInfo() {
        fprintf(stderr, "I'm a RecvNotifications block\n");
    }

    // uncommonly used flags
    void PrepareReset() {}
    void HigherLevelResetStates() {}
    void PostInit() {}


    // The Computational function that is called by the simulator
    // and that distributes the execution to the various functions
    // in this C++ - Class, including: init(), io(), resetStates() and the destructor
    static int CompFn(int flag, struct dynlib_block_t *block) {
//       printf("*** FLAG %d %p\n", flag, block);
        return LibdynCompFnTempate<RecvNotificationsBlock>( flag, block ); // this expands a template for a C-comp fn
    }
    static int syncCallback_(struct dynlib_simulation_t * sim) {
        void * obj = sim->sync_callback.userdat;
        RecvNotificationsBlock *p = (RecvNotificationsBlock *) obj;
        return p->SyncCallback(sim);
    }
    static int syncCallbackDestructor_(struct dynlib_simulation_t * sim) {
        void * obj = sim->sync_callback.userdatDestructor;
        RecvNotificationsBlock *p = (RecvNotificationsBlock *) obj;
        return p->SyncDestructorCallback(sim);
    }

    // The data for this block managed by the simulator
    struct dynlib_block_t *block;
};

class ThreadNotify_Block {
public:
    ThreadNotify_Block(struct dynlib_block_t *block) {
        this->block = block;    //  nothing more here. The real initialisation take place in init()
    }
    ~ThreadNotify_Block()
    {
        // free your allocated memory, ...
    }

    //
    // define states or other variables
    //

    ThreadNotify_SharedObject *IShObj; // instance of the shared object


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

        // Obtain the shared object
        IShObj = NULL;
        if ( ortd_GetSharedObj<ThreadNotify_SharedObject>(block, &IShObj) < 0 ) {
            return -1;
        }

        // Return -1 to indicate an error, so the simulation will be destructed
        return 0;
    }


    inline void updateStates()
    {
    }


    inline void calcOutputs()
    {
        int32_t *signal = (int32_t *) libdyn_get_input_ptr(block,0); //
// 	out[0] = signal;
        IShObj->notify(*signal);
    }


    inline void resetStates()
    {
    }


    void printInfo() {
        fprintf(stderr, "I'm a ThreadNotify_Block\n");
    }

    // uncommonly used flags
    void PrepareReset() {}
    void HigherLevelResetStates() {}
    void PostInit() {}


    // The Computational function that is called by the simulator
    // and that distributes the execution to the various functions
    // in this C++ - Class, including: init(), io(), resetStates() and the destructor
    static int CompFn(int flag, struct dynlib_block_t *block) {
        return LibdynCompFnTempate<ThreadNotify_Block>( flag, block ); // this expands a template for a C-comp fn
    }

    // The data for this block managed by the simulator
    struct dynlib_block_t *block;
};













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
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid+0, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &compu_func_synctimer);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid+1, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &ortd_compu_func_clock);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid+2, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &ClockSyncBlock::CompFn);

    libdyn_compfnlist_add(sim->private_comp_func_list, blockid+110, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &SharedObjBlock<ThreadNotify_SharedObject>::CompFn);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid+111, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &RecvNotificationsBlock::CompFn);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid+112, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &ThreadNotify_Block::CompFn);




    printf("libdyn module sync initialised\n");

}


