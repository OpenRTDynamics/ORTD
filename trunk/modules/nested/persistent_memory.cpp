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

 #include "directory.h"


extern "C" int persistent_memory_block(int flag, struct dynlib_block_t *block);





class persistent_memory {
  public:
    persistent_memory(const char *identName, libdyn_nested *simnest );
    
    register_usage() {
      // FIXME MUTEX
      ++usage_counter;
    }
    unregister_usage() {
      // FIXME MUTEX
      --usage_counter;
    }
    
    ~persistent_memory();
    
    
  private:
    const char *identName;

    libdyn_master* ldmaster; // extracted from simnest
      
      void *buffer:
      
      int usage_counter;
};


persistent_memory::persistent_memory(const char* identName)
{
  this->identName = identName;
  this->usage_counter = 0;
  
  this->ldmaster = simnest->ld_master;
  if (this->ldmaster == NULL) {
    fprintf(stderr, "stderr: persistent_memory: needs a libdyn master\n");
    // FIXME: throuw exception
    
    return;
  }
  
  directory_tree *dtree = ldmaster->dtree;
  if (dtree == NULL) {
    fprintf(stderr, "stderr: persistent_memory: needs a root directory_tree\n");
    // FIXME: throuw exception
     
  }
  
  
  if (dtree->add_entry((char*) identName, ORTD_DIRECTORY_ENTRYTYPE_NESTEDONLINEEXCHANGE, this, this) == false) {
     fprintf(stderr, "stderr: persistent_memory: cound not allocate the filename %s\n", identName);
  }
  
  // alloc memory for the buffer
  
}





persistent_memory::~persistent_memory()
{
  directory_tree *dtree = ldmaster->dtree;
  dtree->delete_entry((char*) identName);
  
  if (this->current_irdata != NULL)
    delete this->current_irdata;
}


// #endif



// A stupid helper classe which accesses the directory
class persistent_memory_init {
public:
  persistent_memory_init() { }
  
  persistent_memory * persistent_memory_register(libdyn_master *master, char fname) {
        if (master == NULL || master->dtree == NULL) {  // no master available
           fprintf(stderr, "WARNING: persistent_memory: block requires a libdyn master\n");
	   
	   return NULL;
	}
	
	// get the identifier
	directory_entry::direntry *dentr = master->dtree->access(nested_simname, NULL);
	
	if (dentr == NULL) {
	  fprintf(stderr, "WARNING: persistent_memory: Can not find simulation %s\n", nested_simname);
	   return NULL;
	}

	if (dentr->type != ORTD_DIRECTORY_ENTRYTYPE_PERSISTENTMEMORY) {
	  fprintf(stderr, "WARNING: persistent_memory: wrong type for %s\n", nested_simname);
	   return NULL;
	}
	
	persistent_memory *pmem = (persistent_memory *) dentr->userptr;
 
  }
  
  
};










class persistent_memory_block_class {
public:
    persistent_memory_block_class(struct dynlib_block_t *block);
    void destruct();
    void io(int update_states);
    void reset();
    int init();
private:
   struct dynlib_block_t *block;
   
   char * directory_entry_fname;
   
   int datatype;
   int size;
   
   persistent_memory *pmem;
};

persistent_memory_block_class::persistent_memory_block_class(dynlib_block_t* block)
{
    this->block = block;
}

int persistent_memory_block_class::init()
{
    double *rpar = libdyn_get_rpar_ptr(block);
    int *ipar = libdyn_get_ipar_ptr(block);

    double Nin = ipar[0];
    double Nout = ipar[1];

//     this->ifname = "ident1_exch.ipar";
//     this->rfname = "ident1_exch.rpar";

  
    this->datatype = ipar[1];
    this->size = ipar[2];
    
    int len_s1 = ipar[3];

    int ofs = 4;
    
    int *s1 = &ipar[ofs+0];
    
    irpar_getstr(&this->directory_entry_fname, s1, 0, len_s1);
    
//     fprintf(stderr, "persistent_memory_block_class: %s %s %s\n", ifname, rfname, nested_simname);
    

    libdyn_master *master = (libdyn_master *) block->sim->master;
    persistent_memory_init meminit;
    pmem = meminit.persistent_memory_register(master, this->directory_entry_fname);
    
    
    return 0;
}


void persistent_memory_block_class::io(int update_states)
{
    if (update_states==0) {
 
    }
}

void persistent_memory_block_class::reset()
{

}


void persistent_memory_block_class::destruct()
{
    persistent_memory_init meminit;
    pmem = meminit.persistent_memory_register(master, this->directory_entry_fname);
    if (pmem != NULL) // another block counld have been deleted this one
      delete pmem;

  free(directory_entry_fname);
}


int persistent_memory_block(int flag, struct dynlib_block_t *block)
{

//     printf("comp_func nested_exchange_fromfile: flag==%d\n", flag);

    double *in;
    double *rpar = libdyn_get_rpar_ptr(block);
    int *ipar = libdyn_get_ipar_ptr(block);

    int Nin = 1;
    int Nout = 1;


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        in = (double *) libdyn_get_input_ptr(block,0);
        persistent_memory_block_class *worker = (persistent_memory_block_class *) libdyn_get_work_ptr(block);

        worker->io(0);
	
	

    }
    return 0;
    break;
    case COMPF_FLAG_UPDATESTATES:
    {
        in = (double *) libdyn_get_input_ptr(block,0);
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

        libdyn_config_block_input(block, 0, 1, DATATYPE_FLOAT);
        libdyn_config_block_input(block, 1, 1, DATATYPE_FLOAT);
        libdyn_config_block_output(block, 0, 1, DATATYPE_FLOAT, 1);


    }
    return 0;
    break;
    case COMPF_FLAG_INIT:  // init
    {
        persistent_memory_block_class *worker = new persistent_memory_block_class(block);
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
        persistent_memory_block_class *worker = (persistent_memory_block_class *) libdyn_get_work_ptr(block);

        worker->reset;
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
    case COMPF_FLAG_DESTUCTOR: // destroy instance
    {
        persistent_memory_block_class *worker = (persistent_memory_block_class *) libdyn_get_work_ptr(block);

        worker->destruct();
	delete worker;

    }
    return 0;
    break;
    case COMPF_FLAG_PRINTINFO:
        printf("I'm a nested_exchange_fromfile block\n");
        return 0;
        break;

    }
}


