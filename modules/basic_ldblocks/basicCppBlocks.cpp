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

#include <cstring>


/*

  This is a template for creating a new ORTD Block's computational function in C++

  Do a search of "RTCrossCorr" and replace it with your own name.

*/




class ArrayInt32Block {
public:
    ArrayInt32Block(struct dynlib_block_t *block) {
        this->block = block;    // no nothing more here. The real initialisation take place in init()
    }
    ~ArrayInt32Block()
    {
        destruct();
    }

    
    //
    // define states or other variables
    //


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
	
// 	// cpp version (nicer), an exception is thrown in case something goes wrong
// 	s = new irpar_string(Uipar, Urpar, 12);
//  	printf("cppstr = %s\n", s->s->c_str());
	
	
        //
        // get a vector of integers (double vectors are similar, replace ivec with rvec)
        //
		
	// c++ version (nicer), an exception is thrown in case something goes wrong

	  Array = new irpar_ivec(Uipar, Urpar, 10); // then use:  veccpp.n; veccpp.v;
// 	  printf("****** n=%d veccpp[0] = %d\n",Array->n, Array->v[0]); // print the first element 
// 	                                           // of the vector that is of size veccpp.n	
	
	
	
        // set the initial states
        resetStates();

      
         } catch(int e) { // check if initialisation went fine
            // deallocate all previously allocated memeory in case something went wrong
            fprintf(stderr, "ArrayInt32Block: something went wrong. Exception = %d\n", e);
            destruct(); // free all memory allocated by now.
            return -1; // indicate an error
        }

        // Return 0 to indicate that there was no error
        return 0;
    }


    inline void updateStates()
    {
    }

    inline void calcOutputs()
    {
        int32_t *in1 = (int32_t *) libdyn_get_input_ptr(block,0); // the first input port
        int32_t *output = (int32_t*) libdyn_get_output_ptr(block, 0); // the first output port

// 	printf("in %d %d\n", *in1, Array->n);
	
	int32_t tmp = *in1;
	
	if (tmp < 0) tmp = 0;	
	if (tmp >= Array->n) tmp = Array->n - 1;
	
        int32_t y = Array->v[tmp];
	   *output = y;
	   
    }

    inline void resetStates()
    {
    }

    void printInfo() {
        fprintf(stderr, "I'm a ArrayInt32Block block\n");
    }

    // uncommonly used flags
    void PrepareReset() {}
    void HigherLevelResetStates() {}
    void PostInit() {}


    // The Computational function that is called by the simulator
    // and that distributes the execution to the various functions
    // in this C++ - Class, including: init(), io(), resetStates() and the destructor
    static int CompFn(int flag, struct dynlib_block_t *block) {
        return LibdynCompFnTempate<ArrayInt32Block>( flag, block ); // this expands a template for a C-comp fn
    }

    // The data for this block managed by the simulator
    struct dynlib_block_t *block;
};






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
	free(filename); // free the memory allocated by irpar_getstr  
    }

    //
    // define states or other variables
    //

    int Len;
    int N;
    int datatype;
    FILE *fd;
    char *filename;


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
        
        if ( irpar_get_ivec(&str_, Uipar, Urpar, 12) < 0 ) error = -1 ;
		
	if (error==-1) {
	  fprintf(stderr, "ReadAsciiFileBlock: Error while loading parameters\n");
	  return -1;	  
	}

	
        irpar_getstr(&filename, str_.v, 0, str_.n);
	
        printf("ReadAsciiFileBlock: Reading ascii file %s\n", filename);

        // Open the file

        fd = fopen ( filename, "r" );
        if (fd == NULL) {
            printf("ReadAsciiFileBlock: ERROR: cannot open file %s\n", filename);
            free(filename);
            return -1;
        }

        

        



        //
        // get some information on the first input port
        //
        N = libdyn_get_outportsize(block, 0);  // the size of the input vector
        datatype = libdyn_get_outportdatatype(block, 0); // the datatype
        int TypeBytes = libdyn_config_get_datatype_len(datatype); // number of bytes allocated for one element of type "datatype"
        int NBytes = N * TypeBytes;  // Amount of bytes allocated for the input vector
		printf("E\n");

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


class ConstBinBlock {
public:
    ConstBinBlock(struct dynlib_block_t *block) {
        this->block = block;    // no nothing more here. The real initialisation take place in init()
    }
    ~ConstBinBlock()
    {
        // free your allocated memory, ...
    }

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

        irpar_ivec veccpp(Uipar, Urpar, 10);
        int insize = veccpp.v[0];

	//        printf("Insize = %d\n", insize);

//         irpar_string s(Uipar, Urpar, 11);
//         const char *input  = s.s->c_str() ;
//        printf("Binary = %s\n", input);
	
        irpar_ivec binary(Uipar, Urpar, 11);
        //int insize = binary.v[0];
	
	

        char *output = (char*) libdyn_get_output_ptr(block, 0); // the first output port

//  	  printf("copy %d bytes\n", insize );
//         std::strcpy( output, input);
	int i;
	for (i=0; i<binary.n; ++i) {
	  output[i] = (char) binary.v[i];
	}
	  
	// memcpy((void*) output, (void*) , s);

//        printf("Output = %s\n", output);

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

    }


    inline void resetStates()    {    }

    void printInfo() {
        fprintf(stderr, "I'm a ConstBin block\n");
    }

    // uncommonly used flags
    void PrepareReset() {}
    void HigherLevelResetStates() {}
    void PostInit() {}


    // The Computational function that is called by the simulator
    // and that distributes the execution to the various functions
    // in this C++ - Class, including: init(), io(), resetStates() and the destructor
    static int CompFn(int flag, struct dynlib_block_t *block) {
        return LibdynCompFnTempate<ConstBinBlock>( flag, block ); // this expands a template for a C-comp fn
    }

    // The data for this block managed by the simulator
    struct dynlib_block_t *block;
};



#include "io.h"

// Read from ascii file
class ORTDIO_PutBuffer {
public:
    ORTDIO_PutBuffer(struct dynlib_block_t *block) {
        this->block = block;    // no nothing more here. The real initialisation take place in init()
    }
    ~ORTDIO_PutBuffer()
    {
        // free your allocated memory, ...
	free(headerstr); // free the memory allocated by irpar_getstr  
    }

    //
    // define states or other variables
    //

    int Len;
    int N;
    int datatype;
    char *headerstr;
    size_t headerstr_len;
    int NBytes;


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
        
        if ( irpar_get_ivec(&str_, Uipar, Urpar, 12) < 0 ) error = -1 ;
		
	if (error==-1) {
	  fprintf(stderr, "ORTDIO_PutBuffer: Error while loading parameters\n");
	  return -1;	  
	}
	
        irpar_getstr(&headerstr, str_.v, 0, str_.n);
	headerstr_len = str_.n;

        //
        // get some information on the first input port
        //
        N = libdyn_get_inportsize(block, 0);  // the size of the input vector
        datatype = libdyn_get_inportdatatype(block, 0); // the datatype
        int TypeBytes = libdyn_config_get_datatype_len(datatype); // number of bytes allocated for one element of type "datatype"
        NBytes = N * TypeBytes;  // Amount of bytes allocated for the input vector

        // set the initial states
        resetStates();

        // Return -1 to indicate an error, so the simulation will be destructed
        return error;
    }


    inline void updateStates()
    {
      
              char *in = (char*) libdyn_get_input_ptr(block, 0); // the first output port
	
	ortd_io::PutBuffer( block->sim, (char*) headerstr, headerstr_len,  (char*) in, NBytes );

    }


    inline void calcOutputs()
    {

    }


    inline void resetStates()
    {
    }


    void printInfo() {
        fprintf(stderr, "I'm a ORTDIO_PutBuffer block\n");
    }

    // uncommonly used flags
    void PrepareReset() {}
    void HigherLevelResetStates() {}
    void PostInit() {}

    // The Computational function that is called by the simulator
    // and that distributes the execution to the various functions
    // in this C++ - Class, including: init(), io(), resetStates() and the destructor
    static int CompFn(int flag, struct dynlib_block_t *block) {
        return LibdynCompFnTempate<ORTDIO_PutBuffer>( flag, block ); // this expands a template for a C-comp fn
    }

    // The data for this block managed by the simulator
    struct dynlib_block_t *block;
};









class ld_SyncFilewrite {
public:
    ld_SyncFilewrite(struct dynlib_block_t *block) {
        this->block = block;    // no nothing more here. The real initialisation take place in init()
    }
    ~ld_SyncFilewrite()
    {
        // free your allocated memory, ...
	free(fname); // free the memory allocated by irpar_getstr  
    }

    //
    // define states or other variables
    //

    int Len;
    int N;
    int datatype;
    char *fname;
    size_t len_fname;
    int NBytes;
    int32_t lastTrigger;
    FILE *fd;
    bool FileOpen;
    
    int WrittenLines;


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
        
	if ( irpar_get_ivec(&str_, Uipar, Urpar, 12) < 0 ) error = -1 ;
		
	if (error==-1) {
	  fprintf(stderr, "ld_SyncFilewrite: Error while loading parameters\n");
	  return -1;	  
	}


        irpar_getstr(&fname, str_.v, 0, str_.n);
	len_fname = str_.n;

        //
        // get some information on the first input port
        //
        N = libdyn_get_inportsize(block, 0);  // the size of the input vector
        datatype = libdyn_get_inportdatatype(block, 0); // the datatype
        Len = libdyn_config_get_datatype_len(datatype); // number of bytes allocated for one element of type "datatype"
        NBytes = N * Len;  // Amount of bytes allocated for the input vector
        
        // set the initial states
        FileOpen = false;
        resetStates();
	

        // Return -1 to indicate an error, so the simulation will be destructed
        return error;
    }
    
    void StartWriting() {
     // 	printf("staw\n");
	if (!FileOpen) {

          fprintf(stderr, "ld_SyncFilewrite: open logfile %s\n", fname);
	  fd = fopen ( fname, "w" );
	  if (fd == NULL) {
	    fprintf(stderr, "ERROR: ld_SyncFilewrite: cannot open file\n");
	  }
	  WrittenLines = 0;
	  
	  FileOpen = true;
	}
      
    }
    
    void StopWriting() {
   //   	printf("stow\n");

      if (FileOpen) {
	fclose(fd);
      	fprintf("ld_SyncFilewrite: closed logfile after writing %d lines of data\n", WrittenLines);
      } 
      FileOpen = false;      
      
    }


    inline void updateStates()
    {
      
              void *in = (void *) libdyn_get_input_ptr(block, 0); // the first output port
	      int32_t *trigger = (int32_t *) libdyn_get_input_ptr(block, 1); // the first output port
	      
	      if (*trigger > 0) {

		if (!FileOpen) {
	  	  StartWriting();
	        }

		
		if (FileOpen) {
			  //printf("w...\n");

		  // write data
		  if (datatype == DATATYPE_FLOAT) {
		    double *fin = (double*) in;
		    
		    int i;
		    
		    for (i = 0; i < N; ++i) {
		      fprintf(fd, "%f ", fin[i]);
		    }
		    fprintf(fd, "\n");
		    
		    WrittenLines += 1;
		    
		  } else {
		      fprintf(stderr, "ld_SyncFilewrite: Datatype not supported!\n");
		  }
		  
		
	        } 
	        
	        
	      } else if (FileOpen && *trigger == 0) {
		StopWriting();
	      }
	      
	      
	      
	      
	      
	
	// ortd_io::PutBuffer( block->sim, (char*) headerstr, headerstr_len,  (char*) in, NBytes );

    }


    inline void calcOutputs()
    {

    }


    inline void resetStates()
    {
      StopWriting();
      
    }


    void printInfo() {
        fprintf(stderr, "I'm a ld_SyncFilewrite block\n");
    }

    // uncommonly used flags
    void PrepareReset() {}
    void HigherLevelResetStates() {}
    void PostInit() {}

    // The Computational function that is called by the simulator
    // and that distributes the execution to the various functions
    // in this C++ - Class, including: init(), io(), resetStates() and the destructor
    static int CompFn(int flag, struct dynlib_block_t *block) {
        return LibdynCompFnTempate<ld_SyncFilewrite>( flag, block ); // this expands a template for a C-comp fn
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
	
	libdyn_compfnlist_add(sim->private_comp_func_list, blockid+303, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &ArrayInt32Block::CompFn);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid+304, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &ConstBinBlock::CompFn);
    	libdyn_compfnlist_add(sim->private_comp_func_list, blockid+305, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &ORTDIO_PutBuffer::CompFn);
	
    	libdyn_compfnlist_add(sim->private_comp_func_list, blockid+306, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &ld_SyncFilewrite::CompFn);
	
	

    
    
    }


}
