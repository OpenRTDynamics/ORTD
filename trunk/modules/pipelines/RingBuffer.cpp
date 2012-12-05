/*
    Copyright (C) 2012  Christian Klauer

    This file is part of OpenRTDynamics, the Real Time Dynamics Toolbox

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

/*
  zwei Typen von Blöcken:

    * Ersteller Block: Hier wird größe und datentyp festgelegt. Er erstellt die Instanz der Klasse RingBuffer,
      sobald PREINIT aufgerufen wird (bevor andere Blöcke initialisiert werden). EIn directory Eintrag wird angelegt
    * Blöcke, die hierauf über das directory zugreifen
    * Falls der usage counter von RingBuffer wieder null ist wird die Instanz von RingBuffer wieder zerstört


*/



#include <malloc.h>


extern "C" {
#include "libdyn.h"
#include "libdyn_scicos_macros.h"
#include <math.h>
#include "irpar.h"

}
#include <libdyn_cpp.h>
#include "global_shared_object.h"

#include "directory.h"




extern "C" {
    extern int RingBuffer_block(int flag, struct dynlib_block_t *block);
    extern int write_RingBuffer_block(int flag, struct dynlib_block_t *block);
    extern int read_RingBuffer_block(int flag, struct dynlib_block_t *block);
};






// Shared object that manages the ringbuffer
// 
// FIXME: The locking mechanism could be better somehow like in log.c
// 

class RingBuffer_shobj : public ortd_global_shared_object {
public:
    RingBuffer_shobj(const char* identName, libdyn_master* master, int datatype, int len, bool UnusedParam) : ortd_global_shared_object(identName, master) {
        this->datatype = datatype;
        this->NumElements = len;
        this->UnusedParam = UnusedParam;


        this->data_element_size = libdyn_config_get_datatype_len(datatype);
        this->memsize = this->data_element_size*this->NumElements;

        this->ReadCounter = 0;
        this->WriteCounter = 0;
        this->ElementsPending = 0;

//         printf("init RingBuffer. Memsize = %d\n", memsize);

        // allocate data
        data = (uint8_t *) malloc(memsize);	memset(data, 0, memsize);
	
//         if (initial_data != NULL)
//             memcpy(data, initial_data, memsize);
//         else
//             memset(data, 0, memsize);

        // initialise mutex
        pthread_mutex_init(&data_mutex, NULL);
        

    }

    void* get_dataptr() {
        return data;
    }
    void lock_data() {
        fprintf(stderr, "Try lock...\n");
        pthread_mutex_lock(&data_mutex);
        fprintf(stderr, "...locked\n");
    }
    void unlock_data() {
        fprintf(stderr, "Try unlock...\n");
        pthread_mutex_unlock(&data_mutex);
	fprintf(stderr, "...unlocked\n");
    }
    void set_wholedata(void *src) { // UNUSED
        lock_data();
        memcpy(data, src, memsize);
        unlock_data();
    }
    bool insertElements(int numElementsToWrite, void *src) {
        lock_data();
        {
            bool overflow_check = ElementsPending == NumElements;
            int freeSlots = NumElements - ElementsPending;

            if (freeSlots < numElementsToWrite) {
                fprintf(stderr, "ringbuffer overflow\n");
                return false; // Buf full
            }

            int i;
            for (i = 0; i < numElementsToWrite; ++i) { // for each element to be written

                // get current write position
                int ofs = WriteCounter * data_element_size;

                // copy one element
                uint8_t* element_ptr = (uint8_t*) src + i*data_element_size; // calc position of src element
                memcpy( (void *) &data[ofs], (void*) element_ptr, data_element_size);

// 		fprintf(stderr, "writer: writing byte ofs=%d, ptr=%p \n", ofs, (void *) &data[ofs]);
// 		printf("Write: %f\n", ((double*)src)[0]);
		

                // increase write counter; take care of the end of the buffer
                WriteCounter++;
                if (WriteCounter >= NumElements) // wrap to first position when end is reached
                    WriteCounter = 0;

            }

            ElementsPending += numElementsToWrite;
        }
        unlock_data();

        return true;
    }
    bool GetElementCopy(void *dst) {
        lock_data();
        {

            if (ElementsPending < 1) {
                fprintf(stderr, "Buffer Empty\n");
                return false;
            }

            // at least one element available
            uint memofs = ReadCounter * data_element_size; // calc position in buffer
            memcpy(dst, (void *) &data[memofs], data_element_size);
	    
// 	    fprintf(stderr, "reader: reading byte ofs=%d, ptr=%p \n", memofs, (void *) &data[memofs]);

            ReadCounter++;
            if (ReadCounter >= NumElements) // read_cnt zurücksetzen
                ReadCounter = 0;

            ElementsPending--;

//             fprintf(stderr, "data read: read_cnt=%d, write_cnt=%d, stored ele=%d\n", ReadCounter, WriteCounter, ElementsPending);
// printf("Read: %f\n", ((double*)dst)[0]);

        }
        unlock_data();

        return true;

    }
    bool getuseMutex() {
        return UnusedParam;
    }


    ~RingBuffer_shobj() {
        free(data);

        if (UnusedParam) {
            pthread_mutex_destroy(&data_mutex);
        }

    }
public:
    uint data_element_size;
    uint NumElements;
    int datatype;

private:
    uint8_t *data;
    uint WriteCounter, ReadCounter;
    uint ElementsPending;

    bool UnusedParam;

    uint memsize;

    pthread_mutex_t data_mutex;
};






class RingBuffer_block_class {
public:
    RingBuffer_block_class(struct dynlib_block_t *block)    {
        this->block = block;
    }
    void destruct() {
        free(directory_entry_fname);
        if (pmem->isUnused()) {
            delete pmem;
        } else {
#ifdef DEBUG
            fprintf(stderr, "RingBuffer_block_class: object is still in use and will therefore remain in memory\n");
#endif
        }
    }

    void io(int update_states) {};
    void reset() {};
    int init() {
        double *rpar = libdyn_get_rpar_ptr(block);
        int *ipar = libdyn_get_ipar_ptr(block);

        this->datatype = ipar[1];
        this->size = ipar[2];
        this->UnusedParam = ipar[3]; // unused

        int len_ident_str = ipar[10];
        irpar_getstr(&this->directory_entry_fname, ipar, 11, len_ident_str);

#ifdef DEBUG
        fprintf(stderr, "RingBuffer_block_class: datatype %d size %d indentStr %s\n", this->datatype, this->size, this->directory_entry_fname);
#endif

        libdyn_master *master = (libdyn_master *) block->sim->master;

        // FIXME: catch exception herer
        pmem = new RingBuffer_shobj( directory_entry_fname, master, this->datatype, this->size, this->UnusedParam );

        // if error return -1;!!!!


// 	fprintf(stderr,"pmem ptr = %p\n", pmem);

        return 0;
    }

private:
    struct dynlib_block_t *block;

    char * directory_entry_fname;

    int datatype;
    int size;
    int UnusedParam;

    RingBuffer_shobj  *pmem;
};

int RingBuffer_block(int flag, struct dynlib_block_t *block)
{

//     printf("RingBuffer_block: flag==%d\n", flag);

    double *in;
    double *rpar = libdyn_get_rpar_ptr(block);
    int *ipar = libdyn_get_ipar_ptr(block);

    int Nin = 0;
    int Nout = 0;


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        in = (double *) libdyn_get_input_ptr(block,0);
        RingBuffer_block_class *worker = (RingBuffer_block_class *) libdyn_get_work_ptr(block);

        worker->io(0);
    }
    return 0;
    break;
    case COMPF_FLAG_UPDATESTATES:
    {
        in = (double *) libdyn_get_input_ptr(block,0);
        RingBuffer_block_class *worker = (RingBuffer_block_class *) libdyn_get_work_ptr(block);

        worker->io(1);

    }
    return 0;
    break;
    case COMPF_FLAG_CONFIGURE:  // configure. NOTE: do not reserve memory or open devices. Do this while init instead!
    {
        int datatype = ipar[1];
        int size = ipar[2];

        libdyn_config_block(block, BLOCKTYPE_STATIC, Nout, Nin, (void *) 0, 0);
    }
    return 0;
    break;
    case COMPF_FLAG_PREINIT:  // pre- init
    {
        RingBuffer_block_class *worker = new RingBuffer_block_class(block);
        libdyn_set_work_ptr(block, (void*) worker);

        int ret = worker->init();
        if (ret < 0)
            return -1;
    }
    return 0;
    break;
    case COMPF_FLAG_RESETSTATES:
    {
        in = (double *) libdyn_get_input_ptr(block,0);
        RingBuffer_block_class *worker = (RingBuffer_block_class *) libdyn_get_work_ptr(block);

        worker->reset();
    }
    return 0;
    break;
//     case COMPF_FLAG_HIGHERLEVELRESET:
//     {
//         in = (double *) libdyn_get_input_ptr(block,0);
//         RingBuffer_block_class *worker = (RingBuffer_block_class *) libdyn_get_work_ptr(block);
//
//         worker->io(1);
//     }
//     return 0;
//     break;
    case COMPF_FLAG_PREINITUNDO: // destroy instance
    {
        RingBuffer_block_class *worker = (RingBuffer_block_class *) libdyn_get_work_ptr(block);

        worker->destruct();
        delete worker;

    }
    return 0;
    break;
    case COMPF_FLAG_DESTUCTOR: // destroy instance
    {
        RingBuffer_block_class *worker = (RingBuffer_block_class *) libdyn_get_work_ptr(block);

        worker->destruct();
        delete worker;
    }
    return 0;
    break;
    case COMPF_FLAG_PRINTINFO:
        printf("I'm a RingBuffer initialising block\n");
        return 0;
        break;

    }
}
















class RingBuffer_write_block_class {
public:
    RingBuffer_write_block_class(struct dynlib_block_t *block)    {
        this->block = block;
    }
    void destruct() {        }

    void io_output() {};
    void io_update() {
        void *dataToWrite = (void *) libdyn_get_input_ptr(block,0);

        pmem->insertElements( elements_to_write, dataToWrite );

    };
    void reset() {};
    int init() {
        double *rpar = libdyn_get_rpar_ptr(block);
        int *ipar = libdyn_get_ipar_ptr(block);

        int datatype = ipar[1];
        this->elements_to_write = ipar[2]; // size of data input

        int len_ident_str = ipar[10];
        char * directory_entry_fname;
        irpar_getstr(&directory_entry_fname, ipar, 11, len_ident_str);

#ifdef DEBUG
        fprintf(stderr, "write_RingBuffer_block: datatype %d size to write %d indentStr %s\n", datatype, this->elements_to_write, directory_entry_fname);
#endif

        libdyn_master *master = (libdyn_master *) block->sim->master;

        // FIXME: need to make sure that this is really a memory object and not something else
        pmem = (RingBuffer_shobj*) get_ortd_global_shared_object(directory_entry_fname, master);

        if (pmem == NULL) {
            fprintf(stderr, "ERROR: write_RingBuffer_block: memory <%s> could not be found\n", directory_entry_fname);
            free(directory_entry_fname);
            return -1;
        }
        free(directory_entry_fname);

        // check consistency
        if (pmem->datatype != datatype) {
            fprintf(stderr, "ERROR: write_RingBuffer_block: missmatching datatype for memory <%s>\n", directory_entry_fname);
            return -1;
        }

        if (pmem->NumElements < this->elements_to_write) {
            fprintf(stderr, "ERROR: write_RingBuffer_block: elements_to_write is bigger than the number of elements in memory <%s>\n", directory_entry_fname);
            return -1;
        }


// 	fprintf(stderr,"pmem ptr = %p\n", pmem);

        return 0;
    }

private:
    struct dynlib_block_t *block;

    int elements_to_write;
    bool useMutex;

    RingBuffer_shobj  *pmem;
};



// NOTE: MUSTER für C++ compfn
int write_RingBuffer_block(int flag, struct dynlib_block_t *block)
{
//     printf("RingBuffer_ write block: flag==%d\n", flag);

    double *in;
    double *rpar = libdyn_get_rpar_ptr(block);
    int *ipar = libdyn_get_ipar_ptr(block);

    int Nin = 1;
    int Nout = 0;

    in = (double *) libdyn_get_input_ptr(block,0);
    RingBuffer_write_block_class *worker = (RingBuffer_write_block_class *) libdyn_get_work_ptr(block);

    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        worker->io_output();
    }
    return 0;
    break;
    case COMPF_FLAG_UPDATESTATES:
    {
        worker->io_update();
    }
    return 0;
    break;
    case COMPF_FLAG_CONFIGURE:  // configure. NOTE: do not reserve memory or open devices. Do this while init instead!
    {
        int datatype = ipar[1];
        int size = ipar[2]; // len of datainput // ONLY SIZE OF 1 is ALLOWED FOR NOW

        if (size < 1) {
            fprintf(stderr, "ERROR: RingBuffer: WriteBlock. Size of data input shall be >= 1\n");
            return -1;
        }

        libdyn_config_block(block, BLOCKTYPE_DYNAMIC, Nout, Nin, (void *) 0, 0);
        libdyn_config_block_input(block, 0, size, datatype); // data input
    }
    return 0;
    break;
    case COMPF_FLAG_INIT:
    {
        worker = new RingBuffer_write_block_class(block);
        libdyn_set_work_ptr(block, (void*) worker);

        int ret = worker->init();
        if (ret < 0)
            return -1;
    }
    return 0;
    break;
    case COMPF_FLAG_RESETSTATES:
    {
        worker->reset();
    }
    return 0;
    break;
//     case COMPF_FLAG_HIGHERLEVELRESET:
//     {
//         worker->io(1);
//     }
//     return 0;
//     break;
    case COMPF_FLAG_PREINITUNDO: // destroy instance
    {
    }
    return 0;
    break;
    case COMPF_FLAG_DESTUCTOR: // destroy instance
    {
        worker->destruct();
        delete worker;
    }
    return 0;
    break;
    case COMPF_FLAG_PRINTINFO:
        printf("I'm a RingBuffer writing block\n");
        return 0;
        break;

    }
}

















class RingBuffer_read_block_class {
public:
    RingBuffer_read_block_class(struct dynlib_block_t *block)    {
        this->block = block;
    }
    void destruct() {        }

    void io_output() {
        void *outputPtr = (void *) libdyn_get_output_ptr(block,0);
        double *DataRead = (double *) libdyn_get_output_ptr(block,1);

        // Copy data
        bool GotElement;
        int i=0;
        do {
            void *outputElementPtr = ((uint8_t*) outputPtr) + pmem->data_element_size * i;
            GotElement = pmem->GetElementCopy( outputElementPtr );
            if (GotElement) 
	      i++;
        } while ( GotElement && i < ElementsToRead );

        // Return the number of elements read
        *DataRead = i;
    };
    void io_update() {       };
    void reset() {};
    int init() {
        double *rpar = libdyn_get_rpar_ptr(block);
        int *ipar = libdyn_get_ipar_ptr(block);

        int datatype = ipar[1];
        this->ElementsToRead = ipar[2];

        int len_ident_str = ipar[10];
        char * directory_entry_fname;
        irpar_getstr(&directory_entry_fname, ipar, 11, len_ident_str);

        fprintf(stderr, "read_RingBuffer_block: datatype %d size to read %d indentStr %s\n", datatype, this->ElementsToRead, directory_entry_fname);

        libdyn_master *master = (libdyn_master *) block->sim->master;

        // FIXME: need to make sure that this is really a Rigbuf object and not something else
        pmem = (RingBuffer_shobj*) get_ortd_global_shared_object(directory_entry_fname, master);


        if (pmem == NULL) {
            fprintf(stderr, "ERROR: read_RingBuffer_block: memory <%s> could not be found\n", directory_entry_fname);
            free(directory_entry_fname);
            return -1;
        }
        free(directory_entry_fname);


        // check consistency
        if (pmem->datatype != datatype) {
            fprintf(stderr, "ERROR: read_RingBuffer_block: missmatching datatype for memory <%s>\n", directory_entry_fname);
            return -1;
        }

        if (pmem->NumElements < this->ElementsToRead) {
            fprintf(stderr, "ERROR: read_RingBuffer_block: elements_to_write is bigger than the number of elements in memory <%s>\n", directory_entry_fname);
            return -1;
        }


        return 0;
    }

private:
    struct dynlib_block_t *block;

    int ElementsToRead;
    bool useMutex;

    RingBuffer_shobj  *pmem;
};



int read_RingBuffer_block(int flag, struct dynlib_block_t *block)
{
//     printf("RingBuffer_ read block: flag==%d\n", flag);

    double *in;
    double *rpar = libdyn_get_rpar_ptr(block);
    int *ipar = libdyn_get_ipar_ptr(block);

    int Nin = 0;
    int Nout = 2; // should be 1

    in = (double *) libdyn_get_input_ptr(block,0);
    RingBuffer_read_block_class *worker = (RingBuffer_read_block_class *) libdyn_get_work_ptr(block);

    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        worker->io_output();
    }
    return 0;
    break;
    case COMPF_FLAG_UPDATESTATES:
    {
        worker->io_update();
    }
    return 0;
    break;
    case COMPF_FLAG_CONFIGURE:  // configure. NOTE: do not reserve memory or open devices. Do this while init instead!
    {
        int datatype = ipar[1];
        int size = ipar[2]; // len of datainput

        libdyn_config_block(block, BLOCKTYPE_DYNAMIC, Nout, Nin, (void *) 0, 0);

        libdyn_config_block_output(block, 0, size, DATATYPE_FLOAT, 1);
        libdyn_config_block_output(block, 1, 1, DATATYPE_FLOAT, 1);

    }
    return 0;
    break;
    case COMPF_FLAG_INIT:
    {
        worker = new RingBuffer_read_block_class(block);
        libdyn_set_work_ptr(block, (void*) worker);

        int ret = worker->init();
        if (ret < 0)
            return -1;
    }
    return 0;
    break;
    case COMPF_FLAG_RESETSTATES:
    {
        worker->reset();
    }
    return 0;
    break;
//     case COMPF_FLAG_HIGHERLEVELRESET:
//     {
//         worker->io(1);
//     }
//     return 0;
//     break;
    case COMPF_FLAG_PREINITUNDO: // destroy instance
    {
    }
    return 0;
    break;
    case COMPF_FLAG_DESTUCTOR: // destroy instance
    {
        worker->destruct();
        delete worker;
    }
    return 0;
    break;
    case COMPF_FLAG_PRINTINFO:
        printf("I'm a RingBuffer reading block\n");
        return 0;
        break;

    }
}




