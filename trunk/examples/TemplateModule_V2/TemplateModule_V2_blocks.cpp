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



/*

  This is a template for creating a new ORTD Block's computational function in C++

  Do a search of "Template" and replace it with your own name.

*/



// TODO: Migrate to LibdynCompFnTempate2

class TemplateBlock {
    // The simplest form of a block
public:
    TemplateBlock(struct dynlib_block_t *block) {
        this->block = block;    // no nothing more here. The real initialisation take place in init()
    }
    ~TemplateBlock()
    {
        destruct();
    }


    //
    // define states or other variables
    //

    double z0; // a state

    //
    // initialise your block
    //


    // variables that point to allocated memot
    irpar_string *s;
    irpar_ivec *Array;

    void destruct()
    {
        // free your during init() allocated memory, ...

        if (s!=NULL) delete s;
        if (Array!=NULL) delete Array;
    }

    int init() {
        int *Uipar;
        double *Urpar;

        try {
            // Get the irpar parameters Uipar, Urpar
            libdyn_AutoConfigureBlock_GetUirpar(block, &Uipar, &Urpar);

            // init all pointers with NULL
            s = NULL;
            Array = NULL;

            //
            // extract some structured sample parameters
            //

            //
            // get a string
            //

            // cpp version (nicer), an exception is thrown in case something goes wrong
            s = new irpar_string(Uipar, Urpar, 12);
            printf("cppstr = %s\n", s->s->c_str());


            //
            // get a vector of integers (double vectors are similar, replace ivec with rvec)
            //

            // c++ version (nicer), an exception is thrown in case something goes wrong

            Array = new irpar_ivec(Uipar, Urpar, 11); // then use:  veccpp.n; veccpp.v;
            printf("veccpp[0] = %d\n", Array->v[0]); // print the first element
            // of the vector that is of size veccpp.n



            //
            // get some informations on the first input port
            //
            int port = 0; // 1st
            int N = libdyn_get_inportsize(block, port);  // the size of the first (=0) input vector
            int datatype = libdyn_get_inportdatatype(block, port); // the datatype
            int TypeBytes = libdyn_config_get_datatype_len(datatype); // number of bytes allocated for one element of type "datatype"
            int NBytes = N * TypeBytes;  // Amount of bytes allocated for the input vector

            // set the initial states
            resetStates();


        } catch(int e) { // check if initialisation went fine
            // deallocate all previously allocated memeory in case something went wrong
            fprintf(stderr, "TemplateBlock: something went wrong. Exception = %d\n", e);
            destruct(); // free all memeory allocated by now.
            return -1; // indicate an error
        }

        // Return 0 to indicate that there was no error
        return 0;
    }


    inline void updateStates()
    {
        double *in1 = (double *) libdyn_get_input_ptr(block,0); // the first input port
        double *in2 = (double *) libdyn_get_input_ptr(block,1); // the 2nd input port
        double *output = (double*) libdyn_get_output_ptr(block, 0); // the first output port

        z0 = *in1 * 2;
    }


    inline void calcOutputs()
    {
        double *in1 = (double *) libdyn_get_input_ptr(block,0); // the first input port
        double *in2 = (double *) libdyn_get_input_ptr(block,1); // the 2nd input port
        double *output = (double*) libdyn_get_output_ptr(block, 0); // the first output port

        //   printf("output calc\n");
        *output = z0;
    }


    inline void resetStates()
    {
        z0 = 0; // return to the blocks initial state
    }



    void printInfo() {
        fprintf(stderr, "I'm a Template block\n");
    }

    // uncommonly used flags
    void PrepareReset() {}
    void HigherLevelResetStates() {}
    void PostInit() {}


    // The Computational function that is called by the simulator
    // and that distributes the execution to the various functions
    // in this C++ - Class, including: init(), io(), resetStates() and the destructor
    static int CompFn(int flag, struct dynlib_block_t *block) {
        return LibdynCompFnTempate<TemplateBlock>( flag, block ); // this expands a template for a C-comp fn
    }

    // The data for this block managed by the simulator
    struct dynlib_block_t *block;
};







class SynchronisingTemplateBlock {
  // This block syncronises schematics. Only one block of this type is allowed in a threaded sub-schematic
  // created e.g. by ld_async_simulation
  
public:
    SynchronisingTemplateBlock(struct dynlib_block_t *block) {
        this->block = block;    // no nothing more here. The real initialisation take place in init()
    }
    ~SynchronisingTemplateBlock()
    {
        // free your allocated memory, ...
      destruct();
    }

    //
    // define states or other variables
    //

    double z0; // a state
    bool ExitLoop;
    double SensorValue;

    // variables that point to allocated memot
    irpar_string *s;
    irpar_ivec *Array;

    void destruct()
    {
        // free your during init() allocated memory, ...

        if (s!=NULL) delete s;
        if (Array!=NULL) delete Array;
    }

    
    //
    // initialise your block
    //

    int init() {
        int *Uipar;
        double *Urpar;
	

	try {
        // Get the irpar parameters Uipar, Urpar
        libdyn_AutoConfigureBlock_GetUirpar(block, &Uipar, &Urpar);

            // init all pointers with NULL
            s = NULL;
            Array = NULL;

            //
            // extract some structured sample parameters
            //

            //
            // get a string
            //

            // cpp version (nicer), an exception is thrown in case something goes wrong
            s = new irpar_string(Uipar, Urpar, 12);
            printf("cppstr = %s\n", s->s->c_str());


            //
            // get a vector of integers (double vectors are similar, replace ivec with rvec)
            //

            // c++ version (nicer), an exception is thrown in case something goes wrong

            Array = new irpar_ivec(Uipar, Urpar, 11); // then use:  veccpp.n; veccpp.v;
            printf("veccpp[0] = %d\n", Array->v[0]); // print the first element
            // of the vector that is of size veccpp.n


        // set the initial states
        resetStates();

        // register the callback function to the simulator that shall trigger the simulation while running in a loop
        libdyn_simulation_setSyncCallback(block->sim, &syncCallback_ , this);
        libdyn_simulation_setSyncCallbackDestructor(block->sim, &syncCallbackDestructor_ , this);

        ExitLoop = false;
	
        } catch(int e) { // check if initialisation went fine
            // deallocate all previously allocated memeory in case something went wrong
            fprintf(stderr, "TemplateBlock: something went wrong. Exception = %d\n", e);
            destruct(); // free all memeory allocated by now.
            return -1; // indicate an error
        }
        // All ok
        return 0;
    }


    inline void updateStates()
    {
        double *in1 = (double *) libdyn_get_input_ptr(block,0); // the first input port
        double *in2 = (double *) libdyn_get_input_ptr(block,1); // the 2nd input port
        double *output = (double*) libdyn_get_output_ptr(block, 0); // the first output port

    }


    inline void calcOutputs()
    {
        double *in1 = (double *) libdyn_get_input_ptr(block,0); // the first input port
        double *in2 = (double *) libdyn_get_input_ptr(block,1); // the 2nd input port
        double *output = (double*) libdyn_get_output_ptr(block, 0); // the first output port

        // Put out the sensor value
        *output = SensorValue;
        //   printf("output calc\n");

    }


    inline void resetStates()
    {
        z0 = 0; // return to the blocks initial state
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

        printf("Threaded simulation started execution\n");

//     // wait until the callback function  real_syncDestructor_callback is called
//     pthread_mutex_lock(&ExitMutex);
//

        // This is the main loop of the new simulation
        while (!ExitLoop) {
            sleep(1); // wait for dummy sensor
            SensorValue = 1.234;

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
        libdyn_simulation_SyncCallback_terminateThread(block->sim, 1); // force termination of the thread. Only a dummy call by now.
//     pthread_mutex_unlock(&ExitMutex);

    }


    void printInfo() {
        fprintf(stderr, "I'm a Template block\n");
    }

    // uncommonly used flags
    void PrepareReset() {}
    void HigherLevelResetStates() {}
    void PostInit() {}


    // The Computational function that is called by the simulator
    // and that distributes the execution to the various functions
    // in this C++ - Class, including: init(), io(), resetStates() and the destructor
    static int CompFn(int flag, struct dynlib_block_t *block) {
        return LibdynCompFnTempate<SynchronisingTemplateBlock>( flag, block ); // this expands a template for a C-comp fn
    }
    static int syncCallback_(struct dynlib_simulation_t * sim) {
        void * obj = sim->sync_callback.userdat;
        SynchronisingTemplateBlock *p = (SynchronisingTemplateBlock *) obj;
        return p->SyncCallback(sim);
    }
    static int syncCallbackDestructor_(struct dynlib_simulation_t * sim) {
        void * obj = sim->sync_callback.userdatDestructor;
        SynchronisingTemplateBlock *p = (SynchronisingTemplateBlock *) obj;
        return p->SyncDestructorCallback(sim);
    }

    // The data for this block managed by the simulator
    struct dynlib_block_t *block;
};












/*
 * A template for using shared objects
*/


#include "global_shared_object.h"
#include "directory.h"


//
// An object that whose instances are shared between multiple blocks
// . This is executed by including the block ld_Template_shObj
//
class Template_SharedObject : public ortd_global_shared_object {
public:
    Template_SharedObject(const char* identName, dynlib_block_t *block, int Visibility) : ortd_global_shared_object(identName, block->sim, Visibility) {
        this->block = block;
    }
    ~Template_SharedObject() {
        destruct();
    }

    int init() {
        irpar_string *Fname;

        try {
            // Get the irpar parameters Uipar, Urpar
            libdyn_AutoConfigureBlock_GetUirpar(block, &Uipar, &Urpar);

            // cpp version (nicer), an exception is thrown in case something goes wrong
            Fname = new irpar_string(Uipar, Urpar, 12);

        } catch(int e) { // check if initialisation went fine
            // deallocate all previously allocated memeory in case something went wrong
            fprintf(stderr, "TemplateBlock: something went wrong while fetching the parameters. Exception = %d\n", e);
            return -1; // indicate an error
        }


        // Open e.g. a device using Fname->s->c_str() as the device name

        // open...
        fprintf(stderr, "Open device: %s\n", Fname->s->c_str() );

//         FD = open(Fname->s->c_str(), O_RDWR);
//         if (FD < 0) {
//             return -1;
//         }

        // Return 0 to indicate that there was no error
        return 0;
    }

    void destruct()
    {
        // e.g. close the device

//         close(FD);
    }

    // A function that can be called by blocks using this shared object
    double doSomething(double val) {
        printf("This is printed by a function in a shared object that is called by a block using this shared object\n");
	
	// e.g. read from / write to the device
	
        return val + 1.2;
    }

private:
    int *Uipar;
    double *Urpar;
    dynlib_block_t *block;

    //
    //   data of the shared object
    //

    // e.g. a file descriptor
    int FD;
};




//
// An example block for accessing the shared object
//
class Template_AccessShObjBlock {
public:
    Template_AccessShObjBlock(struct dynlib_block_t *block) {
        this->block = block;    // no nothing more here. The real initialisation take place in init()
    }
    ~Template_AccessShObjBlock()
    {
        // free your allocated memory, ...
    }

    //
    // define states or other variables
    //

    Template_SharedObject *IShObj; // instance of the shared object

    double z0; // a state

    //
    // initialise your block
    //

    int init() {
        int *Uipar;
        double *Urpar;

        try {

            // Get the irpar parameters Uipar, Urpar
            libdyn_AutoConfigureBlock_GetUirpar(block, &Uipar, &Urpar);

            //
            // extract some structured sample parameters
            //

            // Obtain the shared object's instance
            if ( ortd_GetSharedObj<Template_SharedObject>(block, &IShObj) < 0 ) {
                throw 1;
            }

        } catch(int e) { // check if initialisation went fine
            // deallocate all previously allocated memeory in case something went wrong
            fprintf(stderr, "TemplateBlock: something went wrong. Exception = %d\n", e);
            return -1; // indicate an error
        }

        // set the initial states
        resetStates();


        // Return 0 to indicate that there was no error
        return 0;
    }


    inline void updateStates()
    {
        double *in1 = (double *) libdyn_get_input_ptr(block,0); // the first input port
        double *output = (double*) libdyn_get_output_ptr(block, 0); // the first output port

        z0 = *in1 * 2;
    }

    inline void calcOutputs()
    {
        double *in1 = (double *) libdyn_get_input_ptr(block,0); // the first input port
        double *output = (double*) libdyn_get_output_ptr(block, 0); // the first output port

        // call a function of the shared object
        double val = IShObj->doSomething(in1[0]);

        //   printf("output calc\n");
        *output = *in1 + val;
    }

    inline void resetStates()
    {
        z0 = 0; // return to the blocks initial state
    }



    void printInfo() {
        fprintf(stderr, "I'm a Template block\n");
    }

    // uncommonly used flags
    void PrepareReset() {}
    void HigherLevelResetStates() {}
    void PostInit() {}


    // The Computational function that is called by the simulator
    // and that distributes the execution to the various functions
    // in this C++ - Class, including: init(), io(), resetStates() and the destructor
    static int CompFn(int flag, struct dynlib_block_t *block) {
        return LibdynCompFnTempate<Template_AccessShObjBlock>( flag, block ); // this expands a template for a C-comp fn
    }

    // The data for this block managed by the simulator
    struct dynlib_block_t *block;
};










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
//              The string "TemplateModule_V2" must be exchanged
    int libdyn_module_TemplateModule_V2_siminit(struct dynlib_simulation_t *sim, int bid_ofs)
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
        libdyn_compfnlist_add(sim->private_comp_func_list, blockid, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &TemplateBlock::CompFn);

        // synchronising block
        libdyn_compfnlist_add(sim->private_comp_func_list, blockid+1, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &SynchronisingTemplateBlock::CompFn);

        // shared object initialisation block
        libdyn_compfnlist_add(sim->private_comp_func_list, blockid+10, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &SharedObjBlock<Template_SharedObject>::CompFn);

        // shared object accessing block
        libdyn_compfnlist_add(sim->private_comp_func_list, blockid+11, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &Template_AccessShObjBlock::CompFn);


        printf("module Template is initialised\n");

    }


}
