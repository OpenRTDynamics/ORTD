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

  Do a search of "RTCrossCorr" and replace it with your own name.

*/



class RTCrossCorrBlock {
public:
    RTCrossCorrBlock(struct dynlib_block_t *block) {
        this->block = block;    // no nothing more here. The real initialisation take place in init()
    }
    ~RTCrossCorrBlock()
    {
        free(RingBuf);
    }

    //
    // define states or other variables
    //

    double *RingBuf;
    int BufferCounter;
    int NBytes, N;

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
        int error = 0;


//         //
//         // get a vector of integers (double vectors are similar, replace ivec with rvec)
//         //
//         struct irpar_ivec_t vec;
//         if ( irpar_get_ivec(&vec, Uipar, Urpar, 11) < 0 ) {return -1; }
//

        //
        // get some informations on the first input port
        //
        N = libdyn_get_inportsize(block, 0);  // the size of the input vector
        int datatype = libdyn_get_inportdatatype(block, 0); // the datatype
        int TypeBytes = libdyn_config_get_datatype_len(datatype); // number of bytes allocated for one element of type "datatype"
        NBytes = N * TypeBytes;  // Amount of bytes allocated for the input vector

// 	if ( libdyn_get_inportsize(block, 0) != libdyn_get_inportsize(block, 1) )
// 	  return -1;

        // Allocate Ringbuf
        RingBuf = (double*) malloc(NBytes);

        // set the initial states
        resetStates();

// 	printf("N=%d\n", N);

        // Return -1 to indicate an error, so the simulation will be destructed
        return error;
    }


    inline void updateStates()
    {
    }


    inline void calcOutputs()
    {
        double *in1 = (double *) libdyn_get_input_ptr(block,0); // the first input port
        double *output = (double*) libdyn_get_output_ptr(block, 0); // the first output port
        double *in2 = (double *) libdyn_get_input_ptr(block,1); // the 2nd input port

        // fill in buffer
        RingBuf[BufferCounter] = *in2;

// 	printf("------- BufferCounter %d\n", BufferCounter);
        //
        double sum=0;
        int i;
        int input_i = N-1; // index of the input element to use

        // go from BufferCounter back to the begin
        for (i=BufferCounter; i>=0; --i, --input_i) {
            sum += in1[input_i] * RingBuf[i]; // calc
// 	  printf("sum += in1[%d] * RingBuf[%d]\n", input_i, i);
        }

        // go from the end of BufferCounter back until input_i is zero
// 	printf("Remaining elements %d+1\n", input_i);
        for (i=N-1; input_i >= 0 ; --i, --input_i) {
            sum += in1[input_i] * RingBuf[i]; // calc
// 	  printf("sum += in1[%d] * RingBuf[%d]\n", input_i, i);
        }

        // fill in output val
        *output = sum;

        // increase buffer counter
        BufferCounter++;
        if (BufferCounter >= N) {
            BufferCounter = 0;
// 	  printf("Reset BufferCounter\n");
        }

    }


    inline void resetStates()
    {
        memset(RingBuf, 0, NBytes);
        BufferCounter = 0;
    }



    void printInfo() {
        fprintf(stderr, "I'm a RTCrossCorr block\n");
    }

    // uncommonly used flags
    void PrepareReset() {}
    void HigherLevelResetStates() {}
    void PostInit() {}


    // The Computational function that is called by the simulator
    // and that distributes the execution to the various functions
    // in this C++ - Class, including: init(), io(), resetStates() and the destructor
    static int CompFn(int flag, struct dynlib_block_t *block) {
        return LibdynCompFnTempate<RTCrossCorrBlock>( flag, block ); // this expands a template for a C-comp fn
    }

    // The data for this block managed by the simulator
    struct dynlib_block_t *block;
};




class RTShapeCompareBlock {
public:
    RTShapeCompareBlock(struct dynlib_block_t *block) {
        this->block = block;    // no nothing more here. The real initialisation take place in init()
    }
    ~RTShapeCompareBlock()
    {
        free(RingBuf);
    }

    //
    // define states or other variables
    //

    double *RingBuf;
    int BufferCounter;
    int NBytes, N;

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
        int error = 0;


//         //
//         // get a vector of integers (double vectors are similar, replace ivec with rvec)
//         //
//         struct irpar_ivec_t vec;
//         if ( irpar_get_ivec(&vec, Uipar, Urpar, 11) < 0 ) {return -1; }
//

        //
        // get some informations on the first input port
        //
        N = libdyn_get_inportsize(block, 0);  // the size of the input vector
        int datatype = libdyn_get_inportdatatype(block, 0); // the datatype
        int TypeBytes = libdyn_config_get_datatype_len(datatype); // number of bytes allocated for one element of type "datatype"
        NBytes = N * TypeBytes;  // Amount of bytes allocated for the input vector

// 	if ( libdyn_get_inportsize(block, 0) != libdyn_get_inportsize(block, 1) )
// 	  return -1;

        // Allocate Ringbuf
        RingBuf = (double*) malloc(NBytes);

        // set the initial states
        resetStates();

// 	printf("N=%d\n", N);

        // Return -1 to indicate an error, so the simulation will be destructed
        return error;
    }


    inline void updateStates()
    {
    }


    inline void calcOutputs()
    {
        double *in1 = (double *) libdyn_get_input_ptr(block,0); // the first input port
        double *output = (double*) libdyn_get_output_ptr(block, 0); // the first output port
        double *in2 = (double *) libdyn_get_input_ptr(block,1); // the 2nd input port

        // fill in buffer
        RingBuf[BufferCounter] = *in2;

// 	printf("------- BufferCounter %d\n", BufferCounter);
        //
        double sum=0;
        int i;
        int input_i = N-1; // index of the input element to use

        // go from BufferCounter back to the begin
        for (i=BufferCounter; i>=0; --i, --input_i) {
            sum += in1[input_i] * RingBuf[i]; // calc
// 	  printf("sum += in1[%d] * RingBuf[%d]\n", input_i, i);
        }

        // go from the end of BufferCounter back until input_i is zero
// 	printf("Remaining elements %d+1\n", input_i);
        for (i=N-1; input_i >= 0 ; --i, --input_i) {
            sum += in1[input_i] * RingBuf[i]; // calc
// 	  printf("sum += in1[%d] * RingBuf[%d]\n", input_i, i);
        }

        // fill in output val
        *output = sum;

        // increase buffer counter
        BufferCounter++;
        if (BufferCounter >= N) {
            BufferCounter = 0;
// 	  printf("Reset BufferCounter\n");
        }

    }


    inline void resetStates()
    {
        memset(RingBuf, 0, NBytes);
        BufferCounter = 0;
    }



    void printInfo() {
        fprintf(stderr, "I'm a RTShapeCompareBlock block\n");
    }

    // uncommonly used flags
    void PrepareReset() {}
    void HigherLevelResetStates() {}
    void PostInit() {}


    // The Computational function that is called by the simulator
    // and that distributes the execution to the various functions
    // in this C++ - Class, including: init(), io(), resetStates() and the destructor
    static int CompFn(int flag, struct dynlib_block_t *block) {
        return LibdynCompFnTempate<RTShapeCompareBlock>( flag, block ); // this expands a template for a C-comp fn
    }

    // The data for this block managed by the simulator
    struct dynlib_block_t *block;
};














// Read from ascii file
class ReadAsciiFileBlock {
public:
    ReadAsciiFileBlock(struct dynlib_block_t *block) {
        this->block = block;    // no nothing more here. The real initialisation take place in init()
    }
    ~ReadAsciiFileBlock()
    {
        // free your allocated memory, ...
        fclose(fd);
    }

    //
    // define states or other variables
    //

    int Len;
    int N;
    int datatype;
    FILE *fd;


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
        int error = 0;

        //
        // get a string (not so nice by now)
        //
        struct irpar_ivec_t str_;
        char *str;
        if ( irpar_get_ivec(&str_, Uipar, Urpar, 12) < 0 ) error = -1 ;
        irpar_getstr(&str, str_.v, 0, str_.n);

        printf("Reading ascii file %s\n", str);

        // Open the file

        fd = fopen ( str, "r" );
        if (fd == NULL) {
            printf("ERROR: cannot open file %s\n", str);
            free(str);
            return -1;
        }

        free(str); // do not forget to free the memory allocated by irpar_getstr



        //
        // get some information on the first input port
        //
        N = libdyn_get_outportsize(block, 0);  // the size of the input vector
        datatype = libdyn_get_outportdatatype(block, 0); // the datatype
        int TypeBytes = libdyn_config_get_datatype_len(datatype); // number of bytes allocated for one element of type "datatype"
        int NBytes = N * TypeBytes;  // Amount of bytes allocated for the input vector

        // set the initial states
        resetStates();

        // Return -1 to indicate an error, so the simulation will be destructed
        return error;
    }


    inline void updateStates()
    {
    }


    inline void calcOutputs()
    {
        double *output = (double*) libdyn_get_output_ptr(block, 0); // the first output port

	// Read N values from the file and copy them to the output vector
	int i;
        for (i=0; i<N; ++i) {
            double oneElement;
            int ret = fscanf(fd,"%lf",&oneElement);
	    if (ret != 1)
	      break;
            output[i] = oneElement;
        }

    }


    inline void resetStates()
    {
    }


    void printInfo() {
        fprintf(stderr, "I'm a ReadAsciiFile block\n");
    }

    // uncommonly used flags
    void PrepareReset() {}
    void HigherLevelResetStates() {}
    void PostInit() {}

    // The Computational function that is called by the simulator
    // and that distributes the execution to the various functions
    // in this C++ - Class, including: init(), io(), resetStates() and the destructor
    static int CompFn(int flag, struct dynlib_block_t *block) {
        return LibdynCompFnTempate<ReadAsciiFileBlock>( flag, block ); // this expands a template for a C-comp fn
    }

    // The data for this block managed by the simulator
    struct dynlib_block_t *block;
};






//
// Export to C so the libdyn simulator finds this function
// fn = "RTCrossCorrModule_V2" is the folder name of the module
// and the C- function called by the simulator for requesting
// blocks is then "libdyn_module_<fn>_siminit".
// If you're compiling to a module, make sure to also adapt the file
// pluginLoader.cpp
//
extern "C" {

// CHANGE HERE: Adjust this function name to match the name of your module
    int libdyn_module_basic_ldblocks_siminit_Cpp(struct dynlib_simulation_t *sim, int bid_ofs)
    {
        //
        // Register all blocks to the given simulation "sim", bid_ofs has no meaning by now.
        // All comp. functions for all blocks are added to a list
        //

        int blockid = 60001;

        libdyn_compfnlist_add(sim->private_comp_func_list, blockid+300, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &RTCrossCorrBlock::CompFn);
	libdyn_compfnlist_add(sim->private_comp_func_list, blockid+301, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &ReadAsciiFileBlock::CompFn);
	libdyn_compfnlist_add(sim->private_comp_func_list, blockid+302, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &RTShapeCompareBlock::CompFn);
	
	

	
    }


}
