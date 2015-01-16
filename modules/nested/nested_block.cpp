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
#include <sched.h>

extern "C" {

#include "libdyn_scicos_macros.h"
#include "irpar.h"
#include "math.h"
#include "realtime.h"
}

#include "libdyn_cpp.h"
#include "nested_onlineexchange.h"


// #define DEBUG


extern "C" {
    int libdyn_module_nested_siminit(struct dynlib_simulation_t *sim, int bid_ofs);
}

template <class compute_instance> class background_computation {
public:

    // the class compute_instance should provide a method int computer()
    background_computation(compute_instance *ci);
    background_computation(compute_instance *ci, struct TaskPriority_t TaskPriority);
    
    
    ~background_computation();

    bool start_computation();

    void join_computation();
    void TerminateThread(int signal);

    bool get_CompNotRunning();
    void set_CompNotRunning(bool f);

    bool finishedAComputation();
    void reset_ThereWasAComputaion();


private:
    void loop();
    void internalInit(compute_instance *ci, struct TaskPriority_t TaskPriority);

    compute_instance * ci;
    struct TaskPriority_t TaskPriority;

    int signal;

    pthread_mutex_t mutex;
    pthread_cond_t cond;

    pthread_t thread;

    volatile bool CompNotRunning;
    pthread_mutex_t CompNotRunning_mutex;
    bool ThereWasAComputaion;

    pthread_mutex_t comp_active_mutex;


    static void * start_thread(void *obj);
    void signal_thread(int sig);
};



template <class compute_instance> background_computation<compute_instance>::background_computation(compute_instance *ci)
{
  struct TaskPriority_t TaskPriority;
  TaskPriority.Npar = 0; TaskPriority.par = NULL;

//   internalInit<compute_instance>(ci, TaskPriority);
  internalInit(ci, TaskPriority);  
}

template <class compute_instance> background_computation<compute_instance>::background_computation(compute_instance *ci, struct TaskPriority_t TaskPriority)
{
//   internalInit<compute_instance>(ci, TaskPriority);  
internalInit(ci, TaskPriority);  
}

template <class compute_instance> void background_computation<compute_instance>::internalInit(compute_instance *ci, struct TaskPriority_t TaskPriority)
{
  this->TaskPriority = TaskPriority;
  
  
    pthread_mutex_init(&mutex, NULL);  pthread_mutex_lock(&mutex); // mutex initially locked
    pthread_cond_init(&cond, NULL);

    pthread_mutex_init(&CompNotRunning_mutex, NULL);
    pthread_mutex_init(&comp_active_mutex, NULL);


    this->ci = ci; // the callback class

    signal = 0; // ???

    set_CompNotRunning(true); // initially the computation is finished
    ThereWasAComputaion = false; // the first compu did not happen for now

    int rc = pthread_create(&thread, NULL, &background_computation<compute_instance>::start_thread, (void *) this);
    if (rc) {
        fprintf(stderr, "ERROR; return code from pthread_create() is %d\n", rc);
	throw 1;
    }    
    
    // TODO wait until the thread is ready
}

template <class compute_instance> background_computation<compute_instance>::~background_computation()
// background_computation::~background_computation()
{
    /*    signal = -1;
        pthread_cond_signal(&cond); // Notify reader thread
        pthread_join(thread, NULL);*/

    join_computation();

    fprintf(stderr, "background computer: joinined\n");

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    pthread_mutex_destroy(&CompNotRunning_mutex);
    pthread_mutex_destroy(&comp_active_mutex);
}


template <class compute_instance> void * background_computation<compute_instance>::start_thread(void *obj)
{
    background_computation<compute_instance> *ci;

    ci = (background_computation<compute_instance> *) obj;
    ortd_rt_SetThreadProperties2( ci->TaskPriority );
    ci->loop();
}

template <class compute_instance> void background_computation<compute_instance>::loop()
{
    signal = 0;
    // mutex is initially locked now.
    // Thus, this thread can only be signaled, if 
    // execution pauses at pthread_cond_wait, meaning the thread is initialised
    //
    // This was changed to fix a BUG on 15.8.14
    // 
    // NOTE: Also investigate ThreadNotify_SharedObject in sync_main.cpp (module synchronisation)
    
//    pthread_mutex_lock(&mutex);  // moved to

    for (;;) {

        while (signal == 0) {
            pthread_cond_wait(&cond, &mutex);
        }
        int sig = signal;
        signal = 0;

        set_CompNotRunning(false);

#ifdef DEBUG
        fprintf(stderr, "Comp mgr thread rcved signal\n");
#endif

        pthread_mutex_unlock(&mutex);
	// This thread can be sigaled from here on, but that would not be catched


        // do something based on sig
        if (sig == 1) {

            pthread_mutex_lock(&comp_active_mutex);

            // Run the function computer() of the given class, which type is parametrised by this template
            ci->computer();

            pthread_mutex_unlock(&comp_active_mutex);


// 	     computation finished
            set_CompNotRunning(true);
        }

        if (sig == -1)
            return;

        pthread_mutex_lock(&mutex);
	
	
    }
}



// async_simulationBlock: added schematic
// ortd: Simulation set-up successfully; entering main loop.
// -- step O --- sim=0x3b7b90
// -- step U --- sim=0x3b7b90
// async_simulationBlock: Trigger computation
// Comp mgr sending signal to thread
// Task Prio1 (flags) would be 2 (1 means ORTD_RT_REALTIMETASK)
// Task Prio2 would be 0
// Task CPU would be -1
// realtime.c: initialised a non real-time thread
// realtime.c: Successfully set the task properties
// -- step O --- sim=0x3b7b90
// -- step U --- sim=0x3b7b90


template <class compute_instance> void background_computation<compute_instance>::signal_thread(int sig)
{

#ifdef DEBUG
    fprintf(stderr, "Comp mgr sending signal to thread\n");
//     sleep(1);
#endif

    pthread_mutex_lock(&mutex);
    signal = sig;
    pthread_mutex_unlock(&mutex);
    pthread_cond_signal(&cond); // Notify reader thread

    sched_yield();

}

template <class compute_instance> bool background_computation<compute_instance>::start_computation()
{

    bool CompNotRunning_cpy = get_CompNotRunning();

    //   only if computation is not running
    if (CompNotRunning_cpy == true) {
        ThereWasAComputaion = true;
        signal_thread(1); // start the computation
    }

}

template <class compute_instance> void background_computation<compute_instance>::TerminateThread(int signal)
{
//    printf("About to kill thread\n");
    ortd_pthread_cancel(thread);
}

template <class compute_instance> void background_computation<compute_instance>::join_computation()
{


    // wait until computation is complete
    signal_thread(-1);

    pthread_join(thread, NULL);
}

template <class compute_instance> bool background_computation<compute_instance>::get_CompNotRunning()
{

    pthread_mutex_lock(&CompNotRunning_mutex);
    bool CompNotRunning_cpy = CompNotRunning;
    pthread_mutex_unlock(&CompNotRunning_mutex);

//      printf("get finished %d\n", finished_cpy ? 1 : 0);

    return CompNotRunning_cpy;
}

template <class compute_instance> void background_computation<compute_instance>::set_CompNotRunning(bool f)
{
    pthread_mutex_lock(&CompNotRunning_mutex);
    CompNotRunning = f;
//     printf("set finished to %d\n", f ? 1 : 0);
    pthread_mutex_unlock(&CompNotRunning_mutex);
}

template <class compute_instance> bool background_computation<compute_instance>::finishedAComputation()
{
    pthread_mutex_lock(&CompNotRunning_mutex);
    bool CompNotRunning_cpy = CompNotRunning;
    pthread_mutex_unlock(&CompNotRunning_mutex);

    if (ThereWasAComputaion && CompNotRunning_cpy)
        return true;
    else
        return false;

}

template <class compute_instance> void background_computation<compute_instance>::reset_ThereWasAComputaion()
{
//   fprintf(stderr, "Now there has never been a computatoin\n");
    ThereWasAComputaion = false;
}






/*
class ortd_asychronous_computation_thread {
  public:


  private:


};*/


template <class callback_class> class ortd_asychronous_computation {
private:

    libdyn_nested2 * simnest;
    libdyn *sim;

    callback_class *cb;

    background_computation< ortd_asychronous_computation > *computer_mgr;

    int asynchron_simsteps;

public:
    // simnest: a ready to use set-up schematic
    ortd_asychronous_computation(callback_class *cb, libdyn_nested2 * simnest, int asynchron_simsteps);
    ortd_asychronous_computation(callback_class *cb, libdyn * simnest, int asynchron_simsteps); // not present

    ~ortd_asychronous_computation();

    int computeNSteps(int N);
    bool computation_finished();
    void reset();
    void join_computation();


    int computer();
};


template <class callback_class> ortd_asychronous_computation<callback_class>::ortd_asychronous_computation(callback_class *cb, libdyn_nested2 * simnest, int asynchron_simsteps)
{
    this->cb = cb;
    this->simnest = simnest;
    this->sim = simnest->current_sim;
    this->asynchron_simsteps = asynchron_simsteps;

    computer_mgr = new background_computation< ortd_asychronous_computation > (this);
}

template <class callback_class> ortd_asychronous_computation<callback_class>::~ortd_asychronous_computation()
{
    delete computer_mgr;
}


template <class callback_class> int ortd_asychronous_computation<callback_class>::computeNSteps(int N)
{
    // trigger computation

    computer_mgr->start_computation();

}

template <class callback_class> bool ortd_asychronous_computation<callback_class>::computation_finished()
{
    bool finished = computer_mgr->finishedAComputation();
    return finished;
}

template <class callback_class> void ortd_asychronous_computation<callback_class>::reset()
{
    computer_mgr->reset_ThereWasAComputaion();
}


template <class callback_class> int ortd_asychronous_computation<callback_class>::computer()
{
    // Simulate in a thread

    // Get the C-libdyn object
    dynlib_simulation_t* C_sim = sim->get_C_SimulationObject();


    if (asynchron_simsteps == 1) {
#ifdef DEBUG
        fprintf(stderr, "async_nested: running computer in single mode\n");
#endif
        sim->simulation_step(0);
        sim->simulation_step(1);

        // call the callback to copy the results
        cb->async_copy_output_callback();
    }

    if (asynchron_simsteps == 2) {
#ifdef DEBUG
        fprintf(stderr, "async_nested: running computer in endless mode\n");
#endif


        if ( !sim->IsSyncronised() ) { // unsynchronised simulation

            do { // the simulation synchronises itselft to something
#ifdef DEBUG
                fprintf(stderr, "async_nested: another run\n");
#endif



                sim->event_trigger_mask(1);

                sim->simulation_step(0);

                if (sim->getSyncState()) {
#ifdef DEBUG
                    fprintf(stderr, "async_nested: leaving\n");
#endif
                    break;
                }

                sim->simulation_step(1);


            } while (1);
        } else if ( sim->IsSyncronised() ) {
            // synchronised simulation
#ifdef DEBUG
            fprintf(stderr, "async_nested: Starting the computation of a synchronised simuation\n");
#endif


            do { // the simulation synchronises itselft to something


                /*
                * Wait for synhronisation callback function
                */

                int SyncCallbackRetState = sim->RunSyncCallbackFn();
                if ( SyncCallbackRetState == 1 ) {
                    // abort
#ifdef DEBUG
                    fprintf(stderr, "async_nested: leaving\n");
#endif
                    break;
                }


#ifdef DEBUG
                fprintf(stderr, "async_nested: another run\n");
#endif


//                 // sync_callback returned --> run one simulation step
//                  sim->event_trigger_mask(1);
//                 sim->simulation_step(0);
//                 sim->simulation_step(1);




                //             if (simulation->sync_callback.sync_func != NULL) {
//                 simulation->sync_callback.sync_callback_state = (*simulation->sync_callback.sync_func)(simulation->sync_callback.userdat);
//                 if (simulation->sync_callback.sync_callback_state == 1) {
//                     return 1;
//                 }
//
//                 // sync_callback returned --> output calculation.
//             }
//
            } while (1);

        }

        // call the callback to copy the results
        cb->async_copy_output_callback();

        sim->reset_blocks();


    }


    // while (some abort criterion set by the schematic sim)



#ifdef DEBUG
    fprintf(stderr, "async_nested: finished\n");
#endif
}









class compu_func_nested_class {
    // ld_simnest and ld_simnest2

public:
    compu_func_nested_class(struct dynlib_block_t *block);
    void destruct();
    void io(int update_states);
    void io_sync(int update_states);
    void io_async(int update_states);
    void reset();

    void async_copy_output_callback();

    int init();
private:
    struct dynlib_block_t *block;

    void lock_output();
    void unlock_output();
    pthread_mutex_t output_mutex;

    // Used when asyn computation is desired
    bool async_comp;
    ortd_asychronous_computation<compu_func_nested_class> * async_comp_mgr;

    libdyn_nested2 * simnest;
    libdyn_master * master;
    nested_onlineexchange *exchange_helper; // ifdef REMOTE
    irpar *param;
    char *nested_sim_name;

    int Nin;
    int Nout;
    int *insizes, *outsizes;
    int *intypes, *outtypes;

    int Nsimulations, dfeed, asynchron_simsteps;

    int error;

};

compu_func_nested_class::compu_func_nested_class(dynlib_block_t* block)
{
    this->block = block;
}

int compu_func_nested_class::init()
{
    int simCreateCount;
    int i;



    double *rpar = libdyn_get_rpar_ptr(block);
    int *ipar = libdyn_get_ipar_ptr(block);

    struct irpar_ivec_t insizes_irp, outsizes_irp, intypes_irp, outtypes_irp, param;


    int error = 0; // FIXME Länge prüfen
    if ( irpar_get_ivec(&insizes_irp, ipar, rpar, 10) < 0 ) error = 1 ;
    if ( irpar_get_ivec(&outsizes_irp, ipar, rpar, 11) < 0 ) error = 1 ;
    if ( irpar_get_ivec(&intypes_irp, ipar, rpar, 12) < 0 ) error = 1 ;
    if ( irpar_get_ivec(&outtypes_irp, ipar, rpar, 13) < 0 ) error = 1 ;
    if ( irpar_get_ivec(&param, ipar, rpar, 20) < 0 ) error = 1 ;

    if (error == 1) {
        fprintf(stderr, "nested: could not get a parameter from irpar set\n");
        return -1;
    }

    // Try to get the name of the simulation nest
    struct irpar_ivec_t nested_sim_name__;
    if ( irpar_get_ivec(&nested_sim_name__, ipar, rpar, 21) < 0 ) {
        // no name was provided
        nested_sim_name = NULL;
    } else {
        irpar_getstr(&nested_sim_name, nested_sim_name__.v, 0, nested_sim_name__.n );
    }

    this->insizes = insizes_irp.v;
    this->outsizes = outsizes_irp.v;
    this->intypes = intypes_irp.v;
    this->outtypes = outtypes_irp.v;


// 	int dfeed = param.v[1];

    Nin = insizes_irp.n;
    Nout = outsizes_irp.n;

    Nsimulations = param.v[0];
    dfeed = param.v[1];
    asynchron_simsteps = param.v[2];




    bool use_buffered_input = false;  // in- and out port values are not buffered (default)
    if (asynchron_simsteps > 0) {
        use_buffered_input = true;  // with async computation buffered inputs have to be used.
    }

    // create a new container for multiple simulations
    simnest = new libdyn_nested2(Nin, insizes, intypes, Nout, outsizes, outtypes, use_buffered_input);
    simnest->set_parent_simulation( block->sim ); // set a parent simulation
    simnest->allocate_slots(Nsimulations);

    // If there is a libdyn master : use it
    master = (libdyn_master *) block->sim->master;
    if (master == NULL) {  // no master available
        fprintf(stderr, "ERROR: libdyn: ld_nested blocks require a libdyn master\n");

        simnest->destruct(); // all simulations are destructed
        delete simnest;

        return -1;
    }

    master->check_memory();
    simnest->set_master(master);

    // init the simulation exchange helper if required
    exchange_helper = NULL;

    if (nested_sim_name != NULL)
        if (master != NULL && master->dtree != NULL) {
            fprintf(stderr, "libdyn nested: Register <%s> for online exchange\n", nested_sim_name);
            exchange_helper = new nested_onlineexchange(nested_sim_name, simnest);
        } else {
            fprintf(stderr, "WARNING: libdyn_nested: online exchanging of simulations requires a libdyn master and a directory\n");
//             fprintf(stderr, "master = %p, master->dtree=%p\n", master, master->dtree); // FIXME REMOVE DEBUG
            master->check_memory();

            simnest->destruct(); // all simulations are destructed
            delete simnest;

            return -1;
        }

    //
    // set pointers to the input ports of this block
    //
    if (!use_buffered_input) {
        // not required of the inputs are buffered
        for (i = 0; i < Nin; ++i) {
            double *in_p = (double*) libdyn_get_input_ptr(block, i);
            simnest->cfg_inptr(i, (void*) in_p);
        }
    }


    // load all schematics
    for (simCreateCount = 0; simCreateCount < Nsimulations; ++simCreateCount) {
        int shematic_id = 900 + simCreateCount;

        printf("libdyn_nested: loading shematic id %d\n", shematic_id);
        if (simnest->add_simulation(-1, ipar, rpar, shematic_id) < 0) {
            goto destruct_simulations;  // An error
        }

//         printf("added schematic\n");
    }


    //
    // start async computation manager if desired
    //

    if (asynchron_simsteps <= 0) {
        this->async_comp = false;
    } else {
        // Initialise async computation manager
        printf("nested: Using async\n");

        this->async_comp = true;
        this->async_comp_mgr = new ortd_asychronous_computation<compu_func_nested_class>(this, simnest, asynchron_simsteps);

        pthread_mutex_init(&this->output_mutex, NULL);
    }

//     printf("nested was set-up\n");

    return 0;

destruct_simulations:
    fprintf(stderr, "nested: Destructing all simulations\n");

    simnest->destruct(); // all simulations are destructed
    delete simnest;

    return -1;

}

void compu_func_nested_class::io_sync(int update_states)
{
    int i,j;



    // map scicos events to libdyn events
    // convert scicos events to libdyn events (acutally there is no conversion)
    int eventmask = __libdyn_event_get_block_events(block);
    simnest->event_trigger_mask(eventmask);

    if (update_states == 1) {
        //
        // update states
        //

//         simnest->simulation_step(1);
        simnest->simulation_step_supdate();

        //        printf("neszed sup\n");
    } else {
        //
        // calc outputs
        //


//         simnest->simulation_step(0);
        simnest->simulation_step_outpute();

// 	simnest->initialised_replaced_simulation = true;

        for (i=0; i< Nout ; ++i) {
            double *out_p = (double*) libdyn_get_output_ptr(block, i);
            simnest->copy_outport_vec(i, out_p);

//       printf("nested outp %f\n", out_p[0]);
        }

        // Some control signal input analysis
        double *switch_inp = (double*) libdyn_get_input_ptr(block, Nin+0);
        double *reset_inp = (double*) libdyn_get_input_ptr(block, Nin+1);

        if (*reset_inp > 0) {
#ifdef DEBUG
            fprintf(stderr, "reset states\n");
#endif
            simnest->reset_blocks();
        }

        int nSim = *switch_inp;
#ifdef DEBUG
        fprintf(stderr, "switch sig=%f reset=%f sw=%d\n", *switch_inp, *reset_inp, nSim);
#endif
        simnest->set_current_simulation(nSim);

    }


}

void compu_func_nested_class::io_async(int update_states)
{
    int i;

    // map scicos events to libdyn events
    // convert scicos events to libdyn events (acutally there is no conversion)
    int eventmask = __libdyn_event_get_block_events(block);
    simnest->event_trigger_mask(eventmask);

//      printf("io_async up=%d\n", update_states);

    if (update_states == 1) {
        //
        // update states
        //
        double *comptrigger_inp = (double*) libdyn_get_input_ptr(block, Nin+1);
//         printf("starting comp = %f\n", *comptrigger_inp);

        if (*comptrigger_inp > 0.5) {
#ifdef DEBUG
            fprintf(stderr, "Trigger computation\n");
#endif
            this->async_comp_mgr->computeNSteps(asynchron_simsteps);
        }


    } else {
        //
        // set comp_finieshed output to 1 if result is available
        //

        double *comp_finished = (double*) libdyn_get_output_ptr(block, Nout);

        // If the background computation is finished the the output to 1
        // nonblocking check for a result
        if (this->async_comp_mgr->computation_finished()) {
            *comp_finished = 1;
        } else {
            *comp_finished = 0;
        }


    }


}

void compu_func_nested_class::async_copy_output_callback()
{
// async copy of outputs
    int i;

    lock_output();


    Nout;
    for (i=0; i< Nout ; ++i) {
        double *out_p = (double*) libdyn_get_output_ptr(block, i);

        simnest->copy_outport_vec(i, out_p);


        //       printf("nested outp %f\n", out_p[0]);
    }


    unlock_output();

}



void compu_func_nested_class::io(int update_states)
{
    if (async_comp == false) {
        io_sync(update_states);
    } else {
        io_async(update_states);
    }
}

void compu_func_nested_class::reset()
{
    if (async_comp == false) {
        simnest->reset_blocks();
#ifdef DEBUG
        fprintf(stderr, "Resetting sync simnest\n");
#endif
    } else {
#ifdef DEBUG
        fprintf(stderr, "Resetting async\n");
#endif
        async_comp_mgr->reset();
    }
}


void compu_func_nested_class::destruct()
{
    printf("nested_block.cpp: destructing\n");

    // libdyn_simulation_CallSyncCallbackDestructor(struct dynlib_simulation_t *simulation)

    // notify user code running in the thread that it is over
    simnest->CallSyncCallbackDestructor();

    if (this->async_comp) {
        delete async_comp_mgr;
        pthread_mutex_destroy(&this->output_mutex);
    }




    simnest->destruct();   // HERE WAS A BUG solved at 2.7.12: both lines were flipped  and the exchange_helper was deleted before this!
    delete simnest;

    // do this AFTER simnest has been destroyed because its potentially deletes the managed irpar data
    if (exchange_helper != NULL) {
#ifdef DEBUG
        fprintf(stderr, "compu_func_nested_class: delete exchange helper for %s\n", nested_sim_name);
#endif
        delete exchange_helper;
    }

    // exchange_helper depends on nested_sim_name
    if (this->nested_sim_name != NULL)
        free(this->nested_sim_name);

}

void compu_func_nested_class::lock_output()
{
    pthread_mutex_lock(&this->output_mutex);
}

void compu_func_nested_class::unlock_output()
{
    pthread_mutex_unlock(&this->output_mutex);
}



int compu_func_nested(int flag, struct dynlib_block_t *block)
{

//     printf("comp_func nested: flag==%d\n", flag);

    double *in;
    double *rpar = libdyn_get_rpar_ptr(block);
    int *ipar = libdyn_get_ipar_ptr(block);

    int Nin = ipar[0];
    int Nout = ipar[1];


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        in = (double *) libdyn_get_input_ptr(block,0);
        compu_func_nested_class *worker = (compu_func_nested_class *) libdyn_get_work_ptr(block);

        worker->io(0);

    }
    return 0;
    break;
    case COMPF_FLAG_UPDATESTATES:
    {
        in = (double *) libdyn_get_input_ptr(block,0);
        compu_func_nested_class *worker = (compu_func_nested_class *) libdyn_get_work_ptr(block);

        worker->io(1);

    }
    return 0;
    break;
    case COMPF_FLAG_RESETSTATES:
    {
        compu_func_nested_class *worker = (compu_func_nested_class *) libdyn_get_work_ptr(block);

        worker->reset();;
    }
    return 0;
    break;
    case COMPF_FLAG_CONFIGURE:  // configure. NOTE: do not reserve memory or open devices. Do this while init instead!
    {
//         int irpar_get_ivec(struct irpar_ivec_t *ret, int *ipar, double *rpar, int id);

        struct irpar_ivec_t insizes, outsizes, intypes, outtypes, param;

        irpar_get_ivec(&insizes, ipar, rpar, 10);
        irpar_get_ivec(&outsizes, ipar, rpar, 11);
        irpar_get_ivec(&intypes, ipar, rpar, 12);
        irpar_get_ivec(&outtypes, ipar, rpar, 13);
        irpar_get_ivec(&param, ipar, rpar, 20);

        int dfeed = param.v[1];

        int Ninports = insizes.n;
        int Noutports = outsizes.n;

        int synchron_simsteps = param.v[2];

        int Noutextra = 0;

        if (synchron_simsteps > 0) {
            ;  // with async computation
            Noutextra = 1;
        }


        int i;
        libdyn_config_block(block, BLOCKTYPE_DYNAMIC, Noutports+Noutextra, Ninports + 2, (void *) 0, 0);

        for (i = 0; i < Ninports; ++i) {
            libdyn_config_block_input(block, i, insizes.v[i], DATATYPE_FLOAT);
        }

        libdyn_config_block_input(block, Ninports, 1, DATATYPE_FLOAT); // switch signal input
        libdyn_config_block_input(block, Ninports+1, 1, DATATYPE_FLOAT); // reset signal input

        for (i = 0; i < Noutports; ++i) {
            libdyn_config_block_output(block, i, outsizes.v[i], DATATYPE_FLOAT, dfeed);
        }

        if (Noutextra == 1)
            libdyn_config_block_output(block, Noutports, 1, DATATYPE_FLOAT, dfeed); // computation ready output. (when using async comp)


    }
    return 0;
    break;
    case COMPF_FLAG_INIT:  // init
    {
        compu_func_nested_class *worker = new compu_func_nested_class(block);
        libdyn_set_work_ptr(block, (void*) worker);

        int ret = worker->init();
        if (ret < 0)
            return -1;
    }
    return 0;
    break;
    case COMPF_FLAG_DESTUCTOR: // destroy instance
    {
        printf("nested_block.cpp: fn: compu_func_nested: destructing\n");

        compu_func_nested_class *worker = (compu_func_nested_class *) libdyn_get_work_ptr(block);
        worker->destruct();
        delete worker;

        printf("nested_block.cpp: fn: compu_func_nested: destructing -- ok\n");
    }
    return 0;
    break;
    case COMPF_FLAG_PRINTINFO:
        printf("I'm a nested block\n");

        return 0;
        break;

    }
}




/*
  A state machine using nested simulations
*/

class compu_func_statemachine_class {
public:
    compu_func_statemachine_class(struct dynlib_block_t *block);
    void destruct();
    void io(int update_states);
    void io_sync(int update_states);
    void reset();

    int init();
private:
    struct dynlib_block_t *block;

//     void lock_output();
//     void unlock_output();
//     pthread_mutex_t output_mutex;


    libdyn_nested2 * simnest;
    libdyn_master * master;
    irpar *param;

    int Ndatain;
    int Ndataout;
    int *insizes, *outsizes;
    int *intypes, *outtypes;

    int Nsimulations;
    int inittial_state;

    double active_state; // the currently active state / schematic // FIXME: move this to int


    int error;

    int Nglobal_states; // size of memory space for state storage of the nested simulation
    void *global_states_buffer;
    void *global_states_buffer_initial; // pointer to initial values (used in case of a reset)
};

compu_func_statemachine_class::compu_func_statemachine_class(dynlib_block_t* block)
{
    this->block = block;
    this->global_states_buffer = NULL;
}

int compu_func_statemachine_class::init()
{
    int simCreateCount;
    int i;

    double *rpar = libdyn_get_rpar_ptr(block);
    int *ipar = libdyn_get_ipar_ptr(block);

    struct irpar_ivec_t insizes_irp, outsizes_irp, intypes_irp, outtypes_irp, param;


    int error = 0; // FIXME Länge prüfen
    if ( irpar_get_ivec(&insizes_irp, ipar, rpar, 10) < 0 ) error = 1 ;
    if ( irpar_get_ivec(&outsizes_irp, ipar, rpar, 11) < 0 ) error = 1 ;
    if ( irpar_get_ivec(&intypes_irp, ipar, rpar, 12) < 0 ) error = 1 ;
    if ( irpar_get_ivec(&outtypes_irp, ipar, rpar, 13) < 0 ) error = 1 ;
    if ( irpar_get_ivec(&param, ipar, rpar, 20) < 0 ) error = 1 ;

    if (error == 1) {
        printf("nested: could not get parameter from irpar set\n");
        return -1;
    }

    this->insizes = insizes_irp.v;
    this->outsizes = outsizes_irp.v;
    this->intypes = intypes_irp.v;
    this->outtypes = outtypes_irp.v;


// 	int dfeed = param.v[1];

    Ndatain = insizes_irp.n - 1; // minus one because insizes also contain the size of global states which is an input to the neszed simulation only
    Ndataout = outsizes_irp.n - 2;

    Nsimulations = param.v[0];
    Nglobal_states = param.v[1];
    inittial_state = param.v[2];

//     printf("Nsim = %d, Nglobal_states = %d, init_state %d\n", Nsimulations, Nglobal_states, inittial_state);

    if (Nglobal_states != this->insizes[Ndatain+0]) { // assertion
//       printf("**** %d != %d \n", Nglobal_states, this->insizes[Ndatain+0]);
        return -1;
    }

    // fetch the initial x_global
    struct irpar_rvec_t param_x_global;

    if ( irpar_get_rvec(&param_x_global, ipar, rpar, 21) < 0 ) return -1;
    if (param_x_global.n  != Nglobal_states) return -1;

    this->global_states_buffer_initial = (void*) param_x_global.v;

    // Allocate memeory for the global states buffer
    this->global_states_buffer = (double*) malloc( sizeof(double) * Nglobal_states );

    // copy initial values for x_global
    memcpy(this->global_states_buffer, this->global_states_buffer_initial, sizeof(double) * Nglobal_states);

    //
    // set-up nested
    //
    bool use_buffered_input = false;  // in- and out port values are not buffered

    // create a new container for multiple simulations
    simnest = new libdyn_nested2(Ndatain+1, insizes, intypes, Ndataout+2, outsizes, outtypes, use_buffered_input);
    simnest->set_parent_simulation( block->sim ); // set a parent simulation
    simnest->allocate_slots(Nsimulations);

    //
    // set input pointers of the nest to the input ports of the outer block
    //
    for (i = 0; i < Ndatain; ++i) { // minus 1 because the last input is no input to the OUTER block but an internal thing for global_states_buffer
        double *in_p = (double*) libdyn_get_input_ptr(block, i);
        simnest->cfg_inptr(i, (void*) in_p);
    }

    // inputs to the nest are the states
    simnest->cfg_inptr(Ndatain, (void*) this->global_states_buffer); // The input of the nested to the global_states_buffer

    // If there is a libdyn master : use it
    master = (libdyn_master *) block->sim->master;
    if (master == NULL) {  // no master available
        fprintf(stderr, "ERROR: libdyn: parameter block requires a libdyn master\n");

        simnest->destruct(); // all simulations are destructed
        delete simnest;

        return -1;
    }

    simnest->set_master(master);

    // load all schematics
    for (simCreateCount = 0; simCreateCount < Nsimulations; ++simCreateCount) {
        int shematic_id = 900 + simCreateCount;

        printf("libdyn_nested statemachine: loading shematic id %d\n", shematic_id);
        if (simnest->add_simulation(-1, ipar, rpar, shematic_id) < 0) {
            goto destruct_simulations;  // An error
        }
    }

    // the initial state
    simnest->set_current_simulation(inittial_state-1);

    return 0;

destruct_simulations:
    printf("nested: Destructing all simulations\n");

    simnest->destruct(); // all simulations are destructed
    delete simnest;

    if (this->global_states_buffer != NULL) {
        free(this->global_states_buffer);
        this->global_states_buffer = NULL;
    }

    return -1;

}

void compu_func_statemachine_class::io_sync(int update_states)
{
    int i,j;

    // map scicos events to libdyn events
    // convert scicos events to libdyn events (acutally there is no conversion)
    int eventmask = __libdyn_event_get_block_events(block);
    simnest->event_trigger_mask(eventmask);

    if (update_states == 1) {
        //
        // update states
        //

        simnest->simulation_step(1);
        double tmp;
        simnest->copy_outport_vec(Ndataout+0, &tmp);

        int active = round(tmp)-1;
        if (active >= 0) {
            // copy the x_global states
            simnest->copy_outport_vec(Ndataout+1, this->global_states_buffer);

//              fprintf(stderr, "Switch to state %d\n", active, tmp);
#ifdef DEBUG
            fprintf(stderr, "Switch to state %d\n", active, tmp);
            //printf( "Switch to state %d \n", active, tmp);
#endif

            // switch to the new state
            simnest->reset_blocks();
            simnest->set_current_simulation(active);
            this->active_state = active;


        }

    } else {
        //
        // calc outputs
        //

        simnest->simulation_step(0);

        double *current_state = (double*) libdyn_get_output_ptr(block, Ndataout); // FIXME: This output does not work by now
        *current_state = this->active_state;

        for (i=0; i< Ndataout ; ++i) {
            double *out_p = (double*) libdyn_get_output_ptr(block, i);
            simnest->copy_outport_vec(i, out_p);
        }
    }
}

void compu_func_statemachine_class::reset()
{
//     fprintf(stderr, "nested: reset (NOT TESTED FOR NOW; contact the author)\n");
    // the initial state
    simnest->reset_blocks();

    // go to the initial state
    simnest->set_current_simulation(inittial_state-1);

    // copy initial values for x_global
    memcpy(this->global_states_buffer, this->global_states_buffer_initial, sizeof(double) * Nglobal_states);
}





void compu_func_statemachine_class::io(int update_states)
{
    io_sync(update_states);
}

void compu_func_statemachine_class::destruct()
{
//   fprintf(stderr, "nested: delete statemachine...\n");

    simnest->destruct();
    delete simnest;

    if (this->global_states_buffer != NULL)
        free(this->global_states_buffer);

//   fprintf(stderr, "nested: delete statemachine done\n");

}



int compu_func_statemachine(int flag, struct dynlib_block_t *block)
{

//     printf("comp_func nested: flag==%d\n", flag);

    double *in;
    double *rpar = libdyn_get_rpar_ptr(block);
    int *ipar = libdyn_get_ipar_ptr(block);

    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        in = (double *) libdyn_get_input_ptr(block,0);
        compu_func_statemachine_class *worker = (compu_func_statemachine_class *) libdyn_get_work_ptr(block);

        worker->io(0);

    }
    return 0;
    break;
    case COMPF_FLAG_UPDATESTATES:
    {
        in = (double *) libdyn_get_input_ptr(block,0);
        compu_func_statemachine_class *worker = (compu_func_statemachine_class *) libdyn_get_work_ptr(block);

        worker->io(1);

    }
    return 0;
    break;
    case COMPF_FLAG_RESETSTATES:
    {
        compu_func_statemachine_class *worker = (compu_func_statemachine_class *) libdyn_get_work_ptr(block);

        worker->reset();;
    }
    return 0;
    break;
    case COMPF_FLAG_CONFIGURE:  // configure. NOTE: do not reserve memory or open devices. Do this while init instead!
    {
        struct irpar_ivec_t insizes, outsizes, intypes, outtypes, param;

        irpar_get_ivec(&insizes, ipar, rpar, 10);
        irpar_get_ivec(&outsizes, ipar, rpar, 11);
        irpar_get_ivec(&intypes, ipar, rpar, 12);
        irpar_get_ivec(&outtypes, ipar, rpar, 13);
        irpar_get_ivec(&param, ipar, rpar, 20);

        int Nglobal_states = param.v[1];
        int dfeed = 1;

        // IO of the OUTER block
        int Ndatainports = insizes.n - 1; // minus one because insizes also contain the size of global states which is an input to the neszed simulation only
        int Ndataoutports = outsizes.n - 2;

        int i;
        libdyn_config_block(block, BLOCKTYPE_DYNAMIC, Ndataoutports+2, Ndatainports, (void *) 0, 0);

        for (i = 0; i < Ndatainports; ++i) {
            libdyn_config_block_input(block, i, insizes.v[i], DATATYPE_FLOAT);
        }

        for (i = 0; i < Ndataoutports; ++i) {
            libdyn_config_block_output(block, i, outsizes.v[i], DATATYPE_FLOAT, dfeed);
        }

        libdyn_config_block_output(block, Ndataoutports+0, 1, DATATYPE_FLOAT, dfeed); // output of the currently active state
        libdyn_config_block_output(block, Ndataoutports+1, Nglobal_states , DATATYPE_FLOAT, dfeed); // Output for the global states
    }
    return 0;
    break;
    case COMPF_FLAG_INIT:  // init
    {
        compu_func_statemachine_class *worker = new compu_func_statemachine_class(block);
        libdyn_set_work_ptr(block, (void*) worker);

        int ret = worker->init();
        if (ret < 0)
            return -1;
    }
    return 0;
    break;
    case COMPF_FLAG_DESTUCTOR: // destroy instance
    {
        compu_func_statemachine_class *worker = (compu_func_statemachine_class *) libdyn_get_work_ptr(block);

        worker->destruct();
        delete worker;

    }
    return 0;
    break;
    case COMPF_FLAG_PRINTINFO:
        printf("I'm a statemachine block\n");
        return 0;
        break;

    }
}








int compu_func_survivereset(int flag, struct dynlib_block_t *block)
{
    // contained in a state of a state machine
    // this block remembers values, that are still
    // available after an reset of the state

    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);

    int len = ipar[0];
    double initval = rpar[0];

    int Nout = 1;
    int Nin = 1;


    double *buffer = (double*) libdyn_get_work_ptr(block);


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        void *out = (void *) libdyn_get_output_ptr(block,0);
        void *in = (void *) libdyn_get_input_ptr(block, 0);

//         out[0] = (*in > 0.5) ? 0 : 1;

    }
    return 0;
    break;
    case COMPF_FLAG_UPDATESTATES:
        return 0;
        break;
    case COMPF_FLAG_CONFIGURE:  // configure
    {
        libdyn_config_block(block, BLOCKTYPE_STATIC, Nout, Nin, (void *) 0, 0);

        libdyn_config_block_input(block, 0, 1, DATATYPE_FLOAT);
        libdyn_config_block_output(block, 0, 1, DATATYPE_FLOAT, 0);
    }
    return 0;
    break;
    case COMPF_FLAG_PREPARERESET:  //
    {
        void *out = (void *) libdyn_get_output_ptr(block,0);
        void *in = (void *) libdyn_get_input_ptr(block, 0);

        unsigned int Nbytes = sizeof(double)*(len) + sizeof(unsigned int);
        memcpy(buffer, in, Nbytes);

#ifdef DEBUG
        fprintf(stderr, "survivereset: Saved data\n");
#endif
    }
    return 0;
    break;
    case COMPF_FLAG_RESETSTATES:  //
    {
        // initialise the output
        void *out = (void *) libdyn_get_output_ptr(block,0);
        void *in = (void *) libdyn_get_input_ptr(block, 0);

        unsigned int Nbytes = sizeof(double)*(len) + sizeof(unsigned int);
        memcpy(out, buffer, Nbytes);

#ifdef DEBUG
        fprintf(stderr, "survivereset: Restored data\n");
#endif
    }
    return 0;
    break;
    case COMPF_FLAG_INIT:  // init
    {
        unsigned int Nbytes = sizeof(double)*(len) + sizeof(unsigned int);
        void *buffer__ = malloc(Nbytes);
// 	  memset((void*) buffer, 0,  Nbytes );

        memcpy(buffer__, (void*) &initval, Nbytes);

        libdyn_set_work_ptr(block, (void *) buffer__);
    }
    return 0;
    break;
    case COMPF_FLAG_DESTUCTOR: // destroy instance
    {
        void *buffer__ = (void*) libdyn_get_work_ptr(block);
        free(buffer__);
    }
    return 0;
    break;
    case COMPF_FLAG_PRINTINFO:
        printf("I'm a survivereset block\n");
        return 0;
        break;

    }
}


/*
 *  Async computations, new version
*/






template <class callback_class> class ortd_asychronous_computation2 {
private:

    libdyn_nested2 * simnest;
    libdyn *sim;

    callback_class *cb;

    background_computation< ortd_asychronous_computation2 > *computer_mgr;

    int asynchron_simsteps;
    
    struct TaskPriority_t TaskPriority;

public:
    // simnest: a ready to use set-up schematic
    ortd_asychronous_computation2(callback_class *cb, libdyn_nested2 * simnest, int asynchron_simsteps, struct TaskPriority_t TaskPriority);
    ortd_asychronous_computation2(callback_class *cb, libdyn_nested2 * simnest, int asynchron_simsteps);

    ~ortd_asychronous_computation2();

    int compute();
    bool computation_finished();
    void reset();
    void TerminateThread(int signal);
    void join_computation();

    int computer();
};


template <class callback_class> ortd_asychronous_computation2<callback_class>::ortd_asychronous_computation2(callback_class *cb, libdyn_nested2 * simnest, int asynchron_simsteps, struct TaskPriority_t TaskPriority)
{
  this->TaskPriority = TaskPriority;
    this->cb = cb;
    this->simnest = simnest;
    this->sim = simnest->current_sim;
    this->asynchron_simsteps = asynchron_simsteps;
  
    computer_mgr = new background_computation< ortd_asychronous_computation2 > (this, TaskPriority);  
}

template <class callback_class> ortd_asychronous_computation2<callback_class>::ortd_asychronous_computation2(callback_class *cb, libdyn_nested2 * simnest, int asynchron_simsteps)
{
    TaskPriority.Npar = NULL; TaskPriority.par = 0; // no priority given
    this->cb = cb;
    this->simnest = simnest;
    this->sim = simnest->current_sim;
    this->asynchron_simsteps = asynchron_simsteps;
    
    computer_mgr = new background_computation< ortd_asychronous_computation2 > (this, TaskPriority);
}

template <class callback_class> ortd_asychronous_computation2<callback_class>::~ortd_asychronous_computation2()
{
    delete computer_mgr;
}


template <class callback_class> int ortd_asychronous_computation2<callback_class>::compute()
{
    // trigger computation
    computer_mgr->start_computation();
}

template <class callback_class> bool ortd_asychronous_computation2<callback_class>::computation_finished()
{
    bool finished = computer_mgr->finishedAComputation();
    return finished;
}

template <class callback_class> void ortd_asychronous_computation2<callback_class>::reset()
{
    computer_mgr->reset_ThereWasAComputaion();
}

template <class callback_class> void ortd_asychronous_computation2<callback_class>::TerminateThread(int signal)
{
    computer_mgr->TerminateThread(signal);
}

template <class callback_class> int ortd_asychronous_computation2<callback_class>::computer()
{
    // Simulate in a thread

    // Get the C-libdyn object
    dynlib_simulation_t* C_sim = sim->get_C_SimulationObject();


    if ( !sim->IsSyncronised() ) { // unsynchronised simulation --> only one execution step
#ifdef DEBUG
        fprintf(stderr, "async_nested2: This is an unsynchronised simulation --> running computer in single mode\n");
#endif
        sim->event_trigger_mask(1);
        sim->simulation_step(0);

#ifdef DEBUG
        fprintf(stderr, "async_nested2: Call state-update flag\n");
#endif

        sim->simulation_step(1);

#ifdef DEBUG
        fprintf(stderr, "async_nested2: copying outputs\n");
#endif


        // call the callback to copy the results
        cb->async_copy_output_callback();
    } else {


#ifdef DEBUG
        fprintf(stderr, "async_nested: Starting the computation of a synchronised simuation\n");
#endif


        do { // the simulation synchronises itselft to something


            /*
            * Wait for synhronisation callback function
            */

            int SyncCallbackRetState = sim->RunSyncCallbackFn();
            if ( SyncCallbackRetState == 1 ) {
                // abort
#ifdef DEBUG
                fprintf(stderr, "async_nested: leaving\n");
#endif
                break;
            }


#ifdef DEBUG
            fprintf(stderr, "async_nested: another run\n");
#endif


        } while (1);
    }


    // call the callback to copy the results
    cb->async_copy_output_callback();

    sim->reset_blocks();

#ifdef DEBUG
    fprintf(stderr, "async_nested: finished\n");
#endif

}























class async_simulationBlock {
public:
    async_simulationBlock(struct dynlib_block_t *block) {
        this->block = block;    // no nothing more here. The real initialisation take place in init()
    }
    ~async_simulationBlock()
    {
        // free your allocated memory, ...
        printf("async_simulationBlock: destructing\n");

        // notify user code running in the thread that it is over
        simnest->CallSyncCallbackDestructor();


        destruct();
    }

    //
    // define states or other variables
    //


    void lock_output()   {
        pthread_mutex_lock(&this->output_mutex);
    }
    void unlock_output() {
        pthread_mutex_unlock(&this->output_mutex);
    }
    pthread_mutex_t output_mutex;

    // Used when asyn computation is desired
    ortd_asychronous_computation2<async_simulationBlock> * async_comp_mgr;

    libdyn_nested2 * simnest;
//     libdyn_master * master;
//     nested_onlineexchange *exchange_helper; // ifdef REMOTE
    char *nested_sim_name;

    int Nin;
    int Nout;
    int *insizes, *outsizes;
    int *intypes, *outtypes;

    int dfeed;
    
    struct TaskPriority_t TaskPriority;



    //
    // initialise your block
    //

    int init() {
        fprintf(stderr, "async_simulationBlock: Creating an async simulation V2\n");


        int *Uipar;
        double *Urpar;

        // Get the irpar parameters Uipar, Urpar
        libdyn_AutoConfigureBlock_GetUirpar(block, &Uipar, &Urpar);

        //
        // extract some structured sample parameters
        //



        simnest = NULL;
        async_comp_mgr = NULL;
        nested_sim_name = NULL;


        try {


            struct irpar_ivec_t insizes_irp, outsizes_irp, intypes_irp, outtypes_irp, param;


            // fetch parameters from Uipar, Urpar, throw an exception if something goes wrong
            int error = 0; // FIXME Länge prüfen
            if ( irpar_get_ivec(&insizes_irp, Uipar, Urpar, 10) < 0 ) throw 1;
            if ( irpar_get_ivec(&outsizes_irp, Uipar, Urpar, 11) < 0 ) throw 1;
            if ( irpar_get_ivec(&intypes_irp, Uipar, Urpar, 12) < 0 ) throw 1;
            if ( irpar_get_ivec(&outtypes_irp, Uipar, Urpar, 13) < 0 ) throw 1;
//     if ( irpar_get_ivec(&param, Uipar, Urpar, 20) < 0 ) error = 1 ;

            // The I/O configuration of the nested simulation
            this->insizes = insizes_irp.v;
            this->outsizes = outsizes_irp.v;
            this->intypes = intypes_irp.v;
            this->outtypes = outtypes_irp.v;

            Nin = insizes_irp.n;
            Nout = outsizes_irp.n;


            // Try to get the name of the simulation nest
            struct irpar_ivec_t nested_sim_name__;
            if ( irpar_get_ivec(&nested_sim_name__, Uipar, Urpar, 21) < 0 ) {
                // no name was provided
                nested_sim_name = NULL;
            } else {
                irpar_getstr(&nested_sim_name, nested_sim_name__.v, 0, nested_sim_name__.n );
            }

            // Try to get the priority of the task to be creaded
            struct irpar_ivec_t Prio_irp;
            if ( (irpar_get_ivec(&Prio_irp, Uipar, Urpar, 22) < 0) ) {
                // no prio was attached
                TaskPriority.par = NULL;
		TaskPriority.Npar = 0;
            } else {
	       TaskPriority.par = Prio_irp.v;
	       TaskPriority.Npar = Prio_irp.n;
	       
//             ortd_rt_SetThreadProperties(Prio_irp.v, Prio_irp.n); // TODO: ????
            }

            int Nsimulations=1;
            bool use_buffered_input = true;  // in- and out port values are buffered


            // create a new container for multiple simulations
            simnest = new libdyn_nested2(Nin, Nout, use_buffered_input);
            simnest->set_parent_simulation( block->sim ); // set a parent simulation
            simnest->allocate_slots(Nsimulations);  // tell the expected number of nested simulations

            //
            // set the nested simulations's input signal
            // pointers to the input ports of this block
            //
            int i;
            for (i = 0; i < Nin; ++i) {
                void *in_p = (void*) libdyn_get_input_ptr(block, i);
                simnest->cfg_inptr(i, (void*) NULL, insizes[i], intypes[i] );
            }

            for (i = 0; i < Nout; ++i) {
                simnest->cfg_output(i, outsizes[i], outtypes[i] );
            }

            // Finished in seting up the nested simulations
            // This must be called before adding any simulation
            simnest->FinishConfiguration();


            // load all schematics
            int simCreateCount;

            for (simCreateCount = 0; simCreateCount < Nsimulations; ++simCreateCount) {
                int shematic_id = 900 + simCreateCount;

                fprintf(stderr, "async_simulationBlock: loading shematic id %d\n", shematic_id);
                if (simnest->add_simulation(-1, Uipar, Urpar, shematic_id) < 0) {
                    throw 4;  // An error
                }

                fprintf(stderr, "async_simulationBlock: added schematic\n");
            }


            // Start the thread
            this->async_comp_mgr = new ortd_asychronous_computation2<async_simulationBlock>(this, simnest, 0, TaskPriority);
	    
            // Register Terminate callback function
            libdyn_simulation_setSyncCallbackTerminateThread(simnest->get_current_simulation_libdynSimStruct(), &syncCallbackTerminateThread__, this);

            pthread_mutex_init(&this->output_mutex, NULL);

            // set the initial states
            resetStates();
	     
	    
        } catch (int e) {
            fprintf(stderr, "ld_async: something went wrong. Exception = %d\n", e);
            destruct();
            return -1;
        }

        return 0;
    }

    void destruct() {
        fprintf(stderr, "async_simulationBlock: Destructing all simulations\n");

        // delete computation manager
        if (async_comp_mgr != NULL) {
            delete async_comp_mgr;
        }

        pthread_mutex_destroy(&this->output_mutex);

        // destruct the simulation
        if (simnest != NULL) {
            simnest->destruct();
            delete simnest;
        }

        // Free allocated data for the simulation name
        if (this->nested_sim_name != NULL)
            free(this->nested_sim_name);

    }

    int syncCallbackTerminateThread() {
        fprintf(stderr, "async_simulationBlock: Forcing thread to terminate\n");
        async_comp_mgr->TerminateThread(1);
    }

    static int syncCallbackTerminateThread__(struct dynlib_simulation_t * sim) {
        void * obj = sim->sync_callback.userdatTerminateThread; // the instance of the class
        async_simulationBlock *p = (async_simulationBlock *) obj;
        return p->syncCallbackTerminateThread();
    }

    void async_copy_output_callback()
    {
        // async copy of outputs
        int i;

        lock_output();

        Nout;
        for (i=0; i< Nout ; ++i) {
            void *out_p = (void*) libdyn_get_output_ptr(block, i);

            simnest->copy_outport_vec(i, out_p);
//                    printf("nested outp %f\n", ((double*)out_p)[0]);
        }

        unlock_output();
    }

    inline void updateStates()
    {
        double *comptrigger_inp = (double*) libdyn_get_input_ptr(block, Nin + 0); // additional input #0
//          printf("starting comp = %f\n", *comptrigger_inp);

        if (*comptrigger_inp > 0.5) {
#ifdef DEBUG
            fprintf(stderr, "async_simulationBlock: Trigger computation\n");
#endif
            this->async_comp_mgr->compute();
        }
    }


    inline void calcOutputs()
    {
        //
        // set comp_finieshed output to 1 if result is available
        //

        double *comp_finished = (double*) libdyn_get_output_ptr(block, Nout + 0); // additional output #0

        // If the background computation is finished the the output to 1
        // nonblocking check for a result
        if (this->async_comp_mgr->computation_finished()) {
            *comp_finished = 1;
        } else {
            *comp_finished = 0;
        }

    }

    inline void resetStates()
    {
//    printf("reset states async\n");
//         double *comp_finished = (double*) libdyn_get_output_ptr(block, Nout + 0); // additional output #0
//         *comp_finished = 0;   // added on 8.8.14
//         
        this->async_comp_mgr->reset(); // added on 8.8.14
	
	
      // TODO: forward reset events
    }



    void printInfo() {
        fprintf(stderr, "I'm a async_simulationBlock block\n");
    }

    // uncommonly used flags
    void PrepareReset() {}
    void HigherLevelResetStates() {}
    void PostInit() {}


    // The Computational function that is called by the simulator
    // and that distributes the execution to the various functions
    // in this C++ - Class, including: init(), io(), resetStates() and the destructor
    static int CompFn(int flag, struct dynlib_block_t *block) {
        return LibdynCompFnTempate<async_simulationBlock>( flag, block ); // this expands a template for a C-comp fn
    }

    // The data for this block managed by the simulator
    struct dynlib_block_t *block;
};







class ld_NoResetNest {
public:
    ld_NoResetNest(struct dynlib_block_t *block) {
        this->block = block;    // no nothing more here. The real initialisation take place in init()
    }
    ~ld_NoResetNest()
    {
        destruct();
    }


    //
    // define states or other variables
    //

    irpar *param;

    // variables for nesting schematics
    libdyn_nested2 * simnest;
    irpar_string *SimnestName;

    int Nsimulations;
    int Nin;
    int Nout;
    int *insizes, *outsizes;
    int *intypes, *outtypes;

    //
    // initialise the block
    //

    int init() {
        fprintf(stderr, "ld_NoResetNest: Creating a reset-surviving simulation\n");

        // Every pointer that refers to memory allocated during init is initialiased
        // with NULL at this place. Potential memory refered by these pointers will be
        // de-allocated in destruct().
        simnest = NULL;
        SimnestName = NULL;

        // user parameters
        int *Uipar;
        double *Urpar;


        try { // check for errors during the configuration

            // Get the irpar parameters Uipar, Urpar
            if (libdyn_AutoConfigureBlock_GetUirpar(block, &Uipar, &Urpar) < 0) throw 1;

            // The number of nested simulations
            int Nsimulations=1;

            // fetch parameters from Uipar, Urpar, throw an exception if something goes wrong
            struct irpar_ivec_t insizes_irp, outsizes_irp, intypes_irp, outtypes_irp, param;

            if ( irpar_get_ivec(&insizes_irp, Uipar, Urpar, 10) < 0 ) throw 1;
            if ( irpar_get_ivec(&outsizes_irp, Uipar, Urpar, 11) < 0 ) throw 1;
            if ( irpar_get_ivec(&intypes_irp, Uipar, Urpar, 12) < 0 ) throw 1;
            if ( irpar_get_ivec(&outtypes_irp, Uipar, Urpar, 13) < 0 ) throw 1;
//     if ( irpar_get_ivec(&param, Uipar, Urpar, 20) < 0 ) error = 1 ;

            // The I/O configuration of the nested simulation
            insizes = insizes_irp.v;
            outsizes = outsizes_irp.v;
            intypes = intypes_irp.v;
            outtypes = outtypes_irp.v;

            Nin = insizes_irp.n;
            Nout = outsizes_irp.n;

            // fetch some strings. Memory will be allocated here
            SimnestName = new irpar_string(Uipar, Urpar, 21); // SimnestName->s->c_str() is the string

            // create a new container for multiple simulations
            simnest = new libdyn_nested2(Nin, Nout, false);
            simnest->set_parent_simulation( block->sim ); // set a parent simulation
            simnest->allocate_slots(Nsimulations);  // tell the expected number of nested simulations

            //
            // set the nested simulations's input signal
            // pointers to the input ports of this block
            //
            {
                int i;
                for (i = 0; i < Nin; ++i) {
                    void *in_p = (void*) libdyn_get_input_ptr(block, i);
                    simnest->cfg_inptr(i, (void*) in_p, insizes[i], intypes[i] );
                }

                // configure the output sizes
                for (i = 0; i < Nout; ++i) {
                    simnest->cfg_output(i, outsizes[i], outtypes[i] );
                }
            }

            // Finished in seting up the nested simulations
            // This must be called before adding any simulation
            simnest->FinishConfiguration();

            // load all schematics/simulations from the given parameter sets
            {
                int i;
                for (i = 0; i < Nsimulations; ++i) {
                    int shematic_id = 900 + i;

                    fprintf(stderr, "ld_NoResetNest: loading shematic id %d\n", shematic_id);
                    if (simnest->add_simulation(-1, Uipar, Urpar, shematic_id) < 0) {
                        throw 4;
                    }
                }
            }

            // set the initial states
            resetStates();

        } catch(int e) { // check if initialisation went fine
            // deallocate all previously allocated memeory in case something went wrong
            fprintf(stderr, "ld_NoResetNest: something went wrong. Exception = %d\n", e);
            destruct();
            return -1; // indicate an error
        }

        return 0; // all went fine
    }


    void destruct()
    {
        // free your allocated memory, ...
//         printf("ld_NoResetNest: destructing\n");

        // destruct the simulation
        if (simnest != NULL) {
            simnest->destruct();
            delete simnest;
        }

        // free allocated parameters
        if (SimnestName != NULL)
            delete SimnestName;
    }

    inline void updateStates()
    {
        int eventmask = 1; // __libdyn_event_get_block_events(block);
        simnest->event_trigger_mask(eventmask);

        // run state update of nested simulation
        simnest->simulation_step(1);
    }

    inline void calcOutputs()
    {
        //
        // calc outputs
        //

        // calc one simulation step for the outputs
        simnest->simulation_step(0);

        // copy the output buffers from the nested simulation to the block's outputs
        int i;
        for (i=0; i< Nout ; ++i) {
            void *out_p = (void*) libdyn_get_output_ptr(block, i);
            simnest->copy_outport_vec(i, out_p);
        }

    }

    inline void resetStates()
    {
        // sim->get_C_SimulationObject()->NestedLevel;
        // Get the simulation nest class instance that contains the simulation containing this block
//         class libdyn_nested2* OuterSimnest = libdyn_nested2::GetSimnestClassPtrFromC(block->sim);

            // increases reset-level counter of the simulation in which this block is contained
            // and forwards the reset event to the nested simulation

        // forward reset events
//          simnest->forward_reset();

    }

    void printInfo() {
        fprintf(stderr, "I'm a ld_NoResetNest block\n");
    }

    // uncommonly used flags
    void PrepareReset() {}
    void HigherLevelResetStates() {}
    void PostInit() {}


    // The Computational function that is called by the simulator
    // that distributes execution to the various functions contained
    // in this C++ - Class, including: init(), io(), resetStates() and the destructor
    static int CompFn(int flag, struct dynlib_block_t *block) {
        // printf("CompFn was called, flag = %d\n", flag); // uncomment to print the currently called flag
        // this expands a template for a C-based computitional function
        return LibdynCompFnTempate<ld_NoResetNest>( flag, block );
    }

    // The data for this block managed by the simulator
    struct dynlib_block_t *block;
};







class ld_CaseSwitchNest {
public:
    ld_CaseSwitchNest(struct dynlib_block_t *block) {
        this->block = block;    // no nothing more here. The real initialisation take place in init()
    }
    ~ld_CaseSwitchNest()
    {
        destruct();
    }


    //
    // define states or other variables
    //

    irpar *param;

    // variables for nesting schematics
    libdyn_nested2 * simnest;
    irpar_string *SimnestName;

    int Nsimulations;
    int Nin_nested;
    int Nout_nested;
    int *insizes_nested, *outsizes_nested;
    int *intypes_nested, *outtypes_nested;

    //
    // initialise the block
    //

    int init() {
        fprintf(stderr, "ld_CaseSwitchNest: Creating\n");

        // Every pointer that refers to memory allocated during init is initialiased
        // with NULL at this place. Potential memory refered by these pointers will be
        // de-allocated in destruct().
        simnest = NULL;
        SimnestName = NULL;

        // user parameters
        int *Uipar;
        double *Urpar;

        // The number of nested simulations
	int Nsimulations;
	
	//
	int dfeed;

        try { // check for errors during the configuration

            // Get the irpar parameters Uipar, Urpar
            if (libdyn_AutoConfigureBlock_GetUirpar(block, &Uipar, &Urpar) < 0) throw 1;


	    
	    // extract gene+ parameters
	    irpar_ivec Par(Uipar, Urpar, 1); // then use:  veccpp.n; veccpp.v;
//             printf("veccpp[0] = %d\n", Par.v[0]); // print the first element

	    Nsimulations = Par.v[0];
	    dfeed = Par.v[1];
	    
	    if (dfeed != 1) {
	      fprintf(stderr, "ld_CaseSwitchNest: dfeed != %t not supported by now...\n");
	      
	      throw 1;
	    }
	    
            // fetch parameters from Uipar, Urpar, throw an exception if something goes wrong
            struct irpar_ivec_t insizes_irp, outsizes_irp, intypes_irp, outtypes_irp, param;

	    
	    if ( irpar_get_ivec(&insizes_irp, Uipar, Urpar, 10) < 0 ) throw 1;
            if ( irpar_get_ivec(&outsizes_irp, Uipar, Urpar, 11) < 0 ) throw 1;
            if ( irpar_get_ivec(&intypes_irp, Uipar, Urpar, 12) < 0 ) throw 1;
            if ( irpar_get_ivec(&outtypes_irp, Uipar, Urpar, 13) < 0 ) throw 1;
//     if ( irpar_get_ivec(&param, Uipar, Urpar, 20) < 0 ) error = 1 ;

            // The I/O configuration of the nested simulation
            insizes_nested = insizes_irp.v;
            outsizes_nested = outsizes_irp.v;
            intypes_nested = intypes_irp.v;
            outtypes_nested = outtypes_irp.v;

            Nin_nested = insizes_irp.n;
            Nout_nested = outsizes_irp.n;

            // fetch some strings. Memory will be allocated here
            SimnestName = new irpar_string(Uipar, Urpar, 21); // SimnestName->s->c_str() is the string

            // create a new container for multiple simulations
            simnest = new libdyn_nested2(Nin_nested, Nout_nested, false);
            simnest->set_parent_simulation( block->sim ); // set a parent simulation
            simnest->allocate_slots(Nsimulations);  // tell the expected number of nested simulations

            //
            // set the nested simulations's input signal
            // pointers to the input ports of this block
            //
            {
                int i;
                for (i = 0; i < Nin_nested; ++i) {
                    void *in_p = (void*) libdyn_get_input_ptr(block, i);
                    simnest->cfg_inptr(i, (void*) in_p, insizes_nested[i], intypes_nested[i] );
                }

                // configure the output sizes
                for (i = 0; i < Nout_nested; ++i) {
                    simnest->cfg_output(i, outsizes_nested[i], outtypes_nested[i] );
                }
            }

            // Finished in seting up the nested simulations
            // This must be called before adding any simulation
            simnest->FinishConfiguration();

            // load all schematics/simulations from the given parameter sets
            {
                int i;
                for (i = 0; i < Nsimulations; ++i) {
                    int shematic_id = 900 + i;

                    fprintf(stderr, "ld_CaseSwitchNest: loading shematic id %d\n", shematic_id);
                    if (simnest->add_simulation(-1, Uipar, Urpar, shematic_id) < 0) {
                        throw 4;
                    }
                }
            }

            // set the initial states
            resetStates();

        } catch(int e) { // check if initialisation went fine
            // deallocate all previously allocated memeory in case something went wrong
            fprintf(stderr, "ld_CaseSwitchNest: something went wrong. Exception = %d\n", e);
            destruct();
            return -1; // indicate an error
        }

        return 0; // all went fine
    }


    void destruct()
    {
        // free your allocated memory, ...
//         printf("ld_CaseSwitchNest: destructing\n");

        // destruct the simulation
        if (simnest != NULL) {
            simnest->destruct();
            delete simnest;
        }

        // free allocated parameters
        if (SimnestName != NULL)
            delete SimnestName;
    }

    inline void updateStates()
    {
        int eventmask = 1; // __libdyn_event_get_block_events(block);
        simnest->event_trigger_mask(eventmask);

        // run state update of nested simulation
        simnest->simulation_step(1);
    }

    inline void calcOutputs()
    {
        //
        // calc outputs
        //

//         uint32_t *SelectSignal = (uint32_t*) libdyn_get_output_ptr(block, Nin_nested); // the first addiotnonal input to this block
        uint32_t *SelectSignal = (uint32_t*) libdyn_get_input_ptr(block, Nin_nested ); // the first addiotnonal input to this block
      
//         double *test = (double*) libdyn_get_input_ptr(block, 0 ); // the first addiotnonal input to this block
// 	printf("SelectSignal = %d , in1=%f\n", *SelectSignal, *test);
      
	simnest->set_current_simulation( *SelectSignal - 1 ); // "-1": shift counter start from 1 to 0
	
        // calc one simulation step
        simnest->simulation_step(0);
        simnest->simulation_step(1);

        // copy the output buffers from the nested simulation to the block's outputs
        int i;
        for (i=0; i< Nout_nested ; ++i) {
            void *out_p = (void*) libdyn_get_output_ptr(block, i);
            simnest->copy_outport_vec(i, out_p);
        }

    }

    inline void resetStates()
    {
        // sim->get_C_SimulationObject()->NestedLevel;
        // Get the simulation nest class instance that contains the simulation containing this block
//         class libdyn_nested2* OuterSimnest = libdyn_nested2::GetSimnestClassPtrFromC(block->sim);

            // increases reset-level counter of the simulation in which this block is contained
            // and forwards the reset event to the nested simulation

        // forward reset events
//          simnest->forward_reset();

    }

    void printInfo() {
        fprintf(stderr, "I'm a ld_CaseSwitchNest block\n");
    }

    // uncommonly used flags
    void PrepareReset() {}
    void HigherLevelResetStates() {}
    void PostInit() {}


    // The Computational function that is called by the simulator
    // that distributes execution to the various functions contained
    // in this C++ - Class, including: init(), io(), resetStates() and the destructor
    static int CompFn(int flag, struct dynlib_block_t *block) {
        // printf("CompFn was called, flag = %d\n", flag); // uncomment to print the currently called flag
        // this expands a template for a C-based computitional function
        return LibdynCompFnTempate<ld_CaseSwitchNest>( flag, block );
    }

    // The data for this block managed by the simulator
    struct dynlib_block_t *block;
};





/*
    New implementation of the state machine
*/











// External block comp functions
extern "C" {
    extern int compu_func_nested_exchange_fromfile(int flag, struct dynlib_block_t *block);

    // persistent_memory.cpp
    extern int persistent_memory_block(int flag, struct dynlib_block_t *block);
    extern int write_persistent_memory_block(int flag, struct dynlib_block_t *block);
    extern int read_persistent_memory_block(int flag, struct dynlib_block_t *block);
    extern int write2_persistent_memory_block(int flag, struct dynlib_block_t *block);
};

int libdyn_module_nested_siminit(struct dynlib_simulation_t *sim, int bid_ofs)
{

    // Register my blocks to the given simulation

    int blockid = 15001;
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &compu_func_nested);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid+1, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &compu_func_statemachine);

    libdyn_compfnlist_add(sim->private_comp_func_list, blockid+2, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &compu_func_nested_exchange_fromfile);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid+3, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &compu_func_survivereset);

    libdyn_compfnlist_add(sim->private_comp_func_list, blockid+4, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &persistent_memory_block);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid+5, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &write_persistent_memory_block);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid+6, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &read_persistent_memory_block);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid+7, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &write2_persistent_memory_block);

    
    
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid+10, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &async_simulationBlock::CompFn);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid+11, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &ld_NoResetNest::CompFn);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid+12, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &ld_CaseSwitchNest::CompFn);




#ifdef DEBUG
    fprintf(stderr, "libdyn module nested initialised\n");
#endif  


}
