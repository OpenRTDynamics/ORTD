/*
    Copyright (C) 2010, 2011, 2012, 2013, 2014  Christian Klauer

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


// Wrapper around libdyn.c 's simulation setup code

/*
    TODO: long term:  remove libdyn_nested

    TODO: Suche nach sizeof(double) und abändern
    
    1.8.15: Removed bug in libdyn_nested2 in case the nesting has no in- or outputs 

*/



// #define DEBUG


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

    magic = 0xabcd34;
}

irpar::irpar(int Nipar, int Nrpar)
{
    this->Nipar = Nipar;
    this->Nrpar = Nrpar;

    this->ipar = (int*) malloc( sizeof(int) * Nipar );
    this->rpar = (double*) malloc( sizeof(double) * Nrpar );
}

irpar::~irpar()
{
    destruct();
}

void irpar::destruct()
{
    if (magic != 0xabcd34) {
        printf("irpar: this ptr seems to be wroing\n");
    }

    if (ipar != NULL) {
        free(ipar);
        ipar = NULL;
    }

    if (rpar != NULL) {
        free(rpar);
        rpar = NULL;
    }

}



bool irpar::load_from_afile(char *fname_i, char* fname_r)
{
    int err; // variable for errors

    err = irpar_load_from_afile(&ipar, &rpar, &Nipar, &Nrpar, fname_i, fname_r);
    if (err == -1) {
        fprintf(stderr, "Error loading irpar files\n");
        return false;
    }

//     fprintf(stderr, "irpar::load_from_afile(char*, char*) this ptr=%p ipar=%p rpar=%p\n", this, ipar, rpar );


    return true;
}

bool irpar::load_from_afile(char* fname)
{
    strcpy(fname_ipar, fname);
    strcat(fname_ipar, ".ipar");
    strcpy(fname_rpar, fname);
    strcat(fname_rpar, ".rpar");

    fprintf(stderr, "fnames ipar = %s\n", fname_ipar);
    fprintf(stderr, "fnames rpar = %s\n", fname_rpar);

    bool ret = this->load_from_afile(fname_ipar, fname_rpar);

//    fprintf(stderr, "irpar::load_from_afile(char *fname) this ptr=%p ipar=%p rpar=%p\n", this, ipar, rpar );

    return ret;

}



//
//
// class libdyn_nested Version 1 (OLD)
//
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
    int *intypes_ = (int*) malloc(sizeof(int) * iocfg.inports);
    int *outtypes_ = (int*) malloc(sizeof(int) * iocfg.outports);

    for (i = 0; i < iocfg.inports; ++i) {
        insizes[i] = insizes_[i];
        intypes_[i] = intypes[i]; // DATATYPE_FLOAT; // default datatype
    }
    for (i = 0; i < iocfg.outports; ++i) {
        outsizes[i] = outsizes_[i];
        outtypes_[i] = outtypes[i]; // DATATYPE_FLOAT;// default datatype
    }

    iocfg.insizes = insizes;
    iocfg.outsizes = outsizes;
    iocfg.intypes = intypes_;
    iocfg.outtypes = outtypes_;



    // List of Pointers to in vectors
    iocfg.inptr = (double **) malloc(sizeof(double *) * iocfg.inports);

    for (i=0; i<iocfg.inports; ++i)
        iocfg.inptr[i] = (double*) NULL;

    //
    // Alloc list of pointers for outvalues comming from libdyn
    // These pointers will be set by irpar_get_libdynconnlist
    //

    iocfg.outptr = (double **) malloc(sizeof(double *) * iocfg.outports);

    for (i=0; i<iocfg.outports; ++i)
        iocfg.outptr[i] = (double*) NULL;

    // Initially there is no master
    this->ld_master = NULL;

    // Initially there is no simulation
    this->current_sim = NULL;

//     fprintf(stderr, "Internal init finished\n");

    return true;
}

void libdyn_nested::set_buffer_inptrs()
{
    int i;
    char *ptr = (char*) InputBuffer;

    // share the allocated_inbuffer accoss all inputs

    i = 0;
    iocfg.inptr[i] = (double *) ptr; //sim->cfg_inptr(i, (double *) ptr);

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

bool libdyn_nested::cfg_inptr(int in, void* inptr)
{
    if (this->use_buffered_input == true) {
        fprintf(stderr, "cfg_inptr: You configured buffered input\n");
        return false;
    }

    if ( (iocfg.inports <= in) || (in < 0) ) {
        fprintf(stderr, "cfg_inptr: in out of range\n");
        return false;
    }

    iocfg.inptr[in] = (double*) inptr;

    return true;
}



libdyn_nested::libdyn_nested(int Nin, const int* insizes_, const int* intypes, int Nout, const int* outsizes_, const int* outtypes)
{
    this->sim_slots = NULL;
    this->usedSlots = 0;
    this->internal_init(Nin, insizes_, intypes, Nout, outsizes_, outtypes);
}

libdyn_nested::libdyn_nested(int Nin, const int* insizes_, const int* intypes, int Nout, const int* outsizes_, const int* outtypes, bool use_buffered_input)
{
    this->sim_slots = NULL; // initially slots are not used
    this->use_buffered_input = use_buffered_input;
    this->usedSlots = 0;

    this->internal_init(Nin, insizes_, intypes, Nout, outsizes_, outtypes);


    if (use_buffered_input) {
        /*    printf("Allocate buffers\n");
        */
        this->allocate_inbuffers();
    }


    //this->libdyn_nested(Nin, insizes_, intypes, Nout, outsizes_, outtypes);

}

void libdyn_nested::destruct()
{

    if ( (sim_slots != NULL) ) {

        // slots are used
        // destruct all nested simulations
        int i;

        lock_slots();

        for (i = 0; i < Nslots; i++) {
            libdyn * sim = sim_slots[i];
            if (sim != NULL) {
                sim->destruct();
                delete sim;

                sim_slots[i] = NULL;
            }
        }

        unlock_slots();

    } else if (current_sim != NULL) {
        // the is only one simulation to destruct

        current_sim->destruct();
        delete current_sim;

        current_sim == NULL;
    }


    free_slots();

}


void libdyn_nested::allocate_slots(int n)
{
    if (this->sim_slots != NULL) {
        fprintf(stderr, "nested: slotes are already allocated!\n");
        return;
    }


    this->Nslots = n;
    this->usedSlots = 0;
    this->sim_slots = (libdyn **) malloc(n * sizeof( libdyn * ));

    // all slots are initialised with zero
    int i;
    for (i = 0; i < n; ++i) {
        this->sim_slots[i] = NULL;
    }

    slot_addsim_pointer = 0;
    current_slot = 0;

    pthread_mutex_init(&slots_mutex, NULL);
}

void libdyn_nested::free_slots()
{
    if (this->sim_slots != NULL) {
        free(sim_slots);
        pthread_mutex_destroy(&slots_mutex);
    }
}

int libdyn_nested::slots_available()
{
    return Nslots - slot_addsim_pointer;
}

bool libdyn_nested::slotindexOK(int nSim)
{
    if (nSim < 0)
        return false;

    if (nSim >= this->usedSlots)
        return false;

    return true;
}




int libdyn_nested::add_simulation(int slotID, irpar* param, int boxid)
{
    if (sim_slots != NULL)
        if (slots_available() <= 0)
            return -1; // no more slots available

    return this->add_simulation(slotID, param->ipar, param->rpar, boxid);
}

int libdyn_nested::add_simulation(irpar* param, int boxid)
{
    return this->add_simulation(-1, param->ipar, param->rpar, boxid);
}


// Add a simulation based on provided irpar set
int libdyn_nested::add_simulation(int slotID, int* ipar, double* rpar, int boxid)
{
//     if (sim_slots != NULL)
//         if (slots_available() <= 0)
//             return -1; // no more slots available


    libdyn *sim;

//    sim = new libdyn(&iocfg);
    sim = new libdyn( iocfg.inports, iocfg.insizes, iocfg.outports, iocfg.outsizes );
    sim->set_master(ld_master);

    // set pointers to inputs
    int i;
    for (i = 0; i < iocfg.inports; ++i) {
        sim->cfg_inptr(i, iocfg.inptr[i]);
    }

    // set-up schematic
    int err;
    err = sim->irpar_setup(ipar, rpar, boxid); // compilation of schematic

    if (err == -1) {
        // There may be some problems during compilation.
        // Errors are reported on stdout
        fprintf(stderr, "Error in libdyn\n");

        return err;
    }

    fprintf(stderr, "Simulation was set-up\n");

    if (this->add_simulation(slotID, sim) < 0)
        return -1;


    return err;
}

int libdyn_nested::add_simulation(int slotID, libdyn* sim)
// main add_simulation function that is called by the two other derivates
{



    if (slotID == -1) {
        // also set to be the current simulation
        current_sim = sim;

        if (sim_slots == NULL) {
            return 1; // slots are not used: go out - everything is fine now
        }

        // slots are not used
        if (slots_available() <= 0) {
            fprintf(stderr, "nested: no more slots available\n");
            return -1;
        }

        // add simulation to the next slot
        sim_slots[ slot_addsim_pointer ] = sim;

        //     printf("added sim %p to slot %d\n", sim, slot_addsim_pointer);

        slot_addsim_pointer++;
        usedSlots++;
    } else {
        // slots are used

        if (sim_slots == NULL) {
            fprintf(stderr, "libdyn_nested: ASSERTION FAILED slots are not configured\n");
            return -1;
        }


        if (!slotindexOK(slotID)) {
            fprintf(stderr, "libdyn_nested: ASSERTION FAILED bad slot\n");
            return -1;
        }

        lock_slots();

        // slot has to be free
        if (sim_slots[ slotID ] != NULL) {
            unlock_slots();
            fprintf(stderr, "libdyn_nested: ASSERTION FAILED slot is not free\n");
            return -1;
        }

        sim_slots[ slotID ] = sim;

        unlock_slots();

    }

    return 1;
}

int libdyn_nested::del_simulation(int slotID)
{
    if (sim_slots == NULL) {
        fprintf(stderr, "libdyn_nested: ASSERTION FAILED slots are not configured\n");
        return -1;
    }

    if (!slotindexOK(slotID)) {
        fprintf(stderr, "libdyn_nested: ASSERTION FAILED bad slot\n");
        return -1;
    }

    lock_slots();

    if (is_current_simulation(slotID)) {
        // Tryed to exchange the currently active simulation
        fprintf(stderr, "libdyn_nested: Tryed to exchange the currently active simulation\n");

        unlock_slots();
        return -2;
    }

    if (sim_slots[ slotID ] == NULL) {
        unlock_slots();

        fprintf(stderr, "libdyn_nested: ASSERTION FAILED slot cannot be destructed because it is already free\n");
        return -1;
    }

    // destruct the simulation
    libdyn *sim = this->sim_slots[slotID];
    sim->destruct();

    // mark as a free slot
    this->sim_slots[slotID] = NULL;

    unlock_slots();

    return 1;
}

int libdyn_nested::del_simulation(int slotID, int switchto_slotID)
{
    libdyn *sim; // FIXME volatile

    if (sim_slots == NULL) {
        fprintf(stderr, "libdyn_nested: ASSERTION FAILED slots are not configured\n");
        return -1;
    }

    if (!slotindexOK(slotID)) {
        fprintf(stderr, "libdyn_nested: ASSERTION FAILED bad slot\n");
        return -1;
    }

    if (!slotindexOK(switchto_slotID)) {
        fprintf(stderr, "libdyn_nested: ASSERTION FAILED bad slot for switchto_slotID\n");
        return -1;
    }

    lock_slots();
    {

        if (sim_slots[ slotID ] == NULL) {
            unlock_slots();

            fprintf(stderr, "libdyn_nested: ASSERTION FAILED slot cannot be destructed because it is already free\n");
            return -1;
        }

        // switch to another simulation
        current_sim = this->sim_slots[switchto_slotID];

        // destruct the simulation
        sim = this->sim_slots[slotID];

        // mark as a free slot
        this->sim_slots[slotID] = NULL;

    }
    unlock_slots();

    sim->destruct(); // do the destruction outside the locked area since this lasts a long time

    return 1;

}


bool libdyn_nested::load_simulations(int* ipar, double* rpar, int start_boxid, int NSimulations)
{
    int irparid;

    // add all simulations
    for (irparid = start_boxid; irparid < start_boxid + NSimulations; ++irparid) {
        this->add_simulation(-1, ipar, rpar, irparid);
    }
}

bool libdyn_nested::is_current_simulation(int slotID)
{

    // lock_slots(); has to be called in advance to calling this function
    // unlock_slots() afterwards

    if (sim_slots == NULL) {
        fprintf(stderr, "libdyn_nested: ASSERTION FAILED slots are not configured\n");
        return false;
    }

    if (!slotindexOK(slotID)) {
        fprintf(stderr, "libdyn_nested: ASSERTION FAILED bad slot\n");
        return false;
    }

    bool result = (sim_slots[slotID] == current_sim);

    return result;
}


bool libdyn_nested::set_current_simulation(int nSim)
{

    if (sim_slots == NULL) {
        fprintf(stderr, "libdyn_nested: ASSERTION FAILED slots are not configured\n");
        return false;
    }

    if (!slotindexOK(nSim)) {
       // fprintf(stderr, "libdyn_nested: ASSERTION FAILED bad slot\n");
        return false;
    }

    lock_slots();

    if (sim_slots[ nSim ] == NULL) {
        unlock_slots();

        fprintf(stderr, "libdyn_nested: NOTE: slot cannot be destructed because it is already free\n");
        return true;
    }


    current_sim = this->sim_slots[nSim];

    unlock_slots();

    /*
        if ( (sim_slots != NULL) ) {
            if (slotindexOK(nSim)) {
                if (this->sim_slots[nSim] != NULL) {



    //       printf("switching simulation to %d  cs = %p\n", nSim, current_sim);

                    return true;
                }
            }
        } else {
            fprintf(stderr, "libdyn_nested: slots are not configured\n");
        }

        return false;*/
}



void libdyn_nested::copy_outport_vec(int nPort, void* dest)
{
    void *src = current_sim->get_vec_out(nPort);
    int len = this->iocfg.outsizes[nPort];
    int datasize = sizeof(double); // FIXME

// //   printf("copying src from cs=%p, dptr=%p\n", current_sim, src);
//     int i;
//     printf("copy_outport_vec from ptr %p: ", src);
//     for (i = 0; i<= len; ++i) {
//       printf("%f ",  ( (double*) src )[i]  );
//
//     }
//     printf("\n");

    memcpy(dest, src, len*datasize);
}

// makes only sense of use_buffered_input == true
void libdyn_nested::copy_inport_vec(int nPort, void* src)
{
    if (use_buffered_input == false) {
        fprintf(stderr, "You are not using buffered_input\n");
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

void libdyn_nested::reset_blocks()
{
    current_sim->reset_blocks();
}


void libdyn_nested::event_trigger_mask(int mask)
{
    current_sim->event_trigger_mask(mask);
}


































//
//
// class libdyn_nested Version 2
//
//


void libdyn_nested2::allocate_structures(int Nin, int Nout)
{
    int tmp, i;
    double *p;

    //
    ConfigurationFinished = false;

    // standard configuration
    this->sim_slots = NULL;
    this->usedSlots = 0;
    this->current_sim = NULL;
    this->ParentSim = NULL;
    this->Parent_SimnestClassPtr = NULL;
    this->NestedLevel = 0;
    this->ElementIDcounter = 0; // init the counter for the elements list

    //
    // configure I/O
    //
    iocfg.provided_outcaches = 0;

    iocfg.inports = Nin;
    iocfg.outports = Nout;

    // allocate arrays for the I/O configuration
    iocfg.insizes = (int*) malloc(sizeof(int) * iocfg.inports);
    iocfg.outsizes = (int*) malloc(sizeof(int) * iocfg.outports);
    iocfg.intypes = (int*) malloc(sizeof(int) * iocfg.inports);
    iocfg.outtypes = (int*) malloc(sizeof(int) * iocfg.outports);


    //
    // Allocate the list of pointers to the input vectors
    //
    if (iocfg.inports > 0) {
    iocfg.inptr = (double **) malloc(sizeof(double *) * iocfg.inports);

//     fprintf(stderr, "+++++++++++++++++++ %p allocate_structures iocfg.inptr = %p, iocfg.inports = %d \n", this, iocfg.inptr, iocfg.inports);

    
    // init with NULL
    for (i=0; i<iocfg.inports; ++i)
        iocfg.inptr[i] = (double*) NULL;

    } else {
      iocfg.inptr = NULL;
    }
    
    //
    // Alloc list of pointers for outvalues comming from libdyn
    // These pointers will be set by irpar_get_libdynconnlist
    //

    if (iocfg.outports > 0) {
    iocfg.outptr = (double **) malloc(sizeof(double *) * iocfg.outports);

    // init with NULL
    for (i=0; i<iocfg.outports; ++i)
        iocfg.outptr[i] = (double*) NULL;

    } else {
      iocfg.outptr = NULL;
    }

    // Initially there is no master
    this->ld_master = NULL;

    // Initially there is no simulation
    this->current_sim = NULL;


}


bool libdyn_nested2::internal_init(int Nin, const int* insizes_, const int* intypes, int Nout, const int* outsizes_, const int* outtypes)
{
    int tmp, i;
    double *p;

    allocate_structures(Nin, Nout);
    /*
        iocfg.provided_outcaches = 0;

        iocfg.inports = Nin;
        iocfg.outports = Nout;




        int *insizes = (int*) malloc(sizeof(int) * iocfg.inports);
        int *outsizes = (int*) malloc(sizeof(int) * iocfg.outports);
        int *intypes_ = (int*) malloc(sizeof(int) * iocfg.inports);
        int *outtypes_ = (int*) malloc(sizeof(int) * iocfg.outports);*/

    // copy the given I/O config arrays
    for (i = 0; i < iocfg.inports; ++i) {
        iocfg.insizes[i] = insizes_[i];
        iocfg.intypes[i] = intypes[i]; // DATATYPE_FLOAT; // default datatype
    }
    for (i = 0; i < iocfg.outports; ++i) {
        iocfg.outsizes[i] = outsizes_[i];
        iocfg.outtypes[i] = outtypes[i]; // DATATYPE_FLOAT;// default datatype
    }





//     fprintf(stderr, "Internal init finished\n");

    return true;
}

void libdyn_nested2::set_buffer_inptrs()
{
    int i;
    char *ptr = (char*) InputBuffer;

    // share the allocated_inbuffer accoss all inputs

    i = 0;
    
//     fprintf(stderr, "+++++++++++++++++++ %p set_buffer_inptrs iocfg.inptr = %p \n", this, iocfg.inptr );
    
    
    if (iocfg.inptr != NULL) {
    iocfg.inptr[i] = (double *) ptr; //sim->cfg_inptr(i, (double *) ptr);

    // calc datatype lengthts
    for (i = 1; i < iocfg.inports; ++i) {

        int TypeBytes = libdyn_config_get_datatype_len( iocfg.intypes[i-1] );
        int vlen = iocfg.insizes[i-1];
        ptr += TypeBytes*vlen;

//          printf("--------------------- libdyn_nested2::set_buffer_inptrs %p\n", ptr);

        //  sim->cfg_inptr(i, (double*) ptr);
        iocfg.inptr[i] = (double *) ptr;

    }
    }
}


bool libdyn_nested2::allocate_inbuffers()
{
    int i;

    int nBytes_for_input = 0;

    // calc the required memory for all input ports
    for (i = 0; i < iocfg.inports; ++i) {

        // calc the required bytes for the port i
        int TypeBytes = libdyn_config_get_datatype_len( iocfg.intypes[i] );
        int vlen = iocfg.insizes[i];

//         int element_len = sizeof(double); // libdyn_get_typesize(iocfg.intypes[i]); // FIXME
//         int vlen = iocfg.insizes[i];

        nBytes_for_input += TypeBytes * vlen;
    }

    // allocate the buffer
    InputBuffer = (void*) malloc(nBytes_for_input);

    // set the simulation's input ports
    this->set_buffer_inptrs();

    return true;
}

bool libdyn_nested2::cfg_inptr(int in, void* inptr)
{
    if (ConfigurationFinished) {
        fprintf(stderr, "ASSERTION FAILED: libdyn_nested2::FinishConfiguration: already finished\n");
        throw 1;
    }

    if (this->use_buffered_input == true && inptr != NULL) {
        fprintf(stderr, "ASSERTION failed: cfg_inptr: You configured buffered input. Set inptr = NULL\n");
//         return false;
        throw 1;
    }

    if ( (iocfg.inports <= in) || (in < 0) ) {
        fprintf(stderr, "ASSERTION failed: cfg_inptr: in out of range\n");
//         return false;
        throw 1;
    }

    // if buffered inputs are used, do not overwrite the pointers to the allocated buffers
    if (this->use_buffered_input)
        return true;

    iocfg.inptr[in] = (double*) inptr;

    return true;
}

bool libdyn_nested2::cfg_inptr(int in, void* inptr, int size, int datatype)
{
    if (ConfigurationFinished) {
        fprintf(stderr, "ASSERTION FAILED: libdyn_nested2::FinishConfiguration: already finished\n");
        throw 1;
    }

    if ( !cfg_inptr(in, inptr) ) {
        throw 1;
    };

    iocfg.insizes[in] = size;
    iocfg.intypes[in] = datatype;
}

void libdyn_nested2::cfg_output(int out, int size, int datatype)
{
    if (ConfigurationFinished) {
        fprintf(stderr, "ASSERTION FAILED: libdyn_nested2::cfg_output: already finished\n");
        throw 1;
    }

    if ( (iocfg.outports <= out) || (out < 0) ) {
        fprintf(stderr, "ASSERTION failed:  libdyn_nested2::cfg_output: out of range\n");
        throw 1;
    }

    iocfg.outsizes[out] = size;
    iocfg.outtypes[out] = datatype;
}


// TODO: remove
libdyn_nested2::libdyn_nested2(int Nin, const int* insizes_, const int* intypes, int Nout, const int* outsizes_, const int* outtypes)
{
//     this->sim_slots = NULL;
//     this->usedSlots = 0;
//     this->current_sim = NULL;
//     this->ParentSim = NULL;

    this->use_buffered_input = false;

    this->internal_init(Nin, insizes_, intypes, Nout, outsizes_, outtypes);

//         // for online exchange support
//     this->initialised_replaced_simulation = false;
//     this->replaceable_simulation = false; // start with false
}

// TODO: remove
libdyn_nested2::libdyn_nested2(int Nin, const int* insizes_, const int* intypes, int Nout, const int* outsizes_, const int* outtypes, bool use_buffered_input)
{
//     this->sim_slots = NULL; // initially slots are not used
//     this->usedSlots = 0;
//     this->current_sim = NULL;
//     this->ParentSim = NULL;

    this->use_buffered_input = use_buffered_input;

    this->internal_init(Nin, insizes_, intypes, Nout, outsizes_, outtypes);


    if (use_buffered_input) {
        this->allocate_inbuffers();
    }

}


libdyn_nested2::libdyn_nested2(int Nin, int Nout, bool use_buffered_input)
{
    this->use_buffered_input = use_buffered_input;

    this->allocate_structures(Nin, Nout);

    // The allocated_inbuffers() is performed in the call FinishConfiguration()
    // that has to be called when using this constructor
    
//     if (use_buffered_input) {
//         this->allocate_inbuffers();
//     }
}

void libdyn_nested2::FinishConfiguration()
{
    if (ConfigurationFinished) {
        fprintf(stderr, "ASSERTION FAILED: libdyn_nested2::FinishConfiguration: already finished\n");
        throw 1;
    }

    // allocate the input buffers if needed.
    if (use_buffered_input) {
        this->allocate_inbuffers();
    }

    ConfigurationFinished = true;
}


void libdyn_nested2::destruct()
{

    if ( (sim_slots != NULL) ) {

        // slots are used
        // destruct all nested simulations
        int i;

        lock_slots();

        for (i = 0; i < Nslots; i++) { // for each slot
            if (sim_slots[i].sim != NULL) {
                sim_slots[i].sim->destruct();
                delete sim_slots[i].sim;
		
#ifdef DEBUG
		fprintf(stderr, "libdyn_nested2::destruct(): removed  %p from %p index %d\n",  sim_slots[i].sim, sim_slots, i );
#endif
		
                sim_slots[i].sim = NULL;
            }
        }

        unlock_slots();

    } else if (current_sim != NULL) {
        // the is only one simulation to destruct

        current_sim->destruct();
        delete current_sim;

        current_sim == NULL;
    }


    free_slots();

    // deallocate ioconfig structures
    free(iocfg.insizes);
    free(iocfg.outsizes);
    free(iocfg.intypes);
    free(iocfg.outtypes);
    
    if (iocfg.inptr != NULL)
    free(iocfg.inptr);
    
    if (iocfg.outptr != NULL)
    free(iocfg.outptr);
    
    // dellocate input buffers if any
    if (use_buffered_input) {
      free(InputBuffer); 
    }
}

int libdyn_nested2::CallSyncCallbackDestructor()
{
    //
    //

    if ( (sim_slots != NULL) ) {

        // slots are used
        // call destructor for all nested simulations
        int i;

        lock_slots();

        for (i = 0; i < Nslots; i++) { // for each slot
            if (sim_slots[i].sim != NULL) {
                sim_slots[i].sim->CallSyncCallbackDestructor();
            }
        }

        unlock_slots();

    } else if (current_sim != NULL) {
        // the is only one simulation to destruct
        // Forward Callback
        current_sim->CallSyncCallbackDestructor();

    }



    // FIXME always returns 0
    return 0;
}


void libdyn_nested2::allocate_slots(int n)
{
    if (this->sim_slots != NULL) {
        fprintf(stderr, "libdyn_nested2: slotes are already allocated!\n");
        return;
    }


    this->Nslots = n;
    this->usedSlots = 0;
    this->sim_slots = (sim_slot_t *) malloc(n * sizeof( sim_slot_t  ));

    // all slots are initialised with zero
    int i;
    for (i = 0; i < n; ++i) {
        this->sim_slots[i].sim = NULL;
    }

    slot_addsim_pointer = 0;
    current_slot_nr = 0;

    pthread_mutex_init(&slots_mutex, NULL);
}

void libdyn_nested2::free_slots()
{
    if (this->sim_slots != NULL) {
        free(sim_slots);
        pthread_mutex_destroy(&slots_mutex);
    }
}

int libdyn_nested2::slots_available()
{
    return Nslots - slot_addsim_pointer;
}

bool libdyn_nested2::slotindexOK(int nSim)
{
    if (nSim < 0)
        return false;

    if (nSim >= this->usedSlots)
        return false;

    return true;
}

void libdyn_nested2::set_parent_simulation(dynlib_simulation_t* ParentSim)
{
    this->ld_master = (libdyn_master *) ParentSim->master;
    this->ParentSim = ParentSim;
//   this->Parent_SimnestClassPtr = (class libdyn_nested2*) ParentSim->SimnestClassPtr;
    this->Parent_SimnestClassPtr = libdyn_nested2::GetSimnestClassPtrFromC(ParentSim);

    if (this->Parent_SimnestClassPtr == NULL) {
        fprintf(stderr, "Warning: libdyn_nested2::set_parent_simulation: Parent simulation is not included by a simnest2 wrapper class\n");
        this->setNestedLevel(1); // nested level must be at least one
        return;
    }

    // inc nesting level counters
    this->setNestedLevel( Parent_SimnestClassPtr->getNestedLevel() + 1 );
}


int libdyn_nested2::add_simulation(int slotID, irpar* param, int boxid)
{
//   if (!ConfigurationFinished) {
//     fprintf(stderr, "ASSERTION FAILED: Configuration must be finished before calling add_simulation.");
//   }

    if (sim_slots != NULL)
        if (slots_available() <= 0)
            return -1; // no more slots available

    return this->add_simulation(slotID, param->ipar, param->rpar, boxid);
}

int libdyn_nested2::add_simulation(irpar* param, int boxid)
{
    return this->add_simulation(-1, param->ipar, param->rpar, boxid);
}


// Add a simulation based on provided irpar set
int libdyn_nested2::add_simulation(int slotID, int* ipar, double* rpar, int boxid)
{
//     if (sim_slots != NULL)
//         if (slots_available() <= 0)
//             return -1; // no more slots available


    libdyn *sim;

    // create a new simulation
    sim = new libdyn( iocfg.inports, iocfg.insizes, iocfg.intypes, iocfg.outports, iocfg.outsizes, iocfg.outtypes);

    // master
    sim->set_master(ld_master);


    // set pointers to inputs
    int i;
    for (i = 0; i < iocfg.inports; ++i) {
        sim->cfg_inptr(i, iocfg.inptr[i]);
    }

    // set-up schematic
    int err;
    err = sim->irpar_setup(ipar, rpar, boxid); // compilation of schematic

    if (err == -1) {
        // There may be some problems during compilation.
        // Errors are reported on stdout
        fprintf(stderr, "libdyn_nested2: Error in libdyn interpreter\n");

        return err;
    }
    
#ifdef DEBUG
    fprintf(stderr, "Nested Simulation was set-up. Nesting level is %d\n", NestedLevel);
#endif    

    if (this->add_simulation(slotID, sim) < 0)
        return -1;


    return err; // return
}



int libdyn_nested2::add_simulation(int slotID, libdyn* sim)
// main add_simulation function that is called by the two other derivates
{
    // store the pointer to the inestance of this wrapper class in the simulation
    sim->get_C_SimulationObject()->SimnestClassPtr = (void*) this;
//   printf("------------------------ sim->get_C_SimulationObject()->SimnestClassPtr = (void*) this;\n");

    if (slotID == -1) {

        if (sim_slots == NULL) {
            return 1; // slots are not used: go out - everything is fine now
        }

        // slots are not used
        if (slots_available() <= 0) {
            fprintf(stderr, "nested: no more slots available\n");
            return -1;
        }

        // add simulation to the next free slot
        sim_slots[ slot_addsim_pointer ].sim = sim;
        sim_slots[ slot_addsim_pointer ].replaceable_simulation_initialised = false;

	#ifdef DEBUG
		fprintf(stderr, "libdyn_nested2::add_simulation: inserted1  %p into %p index %d\n",  sim, sim_slots, slot_addsim_pointer );
#endif


        // also set to be the current simulation
        current_sim = sim;
        current_slot = &sim_slots[slot_addsim_pointer];
        current_slot_nr = slot_addsim_pointer;


        // inc counters
        slot_addsim_pointer++;
        usedSlots++;
    } else {
        // slots are used
        if (sim_slots == NULL) {
            fprintf(stderr, "libdyn_nested: ASSERTION FAILED slots are not configured\n");
            return -1;
        }

        if (!slotindexOK(slotID)) {
            fprintf(stderr, "libdyn_nested: ASSERTION FAILED bad slot\n");
            return -1;
        }

        lock_slots();

        // slot has to be free
        if (sim_slots[ slotID ].sim != NULL) {
            unlock_slots();
            fprintf(stderr, "libdyn_nested: ASSERTION FAILED slot is not free\n");
            return -1;
        }

#ifdef DEBUG
		fprintf(stderr, "libdyn_nested2::add_simulation: inserted2  %p into %p index %d\n",  sim, sim_slots, slotID );
#endif

        
        sim_slots[ slotID ].sim = sim;
        sim_slots[ slotID ].replaceable_simulation_initialised = false;

        // also set to be the current simulation // MAYBE WAS A BUG TO NOT DO THIS? related to async nested simulations?
        current_sim = sim;
        current_slot = &sim_slots[ slotID ];
        current_slot_nr = slot_addsim_pointer;


        unlock_slots();
    }

    return 1;
}

int libdyn_nested2::del_simulation(int slotID)
{
    if (sim_slots == NULL) {
        fprintf(stderr, "libdyn_nested: ASSERTION FAILED slots are not configured\n");
        return -1;
    }

    if (!slotindexOK(slotID)) {
        fprintf(stderr, "libdyn_nested: ASSERTION FAILED bad slot\n");
        return -1;
    }

    lock_slots();

    if (is_current_simulation(slotID)) {
        // Tryed to exchange the currently active simulation
        fprintf(stderr, "libdyn_nested: Tryed to exchange the currently active simulation\n");

        unlock_slots();
        return -2;
    }

    if (sim_slots[ slotID ].sim == NULL) {
        unlock_slots();

        fprintf(stderr, "libdyn_nested: NOTE slot cannot be destructed because it is already free\n");
        return 1;
    }

    // destruct the simulation
    libdyn *sim = this->sim_slots[slotID].sim;
    sim->destruct();
    delete sim;

    // mark as a free slot
    this->sim_slots[slotID].sim = NULL;

    unlock_slots();

    return 1;
}

int libdyn_nested2::del_simulation(int slotID, int switchto_slotID)
{
    libdyn *sim; // FIXME volatile

    if (sim_slots == NULL) {
        fprintf(stderr, "libdyn_nested: ASSERTION FAILED slots are not configured\n");
        return -1;
    }

    if (!slotindexOK(slotID)) {
        fprintf(stderr, "libdyn_nested: ASSERTION FAILED bad slot\n");
        return -1;
    }

    if (!slotindexOK(switchto_slotID)) {
        fprintf(stderr, "libdyn_nested: ASSERTION FAILED bad slot for switchto_slotID\n");
        return -1;
    }

    lock_slots();
    {

        if (sim_slots[ slotID ].sim == NULL) {
            unlock_slots();

            fprintf(stderr, "libdyn_nested: NOTE: slot cannot be destructed because it is already free\n");
            return 1;
        }

        // switch to another simulation
        current_sim = this->sim_slots[switchto_slotID].sim;

        // the simulation to destruct
        sim = this->sim_slots[slotID].sim;

        // mark as a free slot
        this->sim_slots[slotID].sim = NULL;

    }
    unlock_slots();

    // do the destruction outside the locked area since this could lasts a long time and would therefore destroy realtime
    sim->destruct();
    delete sim;

    return 1;

}


bool libdyn_nested2::load_simulations(int* ipar, double* rpar, int start_boxid, int NSimulations)
{
    int irparid;

    // add all simulations
    for (irparid = start_boxid; irparid < start_boxid + NSimulations; ++irparid) {
        this->add_simulation(-1, ipar, rpar, irparid);
    }
}

bool libdyn_nested2::is_current_simulation(int slotID)
{

    // lock_slots(); has to be called in advance to calling this function
    // unlock_slots() afterwards

    if (sim_slots == NULL) {
        fprintf(stderr, "libdyn_nested: ASSERTION FAILED slots are not configured\n");
        return false;
    }

    if (!slotindexOK(slotID)) {
        fprintf(stderr, "libdyn_nested: ASSERTION FAILED bad slot\n");
        return false;
    }

//     bool result = (sim_slots[slotID].sim == current_sim);
    bool result = (slotID == current_slot_nr);

    return result;
}

dynlib_simulation_t* libdyn_nested2::get_current_simulation_libdynSimStruct()
{
    return current_sim->get_C_SimulationObject();
}

bool libdyn_nested2::set_current_simulation(int nSim)
{

    if (sim_slots == NULL) {
        fprintf(stderr, "libdyn_nested: ASSERTION FAILED slots are not configured\n");
        return false;
    }

    if (!slotindexOK(nSim)) {
        //fprintf(stderr, "libdyn_nested: ASSERTION FAILED bad slot\n");
        return false;
    }

    lock_slots();

    if (sim_slots[ nSim ].sim == NULL) {
        unlock_slots();
#ifdef DEBUG
        fprintf(stderr, "libdyn_nested: Slot %d is unused! Cannot switch to this simulation.\n", nSim);
#endif
	return false;
    }


    current_sim = this->sim_slots[nSim].sim;
    current_slot = &sim_slots[nSim];
    current_slot_nr = nSim;

    unlock_slots();

}



void libdyn_nested2::copy_outport_vec(int nPort, void* dest)
{
    // copy the output port nPort of the current simulation to dest
    void *src = current_sim->get_vec_out(nPort);

    int len = this->iocfg.outsizes[nPort];
    int outtype = iocfg.outtypes[nPort];

//     printf("libdyn_nested2::copy_outport_vec\n");
    int TypeBytes = libdyn_config_get_datatype_len( outtype );

    memcpy(dest, src, len*TypeBytes);
}

// makes only sense of use_buffered_input == true
void libdyn_nested2::copy_inport_vec(int nPort, void* src)
{
    if (use_buffered_input == false) {
        fprintf(stderr, "libdyn_nested2: ASSERTION FAILED: You are not using buffered_input\n");
        return;
    }

    int vlen = iocfg.insizes[nPort];   // input port size
    int intype = iocfg.intypes[nPort]; // input port type

    int TypeBytes = libdyn_config_get_datatype_len( intype );


    memcpy((void*) iocfg.inptr[nPort], src, TypeBytes * vlen);
}


void libdyn_nested2::simulation_step(int update_states)
{
    current_sim->simulation_step(update_states);
}

void libdyn_nested2::simulation_step_outpute()
{
    current_sim->simulation_step(0);
    current_slot->replaceable_simulation_initialised = true;
//  initialised_replaced_simulation = true;
}

void libdyn_nested2::simulation_step_supdate()
{
    // make sure that simulation_step_output is called before simulation_step_supdate
    // if not,skip and wait

    if (current_slot->replaceable_simulation_initialised) {
//     printf("sim is initialised\n");
        current_sim->simulation_step(1);
    } else {
//      printf("sim is not initialised skipping s update\n");
    }
}


void libdyn_nested2::reset_blocks()
{
    // reset the currently active simulation
    current_sim->reset_blocks();
}

void libdyn_nested2::forwardReset()
{
//   this->ParentSim;
//   this->Parent_SimnestClassPtr;

// TODO Add a counter for the number of forwardings

    this->reset_blocks();
}

void libdyn_nested2::reset_blocks(int slotId)
{
    // UNTESTED AND UNUSED FOR NOW

    //
    // Reset blocks of the simulation assigned to slotID, which cannot be the currently active simulation
    // This may be called from other threads
    //

    if (sim_slots == NULL) {
        fprintf(stderr, "libdyn_nested: ASSERTION FAILED slots are not configured\n");
        return;
    }

    if (!slotindexOK(slotId)) {
        fprintf(stderr, "libdyn_nested: ASSERTION FAILED bad slot\n");
        return;
    }

    lock_slots();
    {

        if (current_slot_nr == slotId) {
            fprintf(stderr, "libdyn_nested: ASSERTION FAILED Cannot reset the currently active simulation\n");
            return;
        }

        libdyn *sim = sim_slots[ slotId ].sim;

        if (sim == NULL) {
            unlock_slots();

            fprintf(stderr, "libdyn_nested: ASSERTION FAILED slot is not in use\n");
            return;
        }

        // reset
        sim->reset_blocks();

    }
    unlock_slots();

}



void libdyn_nested2::event_trigger_mask(int mask)
{
    current_sim->event_trigger_mask(mask);
}

/*
  List of elements
*/

void libdyn_nested2::addElement(char* name, int type, void* userptr)
{
  Element_t e;
  e.type = type; e.userptr = userptr; e.ID = ElementIDcounter;
  std::string s(name);
  Elements.insert(std::make_pair( s, e ) );
  ElementIDcounter++; 
} 

void libdyn_nested2::deleteElement(char* name, int type)
{
  std::string n(name);
  
  Element_map_t::iterator it;
  it = Elements.find(n);
  
  if (it == Elements.end()) {
    throw 1;
  } 

  if (it->second.type != type) {
    // wrong type
    throw 2;
  }

  Elements.erase(it);
}

void* libdyn_nested2::lockupElement(char* name, int type)
{
  std::string n(name);
  
  Element_map_t::iterator it;
  it = Elements.find(n);
  
  if (it == Elements.end()) {
    
    if (this->Parent_SimnestClassPtr == NULL) {
      throw 1;
    }
    
    // lookup in the upper level
    fprintf(stderr, "libdyn_nested2::lockupElement: going t parent simulation\n");
    return this->Parent_SimnestClassPtr->lockupElement(name, type);
  } 
  
  if (it->second.type != type) {
    throw 2;
  }
  fprintf(stderr, "libdyn_nested2::lockupElement: found\n");
  
  return it->second.userptr;
}


 


















//
// class libdyn
//

dynlib_simulation_t* libdyn::get_C_SimulationObject()
{
    return sim;
}

int libdyn::CallSyncCallbackDestructor()
{
    return libdyn_simulation_CallSyncCallbackDestructor(sim);
}

bool libdyn::IsSyncronised()
{

#ifdef DEBUG
    printf("simulation %p is synchronised to %p\n", sim, sim->sync_callback.sync_func);
#endif

    // check for a sync_callback function
    if ( sim->sync_callback.sync_func != NULL )
        return true;
    else
        return false;
}

int libdyn::RunSyncCallbackFn()
{
    int ret = sim->sync_callback.sync_callback_state = (*sim->sync_callback.sync_func)(sim);

    return ret;
}

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

bool libdyn::getSyncState()
{
    return (sim->sync_callback.sync_callback_state == 1);
}


void libdyn::reset_blocks()
{
    libdyn_simulation_resetblocks(sim);
}


int libdyn::irpar_setup(int* ipar, double* rpar, int boxid)
{
    error = libdyn_irpar_setup(ipar, rpar, boxid, &sim, &iocfg);
    if (error < 0) {
        fprintf(stderr, "libdyn_irpar_setup failed\n");
        return error;
    }

    // Make master available, if any
    sim->master = (void*) ld_master; // anonymous copy for "C"-only Code
//     sim->NestedLevel = NestedLevel;

    // Call INIT FLag of all blocks
    error = libdyn_simulation_init(sim);
    if (error < 0) {
        fprintf(stderr, "libdyn_simulation_init failed\n");
        return error;
    }
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

        return NULL;
    }

    return iocfg.outptr[out];
}


void libdyn::libdyn_internal_constructor(int Nin, const int* insizes_, int Nout, const int* outsizes_)
{
    fprintf(stderr, "Setting up new a simulation\n");


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

libdyn::libdyn(int Nin, const int* insizes_, int Nout, const int* outsizes_)
{
    libdyn_internal_constructor(Nin, insizes_, Nout, outsizes_);
}

libdyn::libdyn(int Nin, const int* insizes_, const int*intypes, int Nout, const int* outsizes_, const int *outtypes)
{
    // TODO Datatypes are not considered here
    libdyn_internal_constructor(Nin, insizes_, Nout, outsizes_);
}


// FIXME: geht so nicht
libdyn::libdyn(struct libdyn_io_config_t * iocfg)
{
    error = 0;

//   printf("iocfg: %d %d\n", iocfg->inports, iocfg->outports);

    memcpy((void*) &this->iocfg, (void*) iocfg, sizeof(struct libdyn_io_config_t));

//   printf("iocfg cpy: %d %d\n", this->iocfg.inports, this->iocfg.outports);

    // Initially there is no master
    this->ld_master = NULL;
}


int libdyn::prepare_replacement_sim(int* ipar, double* rpar, int boxid)
{
    // TODO, obsolete by now
}

void libdyn::switch_to_replacement_sim()
{
    // TODO obsolete by now
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

    fprintf(stderr, "libdyn: Successfully destructed simulation\n");
}

//
// Master
//


libdyn_master::libdyn_master()
{
    magic = 0x89abcde2;
    ortd_io = new ortd_io_internal();

    this->realtime_environment = RTENV_UNDECIDED;

    global_comp_func_list = libdyn_new_compfnlist();
//     fprintf(stderr, "Created new libdyn master; ptr=%p\n", this);

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
//   fprintf(stderr, "************* LEGACY VARIANT OF CONSTRUCTOR FOR libdyn_master USED! *******\n");
  
    magic = 0x89abcde2;
    ortd_io = new ortd_io_internal();

    this->realtime_environment = realtime_env;

    global_comp_func_list = libdyn_new_compfnlist();
//     fprintf(stderr, "Created new libdyn master; ptr=%p; tcpport = %d \n", this, remote_control_tcpport);


#ifdef REMOTE

    pmgr = NULL;
    stream_mgr = NULL;
    rts_mgr = NULL;
    dtree = NULL;


    if (init_communication(remote_control_tcpport) <= 0) {   // FIXME: Handle return value
        fprintf(stderr, "WARNING: Running without the communication_server\n");
    }
#endif
}

void libdyn_master::check_memory()
{
    if (magic != 0x89abcde2) {
        fprintf(stderr, "ASSERTION failed: libdyn_master::check_memory: bad magic for object ptr=%p. Propably a memory corruption.\n", this);
        exit(-1);
    }
}


#ifdef REMOTE

int libdyn_master::init_communication(int tcpport)
{

    if (tcpport != 0) {
        // init communication_server

        fprintf(stderr, "Initialising remote control interface on port %d. WARNING: This will become obsolete!\n", tcpport);

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

        fprintf(stderr, "Creating root directory\n");
        dtree = new directory_tree(this->rts_mgr); // Requires rts_mgr

        pmgr = new parameter_manager( rts_mgr, dtree );
        stream_mgr = new ortd_stream_manager(rts_mgr, dtree );

        rts_mgr->start_threads();

    } else {

      fprintf(stderr, "Running without rt_server -- good choice!\n");
      
        // without any tcp server. Now the preferred choice
        this->rts_mgr = NULL;
        dtree = new directory_tree();
    }


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
    
    delete ortd_io;

}

