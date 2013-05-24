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
#include <iostream>

extern "C" {
#include "libdyn.h"
#include "libdyn_scicos_macros.h"
#include <math.h>
//#include "irpar.h"

}
#include <libdyn_cpp.h>
#include "global_shared_object.h"

#include "directory.h"










// Abgeleitete Klasse

class Template_SharedObject : public ortd_global_shared_object {
  public:
    Template_SharedObject(const char* identName, dynlib_block_t *block, int Visibility) : ortd_global_shared_object(identName, block->sim, Visibility) {      
      fprintf(stderr, "---\n");
      
      data = 1;
      this->block = block;

      // Get the irpar parameters Uipar, Urpar      
      libdyn_AutoConfigureBlock_GetUirpar(block, &Uipar, &Urpar);
      
      
    }

    ~Template_SharedObject() {
      
    }
    
    int init() {

      return 0; // init was ok
    }


  int *Uipar;
  double *Urpar;  
  dynlib_block_t *block;
   
  
/*
    Some data of the shared object
*/
private:
  
  int data;
//     int udpport;
//     UDPSocket *socket;

    
};





template <T, ShObj> class T {
public:
    T(struct dynlib_block_t *block) {
        this->block = block;    // no nothing more here. The real initialisation take place in init()
        ObjectIdentifyer = NULL;
    }
    ~T()
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

        //
        // get a vector of integers (double vectors are similar, replace ivec with rvec)
        //
        struct irpar_ivec_t vec;
        if ( irpar_get_ivec(&vec, ipar, rpar, 31) < 0 ) { 
	    fprintf(stderr, "Could not fetch Visibility! Did you create this block using libdyn_CreateSharedObjBlk() in Scilab?\n");
	  return -1;		  
	}
	int Visibility = vec.v[0];
	
        printf("Visibility  = %d\n", Visibility);

	//
	// Create the shared object
	//
	
	ITemplate_SharedObject = new Template_SharedObject(ObjectIdentifyer, block, Visibility);
	
	fprintf(stderr, "Error\n");
	
	if ( ITemplate_SharedObject->init() < 0 ) {
 	  fprintf(stderr, "Error\n");
	  delete ITemplate_SharedObject;
	  return -1;
	}
	
	fprintf(stderr, "OK\n");
        
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
        return LibdynCompFnTempate_PreInit<T>( flag, block ); // this expands a template for a C-comp fn
    }

    // The data for this block managed by the simulator
    struct dynlib_block_t *block;
};











// class Template_CreateSharedObjBlock {
// public:
//     Template_CreateSharedObjBlock(struct dynlib_block_t *block) {
//         this->block = block;    // no nothing more here. The real initialisation take place in init()
//         ObjectIdentifyer = NULL;
//     }
//     ~Template_CreateSharedObjBlock()
//     {
//         // free your allocated memory, ...
//       	// clean up
//       	if (ObjectIdentifyer!=NULL) {
// 	  free(ObjectIdentifyer); // do not forget to free the memory allocated by irpar_getstr
// 	}
// 	
// 	delete ITemplate_SharedObject;
//     }
// 
// private:
//   Template_SharedObject *ITemplate_SharedObject;
//   char *ObjectIdentifyer;
// 
// public:
//     //
//     // initialise your block
//     //
// 
//     int init() {
//         int *Uipar;
//         double *Urpar;
// 
// //         // Get the irpar parameters Uipar, Urpar
// //         libdyn_AutoConfigureBlock_GetUirpar(block, &Uipar, &Urpar);
// 
// 	
//         //
//         // extract some structured sample parameters
//         //
// 
// 	double *rpar = libdyn_get_rpar_ptr(block);
//         int *ipar = libdyn_get_ipar_ptr(block);	
// 	
//         int error = 0;
// 
//         //
//         // get the ObjectIdentifyer
//         //
//         struct irpar_ivec_t str_;
//         if ( irpar_get_ivec(&str_, ipar, rpar, 30) < 0 ) { 
// 	  fprintf(stderr, "Could not fetch ObjectIdentifyer! Did you create this block using libdyn_CreateSharedObjBlk() in Scilab?\n");
// 	  return -1;		  
// 	}
//         irpar_getstr(&ObjectIdentifyer, str_.v, 0, str_.n);
// 
//         printf("ObjectIdentifyer =    %s\n", ObjectIdentifyer);
// 
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
// 
// 	//
// 	// Create the shared object
// 	//
// 	
// 	ITemplate_SharedObject = new Template_SharedObject(ObjectIdentifyer, block, Visibility);
// 	
// 	fprintf(stderr, "Error\n");
// 	
// 	if ( ITemplate_SharedObject->init() < 0 ) {
//  	  fprintf(stderr, "Error\n");
// 	  delete ITemplate_SharedObject;
// 	  return -1;
// 	}
// 	
// 	fprintf(stderr, "OK\n");
//         
//         return 0;
//     }
// 
// 
//     inline void updateStates()   {    }
//     inline void calcOutputs()    {    }
//     inline void resetStates()    {    }
// 
// 
//     void printInfo() {
//         fprintf(stderr, "I'm a Template SharedObject-Main block\n");
//     }
// 
//     // uncommonly used flags
//     void PrepareReset() {}
//     void HigherLevelResetStates() {}
//     void PostInit() {}
// 
// 
//     // The Computational function that is called by the simulator
//     // and that distributes the execution to the various functions
//     // in this C++ - Class, including: init(), io(), resetStates() and the destructor
//     static int CompFn(int flag, struct dynlib_block_t *block) {
//         return LibdynCompFnTempate_PreInit<Template_CreateSharedObjBlock>( flag, block ); // this expands a template for a C-comp fn
//     }
// 
//     // The data for this block managed by the simulator
//     struct dynlib_block_t *block;
// };
// 
// 
// 
// 
// 
// 

