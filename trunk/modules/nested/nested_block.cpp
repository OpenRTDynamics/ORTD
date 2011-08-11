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

#include "libdyn_scicos_macros.h"
#include "irpar.h"
}

#include "libdyn_cpp.h"





extern "C" {
    int libdyn_module_nested_siminit(struct dynlib_simulation_t *sim, int bid_ofs);
}

template <class compute_instance> class background_computation {
public:
  
    // the class compute_instance should provide a method int computer()
    background_computation(compute_instance *ci);
    ~background_computation();

    bool start_computation();

    void join_computation();

    bool get_finished();
    void set_finished();

    void loop();

private:

    compute_instance * ci;
  
    int signal;

    pthread_mutex_t mutex;
    pthread_cond_t cond;

    pthread_t thread;

    bool finished;
    pthread_mutex_t finished_mutex;

    pthread_mutex_t comp_active_mutex;


    static void * start_thread(void *obj);
};



// void * background_computation_thread(void *data)
// {
//     background_computation * inst = (background_computation *) data;
// 
//     printf("comp thread started\n");
// 
//     inst->loop();
// 
//     pthread_exit(NULL);
// }

template <class compute_instance> void * background_computation<compute_instance>::start_thread(void *obj)
{
    background_computation<compute_instance> *ci;
  
    ci = (background_computation<compute_instance> *) obj;
    
    ci->loop();
//     reinterpret_cast<compute_instance *>(obj)->loop();
}

template <class compute_instance> void background_computation<compute_instance>::loop()
// void background_computation::loop()
{
    for (;;) {

        pthread_mutex_lock(&mutex);

        while (signal == 0) { // FIXME do { .. } while() ???
            pthread_cond_wait(&cond, &mutex);
        }
        int sig = signal;
	signal = 0;

        pthread_mutex_unlock(&mutex);


        // do something based on sig
        if (sig == 1) {

            pthread_mutex_lock(&comp_active_mutex);

            // Run the function computer() of the given class, which type is parametrised by this template
	    ci->computer();

            pthread_mutex_unlock(&comp_active_mutex);


// 	     computation finished
            set_finished();
        }

        if (sig == -1)
            return;

    }
}

template <class compute_instance> bool background_computation<compute_instance>::start_computation()
// bool background_computation::start_computation()
{

    bool finished_cpy = get_finished();

//   only if computation is not running
    if (finished_cpy == true) {

        signal = 1;
        pthread_cond_signal(&cond); // Notify reader thread
    }

}

// void background_computation::join_computation()
template <class compute_instance> void background_computation<compute_instance>::join_computation()
{
    // wait until computation is complete
//    TODO
}

template <class compute_instance> bool background_computation<compute_instance>::get_finished()
// bool background_computation::get_finished()
{

    pthread_mutex_lock(&finished_mutex);
    bool finished_cpy = finished;
    pthread_mutex_unlock(&finished_mutex);

    return finished_cpy;
}

template <class compute_instance> void background_computation<compute_instance>::set_finished()
// void background_computation::set_finished()
{
    pthread_mutex_lock(&finished_mutex);
    finished = true;
    pthread_mutex_unlock(&finished_mutex);
}


template <class compute_instance> background_computation<compute_instance>::background_computation(compute_instance *ci)
// background_computation::background_computation(compute_instance *ci)
{
    this->ci = ci;
    signal = 0;
    finished = true;

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    pthread_mutex_init(&finished_mutex, NULL);
    pthread_mutex_init(&comp_active_mutex, NULL);

//     int rc = pthread_create(&thread, NULL, background_computation_thread, (void *) this);
     int rc = pthread_create(&thread, NULL, &background_computation<compute_instance>::start_thread, (void *) this);
     if (rc) {
         printf("ERROR; return code from pthread_create() is %d\n", rc);
     }


}


template <class compute_instance> background_computation<compute_instance>::~background_computation()
// background_computation::~background_computation()
{
    signal = -1;
    pthread_cond_signal(&cond); // Notify reader thread

//     printf("comp: joining\n");
    pthread_join(thread, NULL);
    printf("background computer: joinined\n");


    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    pthread_mutex_destroy(&finished_mutex);
    pthread_mutex_destroy(&comp_active_mutex);

}





/*
class ortd_asychronous_computation_thread {
  public:
    
    
  private:
    
  
};*/


template <class callback_class> class ortd_asychronous_computation {
private:

    libdyn_nested * simnest;
    libdyn *sim;

    callback_class *cb;
    
    background_computation< ortd_asychronous_computation > *computer_mgr;
public:
    // simnest: a ready to use set-up schematic
    ortd_asychronous_computation(callback_class *cb, libdyn_nested * simnest);
    ortd_asychronous_computation(callback_class *cb, libdyn * simnest);

    ~ortd_asychronous_computation();

    int computeNSteps(int N);
    bool computation_finished();
    void join_computation();
    
    
    int computer();
};


template <class callback_class> ortd_asychronous_computation<callback_class>::ortd_asychronous_computation(callback_class *cb, libdyn_nested * simnest)
//ortd_asychronous_computation::ortd_asychronous_computation(libdyn_nested* simnest)
{
    this->simnest = simnest;
    this->sim = simnest->current_sim;

    computer_mgr = new background_computation< ortd_asychronous_computation > (this);

    computer_mgr->start_computation();
}

template <class callback_class> ortd_asychronous_computation<callback_class>::~ortd_asychronous_computation()
// ortd_asychronous_computation::~ortd_asychronous_computation()
{
    delete computer_mgr;
}


template <class callback_class> int ortd_asychronous_computation<callback_class>::computeNSteps(int N)
// int ortd_asychronous_computation::computeNSteps(int N)
{
    // trigger computation

    computer_mgr->start_computation();

}

template <class callback_class> bool ortd_asychronous_computation<callback_class>::computation_finished()
// bool ortd_asychronous_computation::computation_finished()
{
  computer_mgr->get_finished();
}

template <class callback_class> int ortd_asychronous_computation<callback_class>::computer()
// int ortd_asychronous_computation::computer()
{
  
  printf("running computer\n");
  
  sim->simulation_step(0);
  sim->simulation_step(1);

  
//   	  for (i=0; i< Nout ; ++i) {
// 	      double *out_p = (double*) libdyn_get_output_ptr(block, i);
// 	      simnest->copy_outport_vec(i, out_p);
// 
//   //       printf("nested outp %f\n", out_p[0]);
// 	  }

  cb->async_copy_output_callback();
  
//   sleep(1);
  printf("finished\n");
}










class compu_func_nested_class {
public:
    compu_func_nested_class(struct dynlib_block_t *block);
    void destruct();
    void io(int update_states);
    void io_sync(int update_states);
    void io_async(int update_states);

    void async_copy_output_callback();
    
    int init();
private:
    struct dynlib_block_t *block;

    // Used when asyn computation is desired
    bool async_comp;
    ortd_asychronous_computation<compu_func_nested_class> * async_comp_mgr;

    libdyn_nested * simnest;
    libdyn_master * master;
    irpar *param;

    int Nin;
    int Nout;
    int *insizes, *outsizes;
    int *intypes, *outtypes;

    int Nsimulations, dfeed, synchron_simsteps;

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
        printf("nested: could not get parameter from irpar set\n");
        return -1;
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
    synchron_simsteps = param.v[2];

    /*



      */


    bool use_buffered_input = false;  // in- and out port values are not buffered (default)
    if (synchron_simsteps < 0) {
        use_buffered_input == true;  // with async computation buffered inputs have to be used
    }

    simnest = new libdyn_nested(Nin, insizes, intypes, Nout, outsizes, outtypes, use_buffered_input);
    simnest->allocate_slots(Nsimulations);

    //
    // set pointers to the input ports of this block
    //
    for (i = 0; i < Nin; ++i) {
        double *in_p = (double*) libdyn_get_input_ptr(block, i);
        simnest->cfg_inptr(i, (void*) in_p);
    }


    // If there is a libdyn master : use it
    master = (libdyn_master *) block->sim->master;
    if (master == NULL) {  // no master available
//       fprintf(stderr, "WARNING: libdyn: parameter block requires a libdyn master\n");
    }

    simnest->set_master(master);

    for (simCreateCount = 0; simCreateCount < Nsimulations; ++simCreateCount) {
        int shematic_id = 900 + simCreateCount;

        printf("loading shematic id %d\n", shematic_id);
        if (simnest->add_simulation(ipar, rpar, shematic_id) < 0) {
            goto destruct_simulations;  // An error
        }

        printf("added schematic\n");
    }


    //
    // start async computation manager if desired
    //

    if (synchron_simsteps < 0) {
        this->async_comp = false;
    } else {
        // Initialise async computation manager
        printf("nested: Using async\n");
        
        this->async_comp = true;
        this->async_comp_mgr = new ortd_asychronous_computation<compu_func_nested_class>(this, simnest);
    }


    return 0;

destruct_simulations:
    printf("nested: Destructing all simulations\n");
    for (i = 0; i < simCreateCount; ++i) {

    }
    ;
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

        simnest->simulation_step(1);

        //        printf("neszed sup\n");
    } else {
        //
        // calc outputs
        //


        simnest->simulation_step(0);

        for (i=0; i< Nout ; ++i) {
            double *out_p = (double*) libdyn_get_output_ptr(block, i);
            simnest->copy_outport_vec(i, out_p);

//       printf("nested outp %f\n", out_p[0]);
        }

        // Some control signal input analysis
        double *switch_inp = (double*) libdyn_get_input_ptr(block, Nin+0);
        double *reset_inp = (double*) libdyn_get_input_ptr(block, Nin+1);

        if (*reset_inp > 0) {
            printf("reset states\n");
            simnest->reset_blocks();
        }

        int nSim = *switch_inp;
//     printf("switch sig=%f reset=%f sw=%d\n", *switch_inp, *reset_inp, nSim);
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

    if (update_states == 1) {
        //
        // update states
        //

       this->async_comp_mgr->computeNSteps(10);

//         // Some control signal input analysis
//         double *switch_inp = (double*) libdyn_get_input_ptr(block, Nin+0);
//         double *reset_inp = (double*) libdyn_get_input_ptr(block, Nin+1);
// 
//         if (*reset_inp > 0) {
//             printf("reset states\n");
//             simnest->reset_blocks();
//         }


    } else {
        //
        // copy outputs (if result is available)
        //


       if (this->async_comp_mgr->computation_finished()) {

	  for (i=0; i< Nout ; ++i) {
	      double *out_p = (double*) libdyn_get_output_ptr(block, i);
	      simnest->copy_outport_vec(i, out_p);

  //       printf("nested outp %f\n", out_p[0]);
	  }
	  
       }


    }


}

void compu_func_nested_class::async_copy_output_callback()
{
  printf("copy outputs\n ");
}



void compu_func_nested_class::io(int update_states)
{
    if (async_comp == false) {
        io_sync(update_states);
    } else {
        io_async(update_states);
    }

}

void compu_func_nested_class::destruct()
{

    if (this->async_comp)
        delete async_comp_mgr;

    simnest->destruct();
    delete simnest;
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


        int i;
        libdyn_config_block(block, BLOCKTYPE_DYNAMIC, Noutports, Ninports + 2, (void *) 0, 0);

        for (i = 0; i < Ninports; ++i) {
            libdyn_config_block_input(block, i, insizes.v[i], DATATYPE_FLOAT);
        }

        libdyn_config_block_input(block, Ninports, 1, DATATYPE_FLOAT); // switch signal input
        libdyn_config_block_input(block, Ninports+1, 1, DATATYPE_FLOAT); // reset signal input

        for (i = 0; i < Noutports; ++i) {
            libdyn_config_block_output(block, i, outsizes.v[i], DATATYPE_FLOAT, dfeed);
        }


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
        compu_func_nested_class *worker = (compu_func_nested_class *) libdyn_get_work_ptr(block);

        worker->destruct();
        delete worker;

    }
    return 0;
    break;
    case COMPF_FLAG_PRINTINFO:
        printf("I'm a nested block\n");
        return 0;
        break;

    }
}

//#include "block_lookup.h"

int libdyn_module_nested_siminit(struct dynlib_simulation_t *sim, int bid_ofs)
{

    // Register my blocks to the given simulation

    int blockid = 15001;
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &compu_func_nested);

    printf("libdyn module nested initialised\n");

}


//} // extern "C"
