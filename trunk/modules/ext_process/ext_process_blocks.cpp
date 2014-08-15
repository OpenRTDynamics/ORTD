/*
    Copyright (C) 2010, 2011  Christian Klauer

    This file is part of OpenRTDynamics, the Real Time Dynamics Framework

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
#include <string>

extern "C" {
#include "libdyn.h"
#include "libdyn_scicos_macros.h"
#include "irpar.h"
}



#include "libdyn_cpp.h"
#include "global_shared_object.h"
#include "directory.h"
#include "ortd_fork.h"





//
// A shared object
//
class Process_SharedObject : public ortd_global_shared_object {
public:
    Process_SharedObject(const char* identName, dynlib_block_t *block, int Visibility) : ortd_global_shared_object(identName, block->sim, Visibility) {
        this->block = block;

        // Get the irpar parameters Uipar, Urpar
        libdyn_AutoConfigureBlock_GetUirpar(block, &Uipar, &Urpar);


    }

    ~Process_SharedObject() {

    }

    int init() {
        fprintf(stderr, "Init of Process_SharedObject\n");


        // parlist = new_irparam_elemet_ivec(parlist, ascii(executable), 1); // id = 11; A string parameter

        irpar_string executable(Uipar, Urpar, 1);
        printf("executable = %s\n", executable.s->c_str());


        proc = new ortd_fork((char*)executable.s->c_str(), ".", 0, true);
// 	proc = new ortd_fork("./ProcWithUI.sh", ".", 0, true);

	
	// irpar id 10 contains the number of Options
	
	irpar_string Option1(Uipar, Urpar, 11);
	proc->addCommandlineOption( (char*) Option1.s->c_str() );
	
	
        proc->init();

        return 0; // init was ok
    }

    void send(void *buf, uint N) {
        FILE *fd = proc->get_writefd();

        fwrite(buf, 1, N, fd);
        fflush(fd);

        if (feof(fd) != 0) {
            fprintf(stderr, "Process_SharedObject: EOF in send\n");
            return;
        }

        if (ferror(fd) != 0) {
            fprintf(stderr, "Process_SharedObject: ERROR in send\n");
            return;
        }

    }

    int ReceivePacket(void *buf, uint32_t ExpectedBytes) {
        // Block until the sub-process sends something through its stdout
        FILE *fd = proc->get_readfd();

//         printf("ExpectedBytes=%d\n", ExpectedBytes);

        uint32_t Nread = fread(buf, ExpectedBytes, 1, fd);
//         printf("Got=%d Bytes\n", Nread);

        if (feof(fd) != 0) {
//             printf("EOF\n");
            return -1;
        }

        if (ferror(fd) != 0) {
//             printf("ERROR\n");
            return -1;
        }


        // ((char*) buf)[7] = '\0';

        // printf("Got: %s\n", (char*) buf);

// 	    exit(0);

        return Nread;
    }

    int *Uipar;
    double *Urpar;
    dynlib_block_t *block;


    /*
        Some data of the shared object
    */
private:
    ortd_fork *proc;

public:



};



/*
 * from Process-Receiver
*/


class ProcessReceiverBlock {
public:
    ProcessReceiverBlock(struct dynlib_block_t *block) {
        this->block = block;    // no nothing more here. The real initialisation take place in init()
    }
    ~ProcessReceiverBlock()
    {
        // free your allocated memory, ...
    }

    //
    // define states or other variables
    //

    bool ExitLoop;
    int NMaxCopyBytes;
    Process_SharedObject *IShObj; // instance of the shared object

    //
    // initialise your block
    //

    int init() {
        int *Uipar;
        double *Urpar;

        fprintf(stderr, "Process-Read init\n");

        // Get the irpar parameters Uipar, Urpar
        libdyn_AutoConfigureBlock_GetUirpar(block, &Uipar, &Urpar);


        // register the callback function to the simulator that shall trigger the simulation while running in a loop
        libdyn_simulation_setSyncCallback(block->sim, &syncCallback_ , this);
        libdyn_simulation_setSyncCallbackDestructor(block->sim, &syncCallbackDestructor_ , this);

        ExitLoop = false;


        // Get the sh object for the process
        IShObj = NULL;
        if ( ortd_GetSharedObj<Process_SharedObject>(block, &IShObj) < 0 ) {
            return -1;
        }


        // set the initial states
        resetStates();

        // calc how many bytes shall be copied
        int N = libdyn_get_outportsize(block, 0);
        int datatype = libdyn_get_outportdatatype(block, 0);
        int TypeBytes = libdyn_config_get_datatype_len(datatype);
        NMaxCopyBytes = N * TypeBytes;

// 	printf("****************** UDP-Read init fin\n");


        // Return -1 to indicate an error, so the simulation will be destructed
        return 0;
    }


    inline void updateStates()
    {
    }


    inline void calcOutputs()
    {
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

        fprintf(stderr, "Threaded simulation synchronised to an Process-receiver started execution\n");

//     // wait until the callback function  real_syncDestructor_callback is called
//     pthread_mutex_lock(&ExitMutex);
//

        uint32_t *ExpectedBytes = (uint32_t*) libdyn_get_input_ptr(block, 0);
        uint32_t *NumBytes = (uint32_t*) libdyn_get_output_ptr(block, 1);
        void *output = (void *) libdyn_get_output_ptr(block, 0); // the first output port

        // This is the main loop of the new simulation
        while (!ExitLoop) {
//           printf("wait for UDP Data to receive\n");
//             sleep(1); // wait for dummy sensor

            if (*ExpectedBytes > NMaxCopyBytes) {
                fprintf(stderr, "ProcessReceiverBlock: requested to read more bytes than resvered by the output\n");
                *ExpectedBytes = NMaxCopyBytes;
            }

            int Ndata = IShObj->ReceivePacket ( output, *ExpectedBytes );

            *NumBytes = Ndata;

// 	    printf(">> %s\n", output);
            //  ((double*) output)[0] = 12.2435;

            if ( Ndata >= 0 ) {
                // run one step of the ortd simulator
                libdyn_event_trigger_mask(sim, 1);
                libdyn_simulation_step(sim, 0);
                libdyn_simulation_step(sim, 1);
            } else {
                ExitLoop = true;
            }
        }


        return 1; // 1 - this shall not be executed again, directly after returning from this function!
    }

    int SyncDestructorCallback( struct dynlib_simulation_t * sim )
    {
        printf("The Block containing simulation is about to be destructed\n");

        // Trigger termination of the the main loop
        ExitLoop = true;
//     pthread_mutex_unlock(&ExitMutex);

    }


    void printInfo() {
        fprintf(stderr, "I'm a Process Receiver block\n");
    }

    // uncommonly used flags
    void PrepareReset() {}
    void HigherLevelResetStates() {}
    void PostInit() {}


    // The Computational function that is called by the simulator
    // and that distributes the execution to the various functions
    // in this C++ - Class, including: init(), io(), resetStates() and the destructor
    static int CompFn(int flag, struct dynlib_block_t *block) {
        return LibdynCompFnTempate<ProcessReceiverBlock>( flag, block ); // this expands a template for a C-comp fn
    }
    static int syncCallback_(struct dynlib_simulation_t * sim) {
        void * obj = sim->sync_callback.userdat;
        ProcessReceiverBlock *p = (ProcessReceiverBlock *) obj;
        return p->SyncCallback(sim);
    }
    static int syncCallbackDestructor_(struct dynlib_simulation_t * sim) {
        void * obj = sim->sync_callback.userdatDestructor;
        ProcessReceiverBlock *p = (ProcessReceiverBlock *) obj;
        return p->SyncDestructorCallback(sim);
    }

    // The data for this block managed by the simulator
    struct dynlib_block_t *block;
};


//
// An example block for accessing the shared object
//
class ProcessSendToBlock {
public:
    ProcessSendToBlock(struct dynlib_block_t *block) {
        this->block = block;    //  nothing more here. The real initialisation take place in init()
    }
    ~ProcessSendToBlock()
    {
        // free your allocated memory, ...
    }

    //
    // define states or other variables
    //

    Process_SharedObject *IShObj; // instance of the shared object


    uint NCopyBytes;

    //
    // initialise your block
    //

    int init() {
        printf("Init of ProcessSendToBlock\n");

        int *Uipar;
        double *Urpar;

        // Get the irpar parameters Uipar, Urpar
        libdyn_AutoConfigureBlock_GetUirpar(block, &Uipar, &Urpar);

        //
        // extract some structured sample parameters
        //

// 	fprintf(stderr, "Trying to get shared obj\n");
        // Obtain the shared object
        IShObj = NULL;
        if ( ortd_GetSharedObj<Process_SharedObject>(block, &IShObj) < 0 ) {
            return -1;
        }
//         fprintf(stderr, "Trying to get shared obj : %p -- ok\n", IShObj);


//         try {
// 	  irpar_ivec veccpp(Uipar, Urpar, 12); // then use:  veccpp.n; veccpp.v;
// 	  port = veccpp.v[0];
// 	  printf("SendTo port = %d\n", port); // print the first element
// 	                                           // of the vector that is of size veccpp.n
// 	} catch(int e) {
// 	  // parameter not found
// 	  port=0;
// 	}
//
// 	//
// 	irpar_string s(Uipar, Urpar, 13);
// 	const char *hostname  = s.s->c_str() ;
// 	printf("SendTo hostname = %s\n", hostname);




        // set the initial states
        resetStates();

        // calc how many bytes shall be copied
        int N = libdyn_get_inportsize(block, 0);
        int datatype = libdyn_get_inportdatatype(block, 0);
        int TypeBytes = libdyn_config_get_datatype_len(datatype);
        NCopyBytes = N * TypeBytes;

        // Return -1 to indicate an error, so the simulation will be destructed
        return 0;
    }


    inline void updateStates()
    {
    }


    inline void calcOutputs()
    {
        void *in1 = (void *) libdyn_get_input_ptr(block,0); // the first input port
        uint32_t *Nb = (uint32_t *) libdyn_get_input_ptr(block,1);

        if (*Nb <= NCopyBytes) {
//             printf("Sending %d Bytes\n", *Nb);
            IShObj->send(in1, *Nb);

            // call a function of the shared object
            //     sendTo ( in1, *Nb);
        } else {
            fprintf(stderr, "UDPSendTo: ERROR: The Number of bytes to send is too big!\n");
        }
    }


    inline void resetStates()
    {
    }


    void printInfo() {
        fprintf(stderr, "I'm a Sendto process block\n");
    }

    // uncommonly used flags
    void PrepareReset() {}
    void HigherLevelResetStates() {}
    void PostInit() {}


    // The Computational function that is called by the simulator
    // and that distributes the execution to the various functions
    // in this C++ - Class, including: init(), io(), resetStates() and the destructor
    static int CompFn(int flag, struct dynlib_block_t *block) {
        return LibdynCompFnTempate<ProcessSendToBlock>( flag, block ); // this expands a template for a C-comp fn
    }

    // The data for this block managed by the simulator
    struct dynlib_block_t *block;
};

























class compu_func_runproc_class {
public:
    compu_func_runproc_class(struct dynlib_block_t *block);
    void destruct();
    void io(int update_states);
    void reset();
    int init();
    void postinit();
private:
    struct dynlib_block_t *block;

    ortd_fork *process;
    int WhenToStart;

    bool firstShoot;

};

compu_func_runproc_class::compu_func_runproc_class(dynlib_block_t* block)
{
    this->block = block;
}

int compu_func_runproc_class::init()
{
    double *rpar = libdyn_get_rpar_ptr(block);
    int *ipar = libdyn_get_ipar_ptr(block);

    int prio = ipar[3];
    WhenToStart = ipar[4];
    int lenpathstr = ipar[5];
    int lenchcmdhstr = ipar[6];
    char *pathstr, *chpwd;

    irpar_getstr(&pathstr,  ipar, 7,              lenpathstr);
    irpar_getstr(&chpwd, ipar, 7 + lenpathstr, lenchcmdhstr);

    fprintf(stderr, "ortd_fork execpath = %s and %s\n", pathstr, chpwd);
    process = new ortd_fork(pathstr, chpwd, prio, false);   // char* exec_path, char* chpwd, int prio, bool replace_io

    free(pathstr);
    free(chpwd);

    firstShoot = true;

    /*    if (WhenToStart == 0) {
    //       printf("Starting\n");
          process->init();
        }*/

    // Return -1 to indicate an error, so the simulation will be destructed

    return 0;
}

void compu_func_runproc_class::postinit()
{
    if (WhenToStart == 0) {
//       fprintf(stderr, "### post init init\n");
        process->init();
    }
}

void compu_func_runproc_class::reset()
{
//   	fprintf(stderr, "####### RESET\n");

    if (WhenToStart == 1) {
        firstShoot = true;
// 	fprintf(stderr, "####### Terminate\n");
        process->terminate(1);
    }
}



void compu_func_runproc_class::io(int update_states)
{
    if (update_states==0) {

        if (firstShoot && WhenToStart == 1) {
// 	fprintf(stderr, "####### Init\n");
            firstShoot = false;
            process->init();
        }

//       double *output = (double*) libdyn_get_output_ptr(block, 0);
// 	*output = 1;


    }
}

void compu_func_runproc_class::destruct()
{
    process->terminate(2);  // Send "HUP" (Hangup signal)

    delete process;
}


// This is the main C-Callback function, which forwards requests to the C++-Class above
int compu_func_runproc(int flag, struct dynlib_block_t *block)
{


//     printf("compu_func_runproc: flag==%d\n", flag);

    double *in;
    double *rpar = libdyn_get_rpar_ptr(block);
    int *ipar = libdyn_get_ipar_ptr(block);

    int Nin = 0;
    int Nout = 1;


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        in = (double *) libdyn_get_input_ptr(block,0);
        compu_func_runproc_class *worker = (compu_func_runproc_class *) libdyn_get_work_ptr(block);

        worker->io(0);

    }
    return 0;
    break;
    case COMPF_FLAG_UPDATESTATES:
    {
        in = (double *) libdyn_get_input_ptr(block,0);
        compu_func_runproc_class *worker = (compu_func_runproc_class *) libdyn_get_work_ptr(block);

        worker->io(1);

    }
    return 0;
    break;
    case COMPF_FLAG_RESETSTATES:
    {
        in = (double *) libdyn_get_input_ptr(block,0);
        compu_func_runproc_class *worker = (compu_func_runproc_class *) libdyn_get_work_ptr(block);

        worker->reset();
    }
    return 0;
    break;
    case COMPF_FLAG_CONFIGURE:  // configure. NOTE: do not reserve memory or open devices. Do this while init instead!
    {
        int i;

        // FIXME: use BLOCKTYPE_DYNAMIC when dependence on in/output otherwise xx_STATIC, but then the output has to be
        //        connected otherwise the block will not be initialised
        libdyn_config_block(block, BLOCKTYPE_DYNAMIC, Nout, Nin, (void *) 0, 0);


        //libdyn_config_block_input(block, i, 1, DATATYPE_FLOAT);
        libdyn_config_block_output(block, 0, 1, DATATYPE_FLOAT, 1);


    }
    return 0;
    break;
    case COMPF_FLAG_INIT:  // init
    {
        compu_func_runproc_class *worker = new compu_func_runproc_class(block);
        libdyn_set_work_ptr(block, (void*) worker);

        int ret = worker->init();
        if (ret < 0)
            return -1;
    }
    return 0;
    break;
    case COMPF_FLAG_POSTINIT:
    {
        in = (double *) libdyn_get_input_ptr(block,0);
        compu_func_runproc_class *worker = (compu_func_runproc_class *) libdyn_get_work_ptr(block);

        worker->postinit();
    }
    return 0;
    break;
    case COMPF_FLAG_DESTUCTOR: // destroy instance
    {
        compu_func_runproc_class *worker = (compu_func_runproc_class *) libdyn_get_work_ptr(block);

        worker->destruct();
        delete worker;

    }
    return 0;
    break;
    case COMPF_FLAG_PRINTINFO:
        printf("I'm a template block\n");
        return 0;
        break;

    }
}

// Export to C so the libdyn simulator finds this function
extern "C" {
    // ADJUST HERE: must match to the function name in the end of this file
    int libdyn_module_ext_process_siminit(struct dynlib_simulation_t *sim, int bid_ofs);
}

// CHANGE HERE: Adjust this function name to match the name of your module
int libdyn_module_ext_process_siminit(struct dynlib_simulation_t *sim, int bid_ofs)
{

    // Register my blocks to the given simulation

    int blockid =  15300;  // CHANGE HERE: choose a unique id for each block

    libdyn_compfnlist_add(sim->private_comp_func_list, blockid + 0, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &compu_func_runproc);

    libdyn_compfnlist_add(sim->private_comp_func_list, blockid+1, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &SharedObjBlock<Process_SharedObject>::CompFn);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid+2, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &ProcessReceiverBlock::CompFn);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid+3, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &ProcessSendToBlock::CompFn);


//     Process_SharedObject
// ProcessReceiverBlock
// ProcessSendToBlock

#ifdef DEBUG
    printf("libdyn module ext_process initialised\n");
#endif  


}


