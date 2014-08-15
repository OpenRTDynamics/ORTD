/*
    Copyright (C) 2014  Christian Klauer

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


#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/*
 * A template for using shared objects
*/


#include "global_shared_object.h"
#include "directory.h"


//
// An object that whose instances are shared between multiple blocks
// . This is executed by including the block ld_I2CDevice_shObj
//
class I2CDevice_SharedObject : public ortd_global_shared_object {
public:
    I2CDevice_SharedObject(const char* identName, dynlib_block_t *block, int Visibility) : ortd_global_shared_object(identName, block->sim, Visibility) {
        this->block = block;
    }
    ~I2CDevice_SharedObject() {
        destruct();
    }

    #define WriteBuffer_size 1000

    
    int init() {
        irpar_string *Fname;

        try {
            // Get the irpar parameters Uipar, Urpar
            libdyn_AutoConfigureBlock_GetUirpar(block, &Uipar, &Urpar);

            // cpp version (nicer), an exception is thrown in case something goes wrong
            Fname = new irpar_string(Uipar, Urpar, 12);
	    irpar_ivec *Par = new irpar_ivec(Uipar, Urpar, 10); // then use:  veccpp.n; veccpp.v;

	    addr = Par->v[0];

	    WriteBuffer_counter = 0;
	    

        } catch(int e) { // check if initialisation went fine
            // deallocate all previously allocated memeory in case something went wrong
            fprintf(stderr, "I2CDeviceBlock: something went wrong while fetching the parameters. Exception = %d\n", e);
            return -1; // indicate an error
        }


        // Open e.g. a device using Fname->s->c_str() as the device name

        // open...
        fprintf(stderr, "I2CDeviceBlock: Open device: %s\n at adress %x", Fname->s->c_str(), addr );
	
// 	FD = 98797;

	// open i2c
        FD = open(Fname->s->c_str(), O_RDWR);
        if (FD < 0) {
            return -1;
        }

        // set i2c device adress
        if (ioctl(FD, I2C_SLAVE, addr) < 0) {
            fprintf(stderr, "I2CDeviceBlock: ioctl failed -- maybe no i2c device?");
            close(FD);
            return -1;
        }

        // Return 0 to indicate that there was no error
        return 0;
    }

    void destruct()
    {
        // e.g. close the device
        close(FD);
    }

    void EnqueToWriteBuffer(uint8_t Byte)
    {
      WriteBuffer[WriteBuffer_counter] = Byte;
      WriteBuffer_counter++;
      if (WriteBuffer_counter >= WriteBuffer_size) {
	 fprintf(stderr, "I2C: WriteBuffer overflow\n");
	 WriteBuffer_counter = 0;
      }
    }
    
    void TransmitBuffer()
    {
      if (write(FD, WriteBuffer, WriteBuffer_counter) != WriteBuffer_counter) {
            perror("I2C: TransmitBuffer");
        }
        WriteBuffer_counter = 0;
    }
    
    void WriteRegisterSingle(uint8_t reg, uint8_t value)
    {
        uint8_t data[2];
        data[0] = reg;
        data[1] = value;
        if (write(FD, data, 2) != 2) {
            perror("SetRegisterSingle");
        }
        
        // i2c_smbus_write_word_data(FD, reg, value);
    }
    
    // write a 16 bit value to a register pair
// write low byte of value to register reg,
// and high byte of value to register reg+1
    void WriteRegisterPair(uint8_t reg, uint16_t value)
    {
        uint8_t data[3];
        data[0] = reg;
        data[1] = value & 0xff;
        data[2] = (value >> 8) & 0xff;
        if (write(FD, data, 3) != 3) {
            perror("SetRegisterPair");
        }
        
        // i2c_smbus_write_word_data(FD, reg, value);
    }

    uint8_t ReadRegister(uint8_t reg)
    {
        uint8_t data[2];
        data[0] = reg;
        if (write(FD, data, 1) != 1) {
            perror("ReadRegister set register");
        }
        if (read(FD, data, 1) != 1) {
            perror("ReadRegister read value");
        }
        return data[0];
    }
    
    
// read a 16 bit value from a register pair
    uint16_t ReadRegisterPair(uint8_t reg)
    {
        uint8_t data[3];
        data[0] = reg;
        if (write(FD, data, 1) != 1) {
            perror("ReadRegisterPair set register");
        }
        if (read(FD, data, 2) != 2) {
            perror("ReadRegisterPair read value");
        }
        return data[0] | (data[1] << 8);
    }

    /*   __s32 SmbusWritebyteData(__u8 command, __u8 value)
       {
         return i2c_smbus_write_byte_data(FD, command, value);
       }
    */


private:
    int *Uipar;
    double *Urpar;
    dynlib_block_t *block;

    //
    //   data of the shared object
    //

    // e.g. a file descriptor
    int FD;
    int addr;

    uint8_t WriteBuffer[WriteBuffer_size];
    uint32_t WriteBuffer_counter;

  
};




//
// An example block for accessing the shared object
//
class I2CDevice_WriteRegister_AccessShObjBlock {
public:
    I2CDevice_WriteRegister_AccessShObjBlock(struct dynlib_block_t *block) {
        this->block = block;    // no nothing more here. The real initialisation take place in init()
    }
    ~I2CDevice_WriteRegister_AccessShObjBlock()
    {
        // free your allocated memory, ...
    }

    //
    // define states or other variables
    //

    I2CDevice_SharedObject *IShObj; // instance of the shared object

    uint8_t register_;
    
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
	    
	    irpar_ivec *Par = new irpar_ivec(Uipar, Urpar, 10); // then use:  veccpp.n; veccpp.v;    
	    register_ = Par->v[0];
	    
            // Obtain the shared object's instance
            if ( ortd_GetSharedObj<I2CDevice_SharedObject>(block, &IShObj) < 0 ) {
                throw 10;
            }

        } catch(int e) { // check if initialisation went fine
            // deallocate all previously allocated memeory in case something went wrong
            fprintf(stderr, "I2CDeviceBlock_WriteRegister: something went wrong. Exception = %d\n", e);
            return -1; // indicate an error
        }

        // set the initial states
        resetStates();


        // Return 0 to indicate that there was no error
        return 0;
    }


    inline void updateStates()
    {
        int32_t *in1 = (int32_t *) libdyn_get_input_ptr(block,0); // the first input port

// 	printf("writing to register %x: %d\n", register_, *in1);
        IShObj->WriteRegisterSingle(register_, *in1);
    }

    inline void calcOutputs()
    {

    }

    inline void resetStates()
    {
    }



    void printInfo() {
        fprintf(stderr, "I'm a I2CDevice block\n");
    }

    // uncommonly used flags
    void PrepareReset() {}
    void HigherLevelResetStates() {}
    void PostInit() {}


    // The Computational function that is called by the simulator
    // and that distributes the execution to the various functions
    // in this C++ - Class, including: init(), io(), resetStates() and the destructor
    static int CompFn(int flag, struct dynlib_block_t *block) {
        return LibdynCompFnTempate<I2CDevice_WriteRegister_AccessShObjBlock>( flag, block ); // this expands a template for a C-comp fn
    }

    // The data for this block managed by the simulator
    struct dynlib_block_t *block;
};


class I2CDevice_BufferWrite_AccessShObjBlock {
public:
    I2CDevice_BufferWrite_AccessShObjBlock(struct dynlib_block_t *block) {
        this->block = block;    // no nothing more here. The real initialisation take place in init()
    }
    ~I2CDevice_BufferWrite_AccessShObjBlock()
    {
        // free your allocated memory, ...
    }

    //
    // define states or other variables
    //

    I2CDevice_SharedObject *IShObj; // instance of the shared object

    uint32_t NBytes;
    
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

            NBytes = libdyn_get_inportsize(block, 0);  // the size of the first (=0) input vector
	    
	    
            // Obtain the shared object's instance
            if ( ortd_GetSharedObj<I2CDevice_SharedObject>(block, &IShObj) < 0 ) {
                throw 1;
            }

        } catch(int e) { // check if initialisation went fine
            // deallocate all previously allocated memeory in case something went wrong
            fprintf(stderr, "I2CDeviceBlock_BufferWrite: something went wrong. Exception = %d\n", e);
            return -1; // indicate an error
        }

        // set the initial states
        resetStates();


        // Return 0 to indicate that there was no error
        return 0;
    }


    inline void updateStates()
    {
        int32_t *in1 = (int32_t *) libdyn_get_input_ptr(block,0); // the first input port

	int i;
	for (i=0; i<NBytes; ++i) {
// 	  printf("eqnue %d\n",  in1[i] );
	  IShObj->EnqueToWriteBuffer(in1[i]);
	}
//         IShObj->WriteRegisterSingle(register_, *in1);
    }

    inline void calcOutputs()
    {

    }

    inline void resetStates()
    {
    }



    void printInfo() {
        fprintf(stderr, "I'm a I2CDevice block\n");
    }

    // uncommonly used flags
    void PrepareReset() {}
    void HigherLevelResetStates() {}
    void PostInit() {}


    // The Computational function that is called by the simulator
    // and that distributes the execution to the various functions
    // in this C++ - Class, including: init(), io(), resetStates() and the destructor
    static int CompFn(int flag, struct dynlib_block_t *block) {
        return LibdynCompFnTempate<I2CDevice_BufferWrite_AccessShObjBlock>( flag, block ); // this expands a template for a C-comp fn
    }

    // The data for this block managed by the simulator
    struct dynlib_block_t *block;
};

class I2CDevice_Transmit_AccessShObjBlock {
public:
    I2CDevice_Transmit_AccessShObjBlock(struct dynlib_block_t *block) {
        this->block = block;    // no nothing more here. The real initialisation take place in init()
    }
    ~I2CDevice_Transmit_AccessShObjBlock()
    {
        // free your allocated memory, ...
    }

    //
    // define states or other variables
    //

    I2CDevice_SharedObject *IShObj; // instance of the shared object

    uint8_t register_;
    
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
            if ( ortd_GetSharedObj<I2CDevice_SharedObject>(block, &IShObj) < 0 ) {
                throw 1;
            }

        } catch(int e) { // check if initialisation went fine
            // deallocate all previously allocated memeory in case something went wrong
            fprintf(stderr, "I2CDeviceBlock_Transmit: something went wrong. Exception = %d\n", e);
            return -1; // indicate an error
        }


        // Return 0 to indicate that there was no error
        return 0;
    }


    inline void updateStates()
    {
//  	printf("Transmit (Flush)\n");
	IShObj->TransmitBuffer();
    }

    inline void calcOutputs()
    {
    }



    void printInfo() {
        fprintf(stderr, "I'm a I2CDevice block\n");
    }

    // uncommonly used flags
    void PrepareReset() {}
    void HigherLevelResetStates() {}
    void PostInit() {}
    inline void resetStates()
    {
    }

    // The Computational function that is called by the simulator
    // and that distributes the execution to the various functions
    // in this C++ - Class, including: init(), io(), resetStates() and the destructor
    static int CompFn(int flag, struct dynlib_block_t *block) {
        return LibdynCompFnTempate<I2CDevice_Transmit_AccessShObjBlock>( flag, block ); // this expands a template for a C-comp fn
    }

    // The data for this block managed by the simulator
    struct dynlib_block_t *block;
};





class I2CDevice_ReadRegister_AccessShObjBlock {
public:
    I2CDevice_ReadRegister_AccessShObjBlock(struct dynlib_block_t *block) {
        this->block = block;    // no nothing more here. The real initialisation take place in init()
    }
    ~I2CDevice_ReadRegister_AccessShObjBlock()
    {
        // free your allocated memory, ...
    }

    //
    // define states or other variables
    //

    I2CDevice_SharedObject *IShObj; // instance of the shared object

    uint8_t register_;
    
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
	    
	    irpar_ivec *Par = new irpar_ivec(Uipar, Urpar, 10); // then use:  veccpp.n; veccpp.v;    
	    register_ = Par->v[0];
	    
            // Obtain the shared object's instance
            if ( ortd_GetSharedObj<I2CDevice_SharedObject>(block, &IShObj) < 0 ) {
                throw 10;
            }

        } catch(int e) { // check if initialisation went fine
            // deallocate all previously allocated memeory in case something went wrong
            fprintf(stderr, "I2CDeviceBlock_WriteRegister: something went wrong. Exception = %d\n", e);
            return -1; // indicate an error
        }

        // set the initial states
        resetStates();


        // Return 0 to indicate that there was no error
        return 0;
    }


    inline void updateStates()
    {
        int32_t *out = (int32_t *) libdyn_get_output_ptr(block,0); 

        uint8_t content = IShObj->ReadRegister(register_);
	out[0] = content;
	
//  	printf("reading from register %x, %d\n", register_, content);
    }

    inline void calcOutputs()
    {

    }

    inline void resetStates()
    {
    }



    void printInfo() {
        fprintf(stderr, "I'm a I2CDevice read block\n");
    }

    // uncommonly used flags
    void PrepareReset() {}
    void HigherLevelResetStates() {}
    void PostInit() {}


    // The Computational function that is called by the simulator
    // and that distributes the execution to the various functions
    // in this C++ - Class, including: init(), io(), resetStates() and the destructor
    static int CompFn(int flag, struct dynlib_block_t *block) {
        return LibdynCompFnTempate<I2CDevice_ReadRegister_AccessShObjBlock>( flag, block ); // this expands a template for a C-comp fn
    }

    // The data for this block managed by the simulator
    struct dynlib_block_t *block;
};








//
// Export to C so the libdyn simulator finds this function
// fn = "I2CDeviceModule_V2" is the folder name of the module
// and the C- function called by the simulator for requesting
// blocks is then "libdyn_module_<fn>_siminit".
// If you're compiling to a module, make sure to also adapt the file
// pluginLoader.cpp
//
extern "C" {

// CHANGE HERE: Adjust this function name to match the name of your module
//              The string "I2CDeviceModule_V2" must be exchanged
    int libdyn_module_I2C_siminit(struct dynlib_simulation_t *sim, int bid_ofs)
    {
        //
        // Register all blocks to the given simulation "sim", bid_ofs has no meaning by now.
        // All comp. functions for all blocks are added to a list
        //


        // Read the configuration file for the block ids
        int blockid =
#include "blockid_start.conf.tmp"
            ;


        libdyn_compfnlist_add(sim->private_comp_func_list, blockid+10, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &SharedObjBlock<I2CDevice_SharedObject>::CompFn);

        // shared object accessing block
        libdyn_compfnlist_add(sim->private_comp_func_list, blockid+11, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &I2CDevice_WriteRegister_AccessShObjBlock::CompFn);
        libdyn_compfnlist_add(sim->private_comp_func_list, blockid+12, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &I2CDevice_BufferWrite_AccessShObjBlock::CompFn);
        libdyn_compfnlist_add(sim->private_comp_func_list, blockid+13, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &I2CDevice_Transmit_AccessShObjBlock::CompFn);
	libdyn_compfnlist_add(sim->private_comp_func_list, blockid+14, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &I2CDevice_ReadRegister_AccessShObjBlock::CompFn);
	
//         libdyn_compfnlist_add(sim->private_comp_func_list, blockid+12, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &I2CDevice_ReadRegister_AccessShObjBlock::CompFn);

#ifdef DEBUG
        printf("module I2CDevice is initialised\n");
#endif  


    }


}
