/*
    Copyright (C) 2012, 2013  Christian Klauer

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

/*
  zwei Typen von Blöcken:

    * Ersteller Block: Hier wird größe und datentyp festgelegt. Er erstellt die Instanz der Klasse persistent_memory,
      sobald PREINIT aufgerufen wird (bevor andere Blöcke initialisiert werden). EIn directory Eintrag wird angelegt
    * Blöcke, die hierauf über das directory zugreifen
    * Falls der usage counter von persistent_memory wieder null ist wird die Instanz von persistent_memory wieder zerstört


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
    extern int persistent_memory_block(int flag, struct dynlib_block_t *block);
    extern int write_persistent_memory_block(int flag, struct dynlib_block_t *block);
    extern int read_persistent_memory_block(int flag, struct dynlib_block_t *block);
    extern int write2_persistent_memory_block(int flag, struct dynlib_block_t *block);
};






// Shared object that manages the memory

class persistent_memory_shobj : public ortd_global_shared_object {
public:
    persistent_memory_shobj(const char* identName, libdyn_master* master, int datatype, int len, void *initial_data, bool useMutex) : ortd_global_shared_object(identName, master) {
        this->datatype = datatype;
        this->data_num_elements = len;
        this->useMutex = useMutex;


        this->data_element_size = libdyn_config_get_datatype_len(datatype);
        this->memsize = this->data_element_size*this->data_num_elements;

        // printf("init persistent_memory. Memsize = %d\n", memsize);

        // allocate data
        data = malloc(memsize);

        if ((initial_data != NULL) && (datatype == DATATYPE_FLOAT))
            memcpy(data, initial_data, memsize);
        else
            memset(data, 0, memsize);

        // initialise mutex, if desired
        if (useMutex) {
            pthread_mutex_init(&data_mutex, NULL);
        }

    }

    void* get_dataptr() {
        return data;
    }
    void lock_data() {
        pthread_mutex_lock(&data_mutex);
    }
    void unlock_data() {
        pthread_mutex_unlock(&data_mutex);
    }
    void set_wholedata(void *src) {
        lock_data();
        memcpy(data, src, memsize);
        unlock_data();
    }
    bool getuseMutex() {
        return useMutex;
    }


    ~persistent_memory_shobj() {
        free(data);

        if (useMutex) {
            pthread_mutex_destroy(&data_mutex);
        }

    }
public:
    uint data_element_size;
    uint data_num_elements;
    int datatype;

private:
    void *data;
    bool useMutex;

    uint memsize;

    pthread_mutex_t data_mutex;
};






class persistent_memory_block_class {
public:
    persistent_memory_block_class(struct dynlib_block_t *block)    {
        this->block = block;
	
	this->pmem = NULL;
	this->directory_entry_fname = NULL;
    }
    void destruct() {
      
      if (directory_entry_fname != NULL)
        free(directory_entry_fname);
      
      if (pmem != NULL) {
	
        if (!pmem->isUsed()) {
            delete pmem;
        } else {
#ifdef DEBUG
            fprintf(stderr, "ASSERTION FAILED: persistent_memory_block_class: object is still in use and will therefore remain in memory\n");
#endif
        }
      }
    }

    void io(int update_states) {};
    void reset() {};
    int init() {
        double *rpar = libdyn_get_rpar_ptr(block);
        int *ipar = libdyn_get_ipar_ptr(block);
	
	this->pmem = NULL;
	this->directory_entry_fname = NULL;

        this->datatype = ipar[1];
        this->size = ipar[2];
        this->useMutex = ipar[3];
	
	double *initial_data_vec = &rpar[0];

        int len_ident_str = ipar[10];
        irpar_getstr(&this->directory_entry_fname, ipar, 11, len_ident_str);
	
#ifdef DEBUG
        fprintf(stderr, "persistent_memory_block_class: datatype %d size %d indentStr %s\n", this->datatype, this->size, this->directory_entry_fname);
#endif

        libdyn_master *master = (libdyn_master *) block->sim->master;
	
	// FIXME: catch exception herer
        pmem = new persistent_memory_shobj( directory_entry_fname, master, this->datatype, this->size, initial_data_vec, this->useMutex );
	
	// if error return -1;!!!!

	
// 	fprintf(stderr,"pmem ptr = %p\n", pmem);

        return 0;
    }

private:
    struct dynlib_block_t *block;

    char * directory_entry_fname;

    int datatype;
    int size;
    int useMutex;

    persistent_memory_shobj  *pmem;
};

int persistent_memory_block(int flag, struct dynlib_block_t *block)
{

//     printf("persistent_memory_block: flag==%d\n", flag);

    double *in;
    double *rpar = libdyn_get_rpar_ptr(block);
    int *ipar = libdyn_get_ipar_ptr(block);

    int Nin = 0;
    int Nout = 0;


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
// 	fprintf(stderr, "++++++++++++++++++++++++++ persistent_memory_block %p: output: \n", block);
//        in = (double *) libdyn_get_input_ptr(block,0);
	
	
        persistent_memory_block_class *worker = (persistent_memory_block_class *) libdyn_get_work_ptr(block);

        worker->io(0);
    }
    return 0;
    break;
    case COMPF_FLAG_UPDATESTATES:
    {
     //   in = (double *) libdyn_get_input_ptr(block,0);
        persistent_memory_block_class *worker = (persistent_memory_block_class *) libdyn_get_work_ptr(block);

        worker->io(1);

    }
    return 0;
    break;
    case COMPF_FLAG_CONFIGURE:  // configure. NOTE: do not reserve memory or open devices. Do this while init instead!
    {
        int datatype = ipar[1];
        int size = ipar[2];

        libdyn_config_block(block, BLOCKTYPE_STATIC, Nout, Nin, (void *) 0, 0);

//         libdyn_config_block_input(block, 0, 1, DATATYPE_FLOAT);
//         libdyn_config_block_input(block, 1, 1, DATATYPE_FLOAT);
//         libdyn_config_block_output(block, 0, 1, DATATYPE_FLOAT, 1);
    }
    return 0;
    break;
    case COMPF_FLAG_PREINIT:  // pre- init
    {
        persistent_memory_block_class *worker = new persistent_memory_block_class(block);
        libdyn_set_work_ptr(block, (void*) worker);

// 	fprintf(stderr, "++++++++++++++++++++++++++ persistent_memory_block %p: preinit: \n", block);

        int ret = worker->init();
        if (ret < 0)
            return -1;
    }
    return 0;
    break;
    case COMPF_FLAG_RESETSTATES:
    {
//        in = (double *) libdyn_get_input_ptr(block,0);  IMPORTANT NOTE: NOT ALLOWED IN BLOCKS WITHOUT ANY I/O
        persistent_memory_block_class *worker = (persistent_memory_block_class *) libdyn_get_work_ptr(block);

        worker->reset();
    }
    return 0;
    break;
//     case COMPF_FLAG_HIGHERLEVELRESET:
//     {
//         in = (double *) libdyn_get_input_ptr(block,0);
//         persistent_memory_block_class *worker = (persistent_memory_block_class *) libdyn_get_work_ptr(block);
//
//         worker->io(1);
//     }
//     return 0;
//     break;
    case COMPF_FLAG_PREINITUNDO: // destroy instance
    {
        persistent_memory_block_class *worker = (persistent_memory_block_class *) libdyn_get_work_ptr(block);

        worker->destruct();
        delete worker;

    }
    return 0;
    break;
    case COMPF_FLAG_DESTUCTOR: // destroy instance
    {
        persistent_memory_block_class *worker = (persistent_memory_block_class *) libdyn_get_work_ptr(block);

        worker->destruct();
        delete worker;
    }
    return 0;
    break;
    case COMPF_FLAG_PRINTINFO:
        printf("I'm a persistent_memory initialising block\n");
        return 0;
        break;

    }
}















// NOTE: MUSTER für C++ compfn
class persistent_memory_write_block_class {
public:
    persistent_memory_write_block_class(struct dynlib_block_t *block)    {
        this->block = block;
    }
    void destruct() {        }

    void io_output() {};
    void io_update() {
        void *dataToWrite = (void *) libdyn_get_input_ptr(block,0);
        double *index__ = (double *) libdyn_get_input_ptr(block,1);
        int index = round(*index__); // is starting at 1

        int maxindex = pmem->data_num_elements - elements_to_write + 1;
        if (index < 1) index=1;
        if (index > maxindex) index=maxindex;

//         printf("write %d elements to index %d\n", elements_to_write, index);

        index--; // convert to C-index


        // calc destination ptr
        void *destptr = (void*) ( ((char*) pmem->get_dataptr()) + pmem->data_element_size*index );

        if (useMutex) {
            pmem->lock_data();
            memcpy(destptr, dataToWrite, elements_to_write*pmem->data_element_size);
            pmem->unlock_data();
        } else {
            memcpy(destptr, dataToWrite, elements_to_write*pmem->data_element_size);
        }

    };
    void reset() {};
    int init() {
        double *rpar = libdyn_get_rpar_ptr(block);
        int *ipar = libdyn_get_ipar_ptr(block);

        int datatype = ipar[1];
        this->elements_to_write = ipar[2];

        int len_ident_str = ipar[10];
        char * directory_entry_fname;
        irpar_getstr(&directory_entry_fname, ipar, 11, len_ident_str);

#ifdef DEBUG
        fprintf(stderr, "write_persistent_memory_block: datatype %d size to write %d indentStr %s\n", datatype, this->elements_to_write, directory_entry_fname);
#endif

        libdyn_master *master = (libdyn_master *) block->sim->master;
//         pmem = new persistent_memory_shobj( directory_entry_fname, master, this->datatype, this->size, NULL, this->useMutex );

        // FIXME: need to make sure that this is really a memory object and not something else
        pmem = (persistent_memory_shobj*) get_ortd_global_shared_object(directory_entry_fname, master);        

        if (pmem == NULL) {
            fprintf(stderr, "ERROR: write_persistent_memory_block: memory <%s> could not be found\n", directory_entry_fname);
	    free(directory_entry_fname);
            return -1;
        }
        free(directory_entry_fname);
	
        useMutex = pmem->getuseMutex();


        // check consistency
        if (pmem->datatype != datatype) {
            fprintf(stderr, "ERROR: write_persistent_memory_block: missmatching datatype for memory <%s>\n", directory_entry_fname);
            return -1;
        }

        if (pmem->data_num_elements < this->elements_to_write) {
            fprintf(stderr, "ERROR: write_persistent_memory_block: elements_to_write is bigger than the number of elements in memory <%s>\n", directory_entry_fname);
            return -1;
        }


// 	fprintf(stderr,"pmem ptr = %p\n", pmem);

        return 0;
    }

private:
    struct dynlib_block_t *block;




    int elements_to_write;
    bool useMutex;

    persistent_memory_shobj  *pmem;
};



// NOTE: MUSTER für C++ compfn
int write_persistent_memory_block(int flag, struct dynlib_block_t *block)
{
//     printf("persistent_memory_ write block: flag==%d\n", flag);

    double *in;
    double *rpar = libdyn_get_rpar_ptr(block);
    int *ipar = libdyn_get_ipar_ptr(block);

    int Nin = 2;
    int Nout = 0;

    persistent_memory_write_block_class *worker = (persistent_memory_write_block_class *) libdyn_get_work_ptr(block);

    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        worker->io_output();
    }
    return 0;
    break;
    case COMPF_FLAG_UPDATESTATES:
    {
    in = (double *) libdyn_get_input_ptr(block,0);
        worker->io_update();
    }
    return 0;
    break;
    case COMPF_FLAG_CONFIGURE:  // configure. NOTE: do not reserve memory or open devices. Do this while init instead!
    {
        int datatype = ipar[1];
        int size = ipar[2]; // len of datainput

        libdyn_config_block(block, BLOCKTYPE_STATIC, Nout, Nin, (void *) 0, 0);

        libdyn_config_block_input(block, 0, size, datatype); // data input
        libdyn_config_block_input(block, 1, 1, DATATYPE_FLOAT); // control input
//         libdyn_config_block_output(block, 0, 1, DATATYPE_FLOAT, 1);

    }
    return 0;
    break;
    case COMPF_FLAG_INIT:
    {
        worker = new persistent_memory_write_block_class(block);
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
        printf("I'm a persistent_memory writing block\n");
        return 0;
        break;

    }
}





















class persistent_memory_read_block_class {
public:
    persistent_memory_read_block_class(struct dynlib_block_t *block)    {
        this->block = block;
    }
    void destruct() {        }

    void io_output() {
        void *outputPtr = (void *) libdyn_get_output_ptr(block,0);
        double *index__ = (double *) libdyn_get_input_ptr(block,0);
        int index = round(*index__); // is starting at 1

        int maxindex = pmem->data_num_elements - elements_to_write + 1;
        if (index < 1) index=1;
        if (index > maxindex) index=maxindex;

//         printf("read %d elements to index %d\n", elements_to_write, index);

        index--; // convert to C-index


        // calc destination ptr
        void *srcptr = (void*) ( ((char*) pmem->get_dataptr()) + pmem->data_element_size*index );

        if (useMutex) {
            pmem->lock_data();
            memcpy(outputPtr, srcptr, elements_to_write*pmem->data_element_size);
            pmem->unlock_data();
        } else {
            memcpy(outputPtr, srcptr, elements_to_write*pmem->data_element_size);
        }

    };
    void io_update() {       };
    void reset() {};
    int init() {
        double *rpar = libdyn_get_rpar_ptr(block);
        int *ipar = libdyn_get_ipar_ptr(block);

        int datatype = ipar[1];
        this->elements_to_write = ipar[2];

        int len_ident_str = ipar[10];
        char * directory_entry_fname;
        irpar_getstr(&directory_entry_fname, ipar, 11, len_ident_str);

        fprintf(stderr, "write_persistent_memory_block: datatype %d size to write %d indentStr %s\n", datatype, this->elements_to_write, directory_entry_fname);

        libdyn_master *master = (libdyn_master *) block->sim->master;
//         pmem = new persistent_memory_shobj( directory_entry_fname, master, this->datatype, this->size, NULL, this->useMutex );

        // FIXME: need to make sure that this is really a memory object and not something else
        pmem = (persistent_memory_shobj*) get_ortd_global_shared_object(directory_entry_fname, master);
        

        if (pmem == NULL) {
            fprintf(stderr, "ERROR: read_persistent_memory_block: memory <%s> could not be found\n", directory_entry_fname);
	    free(directory_entry_fname);
            return -1;
        }
        
        free(directory_entry_fname);
        
        useMutex = pmem->getuseMutex();




        // check consistency
        if (pmem->datatype != datatype) {
            fprintf(stderr, "ERROR: read_persistent_memory_block: missmatching datatype for memory <%s>\n", directory_entry_fname);
            return -1;
        }

        if (pmem->data_num_elements < this->elements_to_write) {
            fprintf(stderr, "ERROR: read_persistent_memory_block: elements_to_write is bigger than the number of elements in memory <%s>\n", directory_entry_fname);
            return -1;
        }


// 	fprintf(stderr,"pmem ptr = %p\n", pmem);

        return 0;
    }

private:
    struct dynlib_block_t *block;




    int elements_to_write;
    bool useMutex;

    persistent_memory_shobj  *pmem;
};



int read_persistent_memory_block(int flag, struct dynlib_block_t *block)
{
//     printf("persistent_memory_ read block: flag==%d\n", flag);

    double *in;
    double *rpar = libdyn_get_rpar_ptr(block);
    int *ipar = libdyn_get_ipar_ptr(block);

    int Nin = 1;
    int Nout = 1; // should be 1

    persistent_memory_read_block_class *worker = (persistent_memory_read_block_class *) libdyn_get_work_ptr(block);

    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        in = (double *) libdyn_get_input_ptr(block,0);
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

        libdyn_config_block(block, BLOCKTYPE_STATIC, Nout, Nin, (void *) 0, 0);

        libdyn_config_block_input(block, 0, 1, DATATYPE_FLOAT); // control input
        libdyn_config_block_output(block, 0, size, DATATYPE_FLOAT, 1);

    }
    return 0;
    break;
    case COMPF_FLAG_INIT:
    {
        worker = new persistent_memory_read_block_class(block);
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
        printf("I'm a persistent_memory reading block\n");
        return 0;
        break;

    }
}


















class persistent_memory_write2_block_class {
public:
    persistent_memory_write2_block_class(struct dynlib_block_t *block)    {
        this->block = block;
    }
    void destruct() {        }

    void io_output() {};
    void io_update() {
        void *dataToWrite = (void *) libdyn_get_input_ptr(block,0);
        int32_t *index__ = (int32_t *) libdyn_get_input_ptr(block,1);
	int index = *index__;
        int32_t *Nwrite__ = (int32_t *) libdyn_get_input_ptr(block,2);
	int Nwrite = *Nwrite__;
        

        int maxindex = pmem->data_num_elements - Nwrite + 1;
        if (index < 1 || index > maxindex || Nwrite > MaxElements) {
	  fprintf(stderr, "write_persistent_memory_block2: index / Nwrite out of range\n");
	  
	  return;
	}
	  

//         printf("write %d elements to index %d\n", Nwrite, index);

        index--; // convert to C-index


        // calc destination ptr
        void *destptr = (void*) ( ((char*) pmem->get_dataptr()) + pmem->data_element_size*index );

        if (useMutex) {
            pmem->lock_data();
            memcpy(destptr, dataToWrite, Nwrite*pmem->data_element_size);
            pmem->unlock_data();
        } else {
            memcpy(destptr, dataToWrite, Nwrite*pmem->data_element_size);
        }

    };
    void reset() {};
    int init() {
        double *rpar = libdyn_get_rpar_ptr(block);
        int *ipar = libdyn_get_ipar_ptr(block);

        int datatype = ipar[1];
	MaxElements = ipar[2];

        int len_ident_str = ipar[10];
        char * directory_entry_fname;
        irpar_getstr(&directory_entry_fname, ipar, 11, len_ident_str);

#ifdef DEBUG
        fprintf(stderr, "write_persistent_memory_block2: datatype %d indentStr %s\n", datatype, directory_entry_fname);
#endif

        libdyn_master *master = (libdyn_master *) block->sim->master;
//         pmem = new persistent_memory_shobj( directory_entry_fname, master, this->datatype, this->size, NULL, this->useMutex );

        // FIXME: need to make sure that this is really a memory object and not something else
        pmem = (persistent_memory_shobj*) get_ortd_global_shared_object(directory_entry_fname, master);        

        if (pmem == NULL) {
            fprintf(stderr, "ERROR: write_persistent_memory_block2: memory <%s> could not be found\n", directory_entry_fname);
	    free(directory_entry_fname);
            return -1;
        }
        free(directory_entry_fname);
	
        useMutex = pmem->getuseMutex();


        // check consistency
        if (pmem->datatype != datatype) {
            fprintf(stderr, "ERROR: write_persistent_memory_block2: missmatching datatype for memory <%s>\n", directory_entry_fname);
            return -1;
        }



// 	fprintf(stderr,"pmem ptr = %p\n", pmem);

        return 0;
    }

private:
    struct dynlib_block_t *block;




   
    int MaxElements;
    bool useMutex;

    persistent_memory_shobj  *pmem;
};



// NOTE: MUSTER für C++ compfn
int write2_persistent_memory_block(int flag, struct dynlib_block_t *block)
{
//     printf("persistent_memory_ write block: flag==%d\n", flag);

    double *in;
    double *rpar = libdyn_get_rpar_ptr(block);
    int *ipar = libdyn_get_ipar_ptr(block);

    int Nin = 3;
    int Nout = 0;

    persistent_memory_write2_block_class *worker = (persistent_memory_write2_block_class *) libdyn_get_work_ptr(block);

    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
   // in = (double *) libdyn_get_input_ptr(block,0);
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

        libdyn_config_block(block, BLOCKTYPE_STATIC, Nout, Nin, (void *) 0, 0);

        libdyn_config_block_input(block, 0, size, datatype); // data input
        libdyn_config_block_input(block, 1, 1, DATATYPE_INT32); // control input ofs
        libdyn_config_block_input(block, 2, 1, DATATYPE_INT32); // control input Nwrite
	

    }
    return 0;
    break;
    case COMPF_FLAG_INIT:
    {
        worker = new persistent_memory_write2_block_class(block);
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
        printf("I'm a persistent_memory writing2 block\n");
        return 0;
        break;

    }
}

