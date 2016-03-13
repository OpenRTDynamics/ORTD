/*
    Copyright (C) 2012, 2013, 2014, 2015, 2016  Christian Klauer

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

#include <pthread.h>
#include <string>
#include <malloc.h>

extern "C" {
#include "irpar.h"
#include "libdyn.h"
}
#include "libdyn_cpp.h"

struct dynlib_simulation_t;
class ortd_global_shared_object;

// get an instance of an global shared object 
void * get_ortd_global_shared_object(char *identName, libdyn_master *master);
// template <class T> int ortd_GetSharedObj(dynlib_block_t *block, T **IShObj_ret);


class ortd_global_shared_object {
/*  
 * A generic class for shared objects in ORTD
 * that are refered to by a directory entry
 * 
 * 
 * 
 *    
  */
  public:
    ortd_global_shared_object(const char *identName, libdyn_master *master) {
      constructor(identName, master);
    }
    ortd_global_shared_object(const char *identName, dynlib_simulation_t *sim, int Visibility) {
      if (Visibility != 0) {
	fprintf(stderr, "WARINIG: class ortd_global_shared_object: Visibility != 0 not supported by now. Assuming Visibility=0\n");
      }
      if (sim->master == NULL) {
	fprintf(stderr, "WARINIG: class ortd_global_shared_object: No libdyn master found! No sharing of the newly created object will be possible\n");
      } else {
	
        constructor(identName, (libdyn_master *) sim->master);
// 	fprintf(stderr, "ok.\n");
      }
      
    }
    
    void constructor(const char *identName, libdyn_master *master);
    
    void register_usage() {
      pthread_mutex_lock(&counter_mutex);		
      ++usage_counter;
      pthread_mutex_unlock(&counter_mutex);
    }
    void unregister_usage() {
      pthread_mutex_lock(&counter_mutex);		
      --usage_counter;
      pthread_mutex_unlock(&counter_mutex);
    }
    bool isUsed() {
      bool isUs = true;
      pthread_mutex_lock(&counter_mutex);		
      if (usage_counter == 0) {
	isUs = false;
      }
      pthread_mutex_unlock(&counter_mutex);
      return isUs;
    }
    bool isUnused() { return !isUsed(); }
    
    ~ortd_global_shared_object();
    

    
    
  private:
    const char *identName;
    std::string identName_;
    
    libdyn_master* ldmaster; // extracted from simnest
      
      void *buffer;
      
      int usage_counter;
      pthread_mutex_t counter_mutex;
};



//
// This is a template which creates a block that initialises  
// a shared object class ShObj
// 
template <class ShObj> class SharedObjBlock {
public:
    SharedObjBlock(struct dynlib_block_t *block) {
        this->block = block;    // no nothing more here. The real initialisation take place in init()
        ObjectIdentifyer = NULL;
	fprintf(stderr, "New shared obj block, using template SharedObjBlock\n");
    }
    ~SharedObjBlock()
    {
        // free your allocated memory, ...
      	// clean up
      	if (ObjectIdentifyer!=NULL) {
	  free(ObjectIdentifyer); // do not forget to free the memory allocated by irpar_getstr
	}
	
	delete ITemplate_SharedObject;
    }

private:
  ShObj *ITemplate_SharedObject;
  char *ObjectIdentifyer;

public:
    //
    // initialise your block
    //

    int init() {
      
        int *Uipar;
        double *Urpar;

//         // Get the irpar parameters Uipar, Urpar
//         libdyn_AutoConfigureBlock_GetUirpar(block, &Uipar, &Urpar);

	
        //
        // extract some structured sample parameters
        //

	double *rpar = libdyn_get_rpar_ptr(block);
        int *ipar = libdyn_get_ipar_ptr(block);	
	
        int error = 0;

       fprintf(stderr, "init of new shared obj block: ");	
	
        //
        // get the ObjectIdentifyer
        //
        struct irpar_ivec_t str_;
        if ( irpar_get_ivec(&str_, ipar, rpar, 30) < 0 ) { 
	  fprintf(stderr, "Could not fetch ObjectIdentifyer! Did you create this block using libdyn_CreateSharedObjBlk() in Scilab?\n");
	  return -1;		  
	}
        irpar_getstr(&ObjectIdentifyer, str_.v, 0, str_.n);

        fprintf(stderr, "ObjectIdentifyer = %s ", ObjectIdentifyer);

        //
        // get a vector of integers (double vectors are similar, replace ivec with rvec)
        //
        struct irpar_ivec_t vec;
        if ( irpar_get_ivec(&vec, ipar, rpar, 31) < 0 ) { 
	    fprintf(stderr, "Could not fetch Visibility! Did you create this block using libdyn_CreateSharedObjBlk() in Scilab?\n");
	  return -1;		  
	}
	int Visibility = vec.v[0];
	
        fprintf(stderr, "Visibility  = %d\n", Visibility);

	//
	// Create the shared object
	//
	
	try {
	  ITemplate_SharedObject = new ShObj(ObjectIdentifyer, block, Visibility);
	} catch (int e) {
	  fprintf(stderr, "Error\n"); 
	  return -1;
	}
	
	
	
	if ( ITemplate_SharedObject->init() < 0 ) {
 	  fprintf(stderr, "Error\n");
	  delete ITemplate_SharedObject;
	  return -1;
	}
	
// 	fprintf(stderr, "OK\n");
        
        return 0;
    }


    inline void updateStates()   {    }
    inline void calcOutputs()    {    }
    inline void resetStates()    {    }


    void printInfo() {
        fprintf(stderr, "I'm a Template SharedObject-Main block\n");
    }

    // uncommonly used flags
    void PrepareReset() {}
    void HigherLevelResetStates() {}
    void PostInit() {}


    // The Computational function that is called by the simulator
    // and that distributes the execution to the various functions
    // in this C++ - Class, including: init(), io(), resetStates() and the destructor
    static int CompFn(int flag, struct dynlib_block_t *block) {
        return LibdynCompFnTempate_PreInit<SharedObjBlock>( flag, block ); // this expands a template for a C-comp fn

	// I cannot under stand why the line below doesn't have to look like:
// 	return LibdynCompFnTempate_PreInit<SharedObjBlock<ShObj>>( flag, block ); 
	  
    }

    // The data for this block managed by the simulator
    struct dynlib_block_t *block;
};



// A nice function used in blocks that access shared objects
template <class T> int ortd_GetSharedObj(dynlib_block_t *block, T **IShObj_ret)
{
 
  *IShObj_ret = NULL;
  
        //
        // extract some structured sample parameters
        //

	double *rpar = libdyn_get_rpar_ptr(block);
        int *ipar = libdyn_get_ipar_ptr(block);	
	
        int error = 0;

	char *ObjectIdentifyer;
	
        //
        // get the ObjectIdentifyer
        //
        struct irpar_ivec_t str_;
        if ( irpar_get_ivec(&str_, ipar, rpar, 30) < 0 ) { 
	  fprintf(stderr, "Could not fetch ObjectIdentifyer! Did you create this block using libdyn_CreateSharedObjBlk() in Scilab?\n");
	  return -1;		  
	}
        irpar_getstr(&ObjectIdentifyer, str_.v, 0, str_.n);

        printf("ObjectIdentifyer =    %s\n", ObjectIdentifyer);

//         //
//         // get a vector of integers (double vectors are similar, replace ivec with rvec)
//         //
//         struct irpar_ivec_t vec;
//         if ( irpar_get_ivec(&vec, ipar, rpar, 31) < 0 ) { 
// 	    fprintf(stderr, "Could not fetch Visibility! Did you create this block using libdyn_CreateSharedObjBlk() in Scilab?\n");
// 	  return -1;		  
// 	}
// 	int Visibility = vec.v[0];
// 	
//         printf("Visibility  = %d\n", Visibility);

	

        libdyn_master *master = (libdyn_master *) block->sim->master;

        // FIXME: need to make sure that this is really a shared object of a requested type and not something else
	T *IShObj;
	try {
          IShObj = (T *) get_ortd_global_shared_object(ObjectIdentifyer, master);
	} catch(int e) { 
	  free(ObjectIdentifyer);
	  return -1;
	}

        if (IShObj == NULL) {
            fprintf(stderr, "ERROR: ortd_GetSharedObj: Object <%s> could not be found\n", ObjectIdentifyer);
            free(ObjectIdentifyer);
	    	    
            return -1;
        }
        free(ObjectIdentifyer);


	*IShObj_ret = IShObj;
	return 0;
}


