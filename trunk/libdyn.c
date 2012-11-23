/*
    Copyright (C) 2009, 2010, 2011, 2012  Christian Klauer

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
 * libdyn.c - Library for realtime controller implementations
 *
 * Author: Christian Klauer 2009-2012
 *
 * This library interpretes schematics of connected blocks
 * in realtime similar to scicos or simulink
 * 
 * Schematics are generated with help of lindyn.sci within Scilab
 * as scripts and typically stored in two files *.ipar and *.rpar
 *
 */

/*
 TODO:
 
 - check wheter all blocks are fully connected - 25.7.11 done not tested for now
 - check wheter an inport was already connected before? - Maybe done?

*/




#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "libdyn.h"
#include "plugin_loader.h"

#define absf(a) ( ((a) > 0) ? (a) : -(a) )

#define mydebug(level) if ((level) > 10)



//#define mydebug(level) if ((level) >= 0)

//
// special malloc implementations are possible (for example on mirocontrollers)
// FIXME: These functions are not used at the moment
//

void * libdyn_malloc(struct dynlib_simulation_t *sim, unsigned int size, int type)
{
  return malloc(size);
}

void libdyn_free(struct dynlib_simulation_t *sim, void * p)
{
  free(p);
}


/*
 * Datatype handling
*/

// #define DATATYPE_UNCONFIGURED 0
// #define DATATYPE_FLOAT (1 | (sizeof(double) << 5))
// #define DATATYPE_SHORTFLOAT 4
// #define DATATYPE_INT 2
// #define DATATYPE_BOOLEAN 3
// #define DATATYPE_EVENT 5

int libdyn_config_get_datatype_len(int datatype)
{  // FIXME NEW 25.6. encode the datasize with bitoperations within datatype
  //  BAUSTELLE !
  //
  // datatype = [ datatype_len, 5 Bits for datatype encoding ];
  //
  int datatype_len = datatype >> 5;

  switch (datatype) {
     case DATATYPE_FLOAT :
       return sizeof(double);
       break;
     case DATATYPE_SHORTFLOAT :
       return sizeof(float);
       break;
     case DATATYPE_INT :
       return sizeof(int);
       break;
     case DATATYPE_BOOLEAN : // FIXME: Hmmm
       return sizeof(char);
       break;       
     case DATATYPE_EVENT : // FIXME: Hmmm
       return sizeof(char);
       break;       
  }
}




//
// Simulation handling
//

// This file is generated automatically by the build system
#include "module_list__.h"
#include <string.h>

struct dynlib_simulation_t *libdyn_new_simulation()
{
    int i, ret;
    struct dynlib_simulation_t *sim = (struct dynlib_simulation_t *) malloc(sizeof(struct dynlib_simulation_t));

    sim->blocks_initialised = 0;
    
    sim->t = 0.0;
    sim->stepcounter = 0;
    sim->numID_counter = 0;

    // Initialise the used lists
    sim->execution_list_head = NULL; // noch keine Exec listen verfügbar
    sim->execution_list_tail = NULL;
    sim->execution_sup_list_head = NULL;
    sim->execution_sup_list_tail = NULL;
    sim->allblocks_list_head = NULL;
    sim->allblocks_list_tail = NULL;


    // syncronisation callbacks
    sim->sync_callback.userdat = NULL;
    sim->sync_callback.sync_func = NULL;
    
    // events
    sim->events.Nevents = 0;
    sim->events.event_active = 0;

    // init auto event generator
    sim->num_cm = 0;
    for (i = 0; i < LIBDYN_MAX_EVENTS; ++i)
      sim->clock_multiplier[i] = 0;

    // New comp_func list (computaional function list)
    sim->private_comp_func_list = libdyn_new_compfnlist();
    sim->global_comp_func_list = NULL; // noting set for now
    
    // WENN VERERBUNG NICHT AKTIV
    // BEGIN
    
    // Call functions of all modules
    // They will create new elements of
    // the compfnlist
    // This function is programmed by the make file within module_list__.c
    
    ret = libdyn_siminit_modules(sim);
    
    // kein Master; wird von libdyn_cpp überschrieben, falls ein master eingesetzt wird.
    sim->master = NULL;
    
    // END
    
    return(sim);
}


// Set global Computational function dictionary (called by the user to add new comp_funcs)
void libdyn_simulation_set_cfl(struct dynlib_simulation_t *sim, struct lindyn_comp_func_list_head_t *list)
{
  sim->global_comp_func_list = list;
}

void libdyn_simulation_IOlist_add(struct dynlib_simulation_t *sim, int inout, struct dynlib_block_t *block, int port)
{
  //  inout = 0 -> inport
  //  inout = 0 -> outport

  struct dynlib_portlist_t *listelement = (struct dynlib_portlist_t *) malloc(sizeof(struct dynlib_portlist_t));

  listelement->block = block;
  listelement->port = port;

  listelement->next = (struct dynlib_portlist_t *) 0; // No next list element

  if (sim->simIO.inlist_head == 0) {
    sim->simIO.inlist_head = listelement;
    sim->simIO.inlist_tail = listelement;
  } else {
    struct dynlib_portlist_t *tmp;

    tmp = sim->simIO.inlist_tail;
    sim->simIO.inlist_tail = listelement;
    tmp->next = listelement;
  }

}

void libdyn_del_simulation(struct dynlib_simulation_t *sim)
{
  // Destroy all blocks, if exits
  struct dynlib_block_t *current = sim->allblocks_list_head;

  if (current != 0) {
    do { // Destroy all blocks
      mydebug(1) fprintf(stderr, "#%d - \n", current->numID);
 
      struct dynlib_block_t *tmp = current;
      current = current->allblocks_list_next; // step to the next block in list
      libdyn_del_block(tmp);
    
    } while (current != 0); // while there is a next block in this list
  }
  
  // Destroy the library of computational functions
  libdyn_del_compfnlist(sim->private_comp_func_list);
  
  // Destroy the simulation struct
  free(sim);
  
  
}

// Print all Blocks to stdout
void libdyn_dump_all_blocks(struct dynlib_simulation_t *sim)
{
  fprintf(stderr, "\nDump of all Blocks:\n\n");
  mydebug(2) fprintf(stderr, "Traversing through allblock list:\n");
  struct dynlib_block_t *current = sim->allblocks_list_head;

  
  // was wenn current = 0, ist das initialisiert? - Ja
  if (current != 0) {
    do { // Destroy all blocks
      mydebug(1) fprintf(stderr, "#%d - \n", current->numID);
 
      struct dynlib_block_t *tmp = current;
      current = current->allblocks_list_next; // step to the next block in list
      
      //libdyn_del_block(tmp);
      libdyn_block_dumpinfo(tmp);
    
    } while (current != 0); // while there is a next block in this list
  }
  
  fprintf(stderr, "----------\n");
}


// The allblocks list contains a list of all created blocks in the simulation
void libdyn_add_to_allblocks_list(struct dynlib_simulation_t *simulation, struct dynlib_block_t *block)
{
  if (simulation->allblocks_list_head != 0) { // there is a list
    struct dynlib_block_t *lastblock = simulation->allblocks_list_tail;

    lastblock->allblocks_list_next = block; // add to the tail of the list
    simulation->allblocks_list_tail = block; // Set new tail of list
    block->allblocks_list_next = 0; // Mark end of list
  } else { // begin with list
    simulation->allblocks_list_head = block;
    simulation->allblocks_list_tail = block;
    block->allblocks_list_next = 0; // Mark end of list
  } 
}



struct dynlib_block_t *libdyn_new_block__(struct dynlib_simulation_t *sim, void *comp_func, int *ipar, double *rpar, void *opar, void *extra_par)
{
      int i;
    //  struct block_config_ret_t block_config;

      struct dynlib_block_t *block = (struct dynlib_block_t *) malloc(sizeof(struct dynlib_block_t));

      block->sim = sim;
      
      sim->numID_counter++;
      block->numID = sim->numID_counter;
      block->identification = "noname";

      mydebug(4) fprintf(stderr, "Creating new Block instance numID=#%d\n", block->numID);

      block->inlist = 0;
      block->outlist = 0;
      block->outdata = (void *) 0;
      block->needs_output_update = 1; // Enable initial output calculation based on x0 or for const block or static blocks

      // Load parameter pointers
      block->ipar = ipar;
      block->rpar = rpar;
      block->opar = opar;

      // init events
      block->event_mask = 0;
      block->Nevents = 0;
      //event_map  // no init

      // Pointer to comp function
      block->comp_func = comp_func;
      
      // Block is not successfully initialised by now
      block->block_initialised = 0;

      
      block->irpar_config_id = -1;

      // call comp function and get config
      // All in- and outports should be configured


      libdyn_set_extraparam(block, extra_par);

      int ret = (*block->comp_func)(COMPF_FLAG_CONFIGURE, block);
      if (ret == -1) { // undo block creation
	fprintf(stderr, "ERROR: libdyn_new_block__: Computational function returned an error. Comp. func ptr = %p\n", block->comp_func);
	libdyn_compfnlist_Show_comp_fn(block->sim->private_comp_func_list, block->comp_func);       libdyn_compfnlist_Show_comp_fn(block->sim->global_comp_func_list, block->comp_func);
        libdyn_del_block(block);  // 
        return 0;
      }
      
      // check wheter all ports are configured
      int failed = 0;
      for (i = 0; i < block->Nin; ++i) {
        if (block->inlist[i].datatype == DATATYPE_UNCONFIGURED)  // unconfigured
          {
            failed = 1;
            fprintf(stderr, "ASSERTION FAILD: INPORT %d NOT CONFIGURED in comp func ptr %p\n", i, block->comp_func);
	    libdyn_compfnlist_Show_comp_fn(block->sim->private_comp_func_list, block->comp_func);       libdyn_compfnlist_Show_comp_fn(block->sim->global_comp_func_list, block->comp_func);
          }
      }
      for (i = 0; i < block->Nout; ++i) {
        if (block->outlist[i].datatype == DATATYPE_UNCONFIGURED)  // unconfigured
          {
            failed = 1;
            fprintf(stderr, "ASSERTION FAILD: OUTPORT %d NOT CONFIGURED in comp func ptr %p\n", i, block->comp_func);
	    libdyn_compfnlist_Show_comp_fn(block->sim->private_comp_func_list, block->comp_func);       libdyn_compfnlist_Show_comp_fn(block->sim->global_comp_func_list, block->comp_func);
          }
      }

      if (failed == 1) { // undo block creation
        libdyn_del_block(block);
        return 0;
      }

      if (block->own_outcache == 0) {

        // Reserve mem for output cache
        int mem_needed = 0;
        for (i = 0; i < block->Nout; ++i) {
          mem_needed += block->outlist[i].datasize;
        }

        mydebug(1) fprintf(stderr, "Output size %d Bytes\n", mem_needed);

        void *outdata = (void *) malloc(mem_needed);
	memset(outdata, 0, mem_needed); // zero this memory initiallly
        block->outdata = outdata;

        // Share this memory among all outputs
        int ofset = 0;
        for (i = 0; i < block->Nout; ++i) {
          block->outlist[i].data = (void *) (block->outdata + ofset);
          ofset += block->outlist[i].datasize;
        }

      } else { // Block uses its own outputcache
        if (block->own_outcache_toconfigure != 0) {
          fprintf(stderr, "ERROR: NOT ALL OUTPUTS HAVE CONFIGURED CACHES in comp func ptr %p\n", block->comp_func);
	  libdyn_compfnlist_Show_comp_fn(block->sim->private_comp_func_list, block->comp_func);       libdyn_compfnlist_Show_comp_fn(block->sim->global_comp_func_list, block->comp_func);
          // undo block creation
       
          libdyn_del_block(block);
          return 0;
        }
      }

      // Insert into list of blocks (allblocks)
      libdyn_add_to_allblocks_list(sim, block);

      // Insert into state update execution list if dynamic block
      if (block->block_type == BLOCKTYPE_DYNAMIC)
        libdyn_add_to_sup_execution_list(sim, block);

      return(block);
 // exit:
}

struct dynlib_block_t *libdyn_new_block(struct dynlib_simulation_t *sim, void *comp_func, int *ipar, double *rpar, void *opar, void *extra_par)
{
  return libdyn_new_block__(sim, comp_func, ipar, rpar, opar, extra_par);
}


void libdyn_config_outcache(struct dynlib_block_t *block, int Nout,  void *data)
{
  block->own_outcache_toconfigure--; // FIXME: Hier richtige prüfung auf configuration einbauen
  block->outlist[Nout].data = data;
}

void libdyn_config_block(struct dynlib_block_t *block, int block_type, int Nout, int Nin, void *work, int own_outcache)
{
  int i,j;

  block->block_type = block_type;
  block->d_feedthrough = 0;  // start with no df; when an output is configured this may change// d_feedthrough;
  block->Nout = Nout;
  block->Nin = Nin;
  block->work = work;
  block->own_outcache = own_outcache;
  block->own_outcache_toconfigure = Nout; // Nout output caches that have to be set-up!

  // Init List of Input connectors
  if (Nin != 0) {
  struct dynlib_inlist_t *inlist = (struct dynlib_inlist_t *) malloc(block->Nin * sizeof(struct dynlib_inlist_t));

  block->inlist = inlist;

  for (i = 0; i < block->Nin; ++i) {
    inlist[i].intype = INTYPE_UNDEF;  // Block unconnected
    inlist[i].datatype = DATATYPE_UNCONFIGURED; // initially set to unconfigured

    // reset list of input junction points
    for (j = 0; j < INPUT_MAX_CONNECTIONS; ++j) {
       inlist[i].next_juncture_block[j] = (struct dynlib_block_t *) 0; // leere Liste für junction point
       inlist[i].next_juncture_input[j] = 0;
       inlist[i].input_available[j] = 0;
    }
  }
  } else block->inlist = NULL;

  
  // Init List of Output connectors
  if (Nout != 0) {
  struct dynlib_outlist_t *outlist = (struct dynlib_outlist_t *) malloc(block->Nout * sizeof(struct dynlib_outlist_t));

  block->outlist = outlist;

  for (i = 0; i < block->Nout; ++i) {
    outlist[i].outtype = OUTTYPE_UNDEF; // Block unconnected
    outlist[i].datatype = DATATYPE_UNCONFIGURED; // initially set to unconfigured

    // reset list of input junction points
    //for (j = 0; j < INPUT_MAX_CONNECTIONS; ++j) {
      outlist[i].post = 0; // No connected block for every out i at first
      outlist[i].post_input = 0;
      outlist[i].post_subinput = 0;
    //}
  }
  } else block->outlist = NULL;
}

// Tell computaional functio about extra parameters
void libdyn_block_notify_extrapar(struct dynlib_block_t *block, void *par)
{  // Call comp func with extra parameters
      libdyn_set_extraparam(block,par);

      int ret = (*block->comp_func)(COMPF_FLAG_SETEXTRAPAR, block);
}

void libdyn_block_setid(struct dynlib_block_t *block, char *id)
{
  block->identification = id;
}



int libdyn_config_block_input(struct dynlib_block_t *block, int in, int len, int datatype)
{
  if ((in < block->Nin) && (in >= 0)) {

 // block->inlist[in].intype = intype; // Wir bei Block connect gesetzt
    block->inlist[in].len = len;
    block->inlist[in].datatype = datatype;

    mydebug(1) fprintf(stderr, "confiured inport #%d\n", in);
    return 0;
  } else {
    fprintf(stderr, "ASSERTION FAILD: libdyn_config_block_input: TRIED TO CONFIGURE A NON-AVAILABLE INPUT\n  You made a mistake in your computational c-function in comp func ptr %p\n", block->comp_func);
    libdyn_compfnlist_Show_comp_fn(block->sim->private_comp_func_list, block->comp_func);       libdyn_compfnlist_Show_comp_fn(block->sim->global_comp_func_list, block->comp_func);    
    return -1;
  }
}

//#define libdyn_config_block_output(block,out,len,datatype) libdyn_config_block_output_ext((block),(out),(len),(datatype),);

int libdyn_config_block_output(struct dynlib_block_t *block, int out, int len, int datatype, int dinput_dependence)
{
  if ((out < block->Nout) && (out >= 0)) {
//  block->outlist[out].outtype = OUTTYPE_DIRECT; // Wird bei connect gesetzt


    block->outlist[out].len = len; 
    block->outlist[out].datatype = datatype;  
    block->outlist[out].datasize = len * libdyn_config_get_datatype_len(datatype); //sizeof(double); // FIXME SOLVED Hier auf Datentyp eingehen
 
    if (dinput_dependence == 1 && block->Nin == 0) {
       fprintf(stderr, "ASSERTION FAILD: libdyn_config_block_output; TRIED TO HAVE DFEED WITH NO INPUT in comp func ptr %p\n", block->comp_func);
       libdyn_compfnlist_Show_comp_fn(block->sim->private_comp_func_list, block->comp_func);       libdyn_compfnlist_Show_comp_fn(block->sim->global_comp_func_list, block->comp_func);
       return -1;
    }
    block->outlist[out].dinput_dependence = dinput_dependence; // FIXME check if there is an input
    if (dinput_dependence == 1)
      block->d_feedthrough = 1; // At least one d feedthrough 
      

    mydebug(1) fprintf(stderr, "confiured outport #%d df=%d\n", out, block->d_feedthrough);
    return 0;
  } else {
    fprintf(stderr, "ASSERTION FAILD: libdyn_config_block_output: TRIED TO CONFIGURE A NON-AVAILABLE OUTPUT in comp func ptr %p\n", block->comp_func);
    libdyn_compfnlist_Show_comp_fn(block->sim->private_comp_func_list, block->comp_func);       libdyn_compfnlist_Show_comp_fn(block->sim->global_comp_func_list, block->comp_func);
    return -1;
  }
  
  
}

// Print info about a block instance to stdout
void libdyn_block_dumpinfo(struct dynlib_block_t *block)
{
  int i;
  struct dynlib_inlist_t *inlist;
  inlist = block->inlist;
  struct dynlib_outlist_t *outlist;
  outlist = block->outlist;

  fprintf(stderr, "------- INFO ON BLOCK INSTANCE ----------\n");
  fprintf(stderr, " #%d (%s) irpar_cfg_id = %d\n", block->numID, block->identification, block->irpar_config_id);
  fprintf(stderr, "\n");
  fprintf(stderr, "block_type=%d, d_feedthrough=%d\n", block->block_type, block->d_feedthrough);
  fprintf(stderr, "Nout=%d, Nin=%d\n", block->Nout, block->Nin);
  fprintf(stderr, "comp fn ptr = %p\n",  (void*) block->comp_func);
  fprintf(stderr, "\n");

  fprintf(stderr, "INPUTS:\n");
  for (i = 0; i < block->Nin; ++i) {
    fprintf(stderr, "    IN #%d: intype=%d, len=%d, datatype=%d, nrInput=%d, datptr=%p\n",i,inlist[i].intype, inlist[i].len, inlist[i].datatype, inlist[i].nrInput,  (void*) inlist[i].data);

    //fprintf(stderr, "Bla\n");
    struct dynlib_block_t *cbl = inlist[i].pre[0];
    switch (inlist[i].intype) {
      case INTYPE_UNDEF:
        fprintf(stderr, "      No connection!\n");
        break;
      case INTYPE_DIRECT:
        fprintf(stderr, "      Connected Block: #%d (%s); output #%d\n", cbl->numID, cbl->identification, 0 );
        break;
      case INTYPE_EXTERN:
        fprintf(stderr, "      External Input Data at %p\n",  (void*) inlist->data);
        break;
    }


  }

  fprintf(stderr, "\nOUTPUTS:\n");

  for (i = 0; i < block->Nout; ++i) {
    fprintf(stderr, "    OUT #%d: outtype=%d, len=%d, datatype=%d, datptr=%p\n",i,outlist[i].outtype, outlist[i].len, outlist[i].datatype,  (void*) outlist[i].data);

    //fprintf(stderr, "Bla\n");
    struct dynlib_block_t *cbl = outlist[i].post;
    int cbli = outlist[i].post_input;

    if (outlist[i].outtype != OUTTYPE_UNDEF) {
      fprintf(stderr, "      Connected Block: #%d (%s); input #%d\n", cbl->numID, cbl->identification, cbli );
 
      // get next (block,#in) of junction point

      __libdyn_macro_junctionlist_head;

      while(current != 0) {
        fprintf(stderr, "      Connected Block: #%d (%s); input #%d\n", current->numID, current->identification, currenti );

        __libdyn_macro_junctionlist_next;
      }

    } else {
       fprintf(stderr, "      No connection!\n");
    }
  }
  fprintf(stderr, "\nBlock says:\n\n");
  
  int ret = (*block->comp_func)(COMPF_FLAG_PRINTINFO, block);
  
  fprintf(stderr, "---------------- END --------------------\n");
}

void libdyn_del_block(struct dynlib_block_t *block)
{
  if (block->block_initialised == 1) {
    // call destructor of Comp function ONLY if the block was successfully initialised
    int ret = (*block->comp_func)(COMPF_FLAG_DESTUCTOR, block);
  }

  //Output cache löschen
  if (block->outdata != 0) free(block->outdata);
  if (block->inlist != 0) free(block->inlist);
  if (block->outlist != 0) free(block->outlist);
  
  free(block);
}

int libdyn_block_connect(struct dynlib_block_t *blockfrom, int outNr, struct dynlib_block_t *blockto, int inNr)
{
  // FIXME: POSPONED - MAYBE FOREVER Subinput are not considered
  //  DONE Testen ob port schon verbunden ist  
  // 14.8.2012 - fixed a possible segfault with incorrect error handling
  

  // check conditions
  int test = 0;

  if ((inNr < 0) || (inNr >= blockto->Nin)) 
    { test = -1; goto err_nonavailin; }
  if ((outNr < 0) || (outNr >= blockfrom->Nout))
    { test = -2; goto err_nonavailout; }

  if (blockto->inlist[inNr].len != blockfrom->outlist[outNr].len)
    { test = -3; goto err; }
  if (blockto->inlist[inNr].datatype != blockfrom->outlist[outNr].datatype)
    { test = -4; goto err; }

  if (blockto->inlist[inNr].intype != INTYPE_UNDEF) // Dst inport already connected
    { test = -5; goto err; }

  // Direct connect of OUtput #outNr of fromblock to input #inNr of blockto
  blockto->inlist[inNr].pre[0] = blockfrom; // index [0] since no addition point is used (only direct connect)
  blockto->inlist[inNr].outNr[0] = outNr;
  blockto->inlist[inNr].intype = INTYPE_DIRECT;
  blockto->inlist[inNr].data = blockfrom->outlist[outNr].data; // Abkürzung zu output cache

  blockfrom->outlist[outNr].outtype = OUTTYPE_DIRECT;

  // update junction point list
  struct dynlib_block_t *tmp;
  int tmpi;

  tmp = blockfrom->outlist[outNr].post; 
  tmpi = blockfrom->outlist[outNr].post_input;
  
  blockfrom->outlist[outNr].post = blockto; // New list head; Connection from output to input
  blockfrom->outlist[outNr].post_input = inNr;

  blockto->inlist[inNr].next_juncture_block[0] = tmp; // connect the previous head element to the new head element
  blockto->inlist[inNr].next_juncture_input[0] = tmpi;

  return 1;

err_nonavailin:
  fprintf(stderr, "ERROR CONNECTING BLOCK irparid = %d TO %d error code=%d!\n", blockfrom->irpar_config_id, blockto->irpar_config_id, test);
  fprintf(stderr, "  Ports to connect: inport = %d, outport = %d\n", inNr, outNr);
  fprintf(stderr, "  Number of ports: from->Nout = %d, to->Nin = %d\n", blockfrom->Nout, blockto->Nin);
  fprintf(stderr, "  Tryed to connect to an non available input\n");
  fprintf(stderr, "\n");

  return test;

err_nonavailout:
  fprintf(stderr, "ERROR CONNECTING BLOCK irparid = %d TO %d error code=%d!\n", blockfrom->irpar_config_id, blockto->irpar_config_id, test);
  fprintf(stderr, "  Ports to connect: inport = %d, outport = %d\n", inNr, outNr);
  fprintf(stderr, "  Number of ports: from->Nout = %d, to->Nin = %d\n", blockfrom->Nout, blockto->Nin);
  fprintf(stderr, "  Tryed to connect to an non available output\n");
  fprintf(stderr, "\n");

  return test;

err:
  fprintf(stderr, "ERROR CONNECTING BLOCK irparid = %d TO %d error code=%d!\n", blockfrom->irpar_config_id, blockto->irpar_config_id, test);
  fprintf(stderr, "  The following missmatching interface was specified:\n\n");
  fprintf(stderr, "  Ports to connect: inport = %d, outport = %d\n", inNr, outNr);
  fprintf(stderr, "  Number of ports: from->Nout = %d, to->Nin = %d\n", blockfrom->Nout, blockto->Nin);
  fprintf(stderr, "  srclen = %d, dstlen = %d\n", blockfrom->outlist[outNr].len, blockto->inlist[inNr].len);
  fprintf(stderr, "  srcdatatype = %d, dstdatatype = %d\n\n", blockfrom->outlist[outNr].datatype, blockto->inlist[inNr].datatype);
  fprintf(stderr, "\n");
  
  return test;
}

// Connect Block input port to external data pointer *external // ADD datatype
int libdyn_block_connect_external(struct dynlib_block_t *blockto, int inNr, void *external_data, int portsize)
{
  if ((inNr < 0) || (inNr >= blockto->Nin)) {
    fprintf(stderr, "Faild to connect external of block irparid=%d; invalid port num inNr = %d not in [0,%d]\n", blockto->irpar_config_id, inNr, blockto->Nin);
    return -1;
  }
  
  if (portsize != blockto->inlist[inNr].len) {
    fprintf(stderr, "Faild to connect external of block irparid=%d; invalid port sizes %d (extern in) != %d (blockto)\n", blockto->irpar_config_id, portsize, blockto->inlist[inNr].len );
    return -2;
  }
  
  blockto->inlist[inNr].intype = INTYPE_EXTERN;
  blockto->inlist[inNr].pre[0] = (struct dynlib_block_t *) external_data; // Feld wird missbraucht // misstreated field
  blockto->inlist[inNr].data = external_data;
 
  return 0;
}


//
// Events handling
//

int libdyn_new_event(struct dynlib_simulation_t *simulation)
{
      // init events
   //   block->events.Nevents = 0;
      //block->events.event_active = 0;

  simulation->events.Nevents++; //
  return simulation->events.Nevents -1;
}


// Register the block to a defined event
int libdyn_new_event_notifier(struct dynlib_simulation_t *simulation, struct dynlib_block_t *block, int event)
{
  // Insert event #event into list of subscriptions

  if (event > 32)
    return 1;

  block->event_map[block->Nevents] = event;
  block->Nevents++;
  block->event_mask |= (1 << event);
  
  return 0;
}

// Map the global simulation events to the blocks ones
int libdyn_event_mapper(struct dynlib_simulation_t *simulation, struct dynlib_block_t *block)
{
  int i;
  int block_events = 0;
  char eventN;

  mydebug(1) fprintf(stderr, "simulation events = %x\n", simulation->events.event_active);

  for (i = 0; i < block->Nevents; ++i) { // for all events registered by block   FIXME: Do this more efficient
    eventN = block->event_map[i];
    mydebug(1)  fprintf(stderr, "is sim event %d triggered?\n", eventN);

    // Teste, ob eventN gesetzt
    if (simulation->events.event_active & (1 << eventN)) {
      block_events |= (1 << i);
      mydebug(1) fprintf(stderr, "yes!\n");
    }
  }

  return block_events;
}

int libdyn_event_check(struct dynlib_block_t *block, int event)
{
  char eventN = block->event_map[event];
 
  if (block->sim->events.event_active & (1 << eventN))
    return 1;
  
  return 0;
}

void libdyn_event_trigger(struct dynlib_simulation_t *simulation, int event)
{
  mydebug(1) fprintf(stderr, "triigering event %d\n", event);

  simulation->events.event_active |= (1 << event);
}

// Trigger multiple events
void libdyn_event_trigger_mask(struct dynlib_simulation_t *simulation, int event_mask)
{
  mydebug(1) fprintf(stderr, "triigering event mask %x\n", event_mask);

  simulation->events.event_active = event_mask; // FIXME: Do not overwrite! Setting &= does not behave correctly
}



void libdyn_event_reset(struct dynlib_simulation_t *simulation)
{
  simulation->events.event_active = 0;
}


//
// Configurable autogeneration of periodic events
//
int libdyn_clock_event_generator_addclock(struct dynlib_simulation_t *simulation, int mult)
{
  if (simulation->num_cm < LIBDYN_MAX_EVENTS) {
    simulation->clock_multiplier[simulation->num_cm] = mult;
    simulation->num_cm++;
  }
}

void libdyn_clock_event_generator(struct dynlib_simulation_t *simulation)
{
  int i;

  for (i = 0; i < simulation->num_cm; ++i) {
    if ((simulation->stepcounter % simulation->clock_multiplier[i]) == 0) {
      mydebug(2) fprintf(stderr, "autoevent %d (mul=%d)\n", i, simulation->clock_multiplier[i]);
      libdyn_event_trigger(simulation, i);
    }
  }
}


/*
 *  Check whether all inputs are connected
 */
int libdyn_simulation_checkinputs(struct dynlib_simulation_t * sim)
{
  int fault = 0;
  
  struct dynlib_block_t *current = sim->allblocks_list_head;
  
  if (current != 0) {
    do { // Destroy all blocks
      struct dynlib_block_t *block = current;
      

      int i;
      for (i = 0; i<block->Nin; ++i) {
	if (block->inlist[i].data == NULL) {
	  fprintf(stderr, "libdyn: error: port %d of block with irparid=%d of blocktype %d is not connected\n", i, block->irpar_config_id, block->block_type);
	  
	  fault = -1;
	}
      }
/*      int ret = (*block->comp_func)(COMPF_FLAG_RESETSTATES, block);
       if (ret == -1) {
	 ;
       }*/
    
      current = current->allblocks_list_next; // step to the next block in list
    } while (current != 0); // while there is a next block in this list
  }

  return fault;
}


//
// Call COMPF_FLAG_INIT for each block and abort in case of an error
//

// int libdyn_simulation_init(struct dynlib_simulation_t * sim) // obsolete and unused
// {
//   struct dynlib_block_t *current;
//   int counter;  
//   int abort_at;
//   
//   
//   
//   
//   current = sim->allblocks_list_head;
//   counter = 0;
//   if (current != 0) {
//     do { // Destroy all blocks
//       struct dynlib_block_t *block = current;
//       
// //      fprintf(stderr, "init id=%d\n", block->irpar_config_id);
//       
//       int ret = (*block->comp_func)(COMPF_FLAG_INIT, block);
//        if (ret == -1) {
//  	 fprintf(stderr, "WARNING (for now): ERROR: libdyn_simulation_init: Computational function returned an error blockid=nn, block_irparid=%d\n", block->irpar_config_id);
//          abort_at = counter;
// 	 goto undo_everything;
//        }
//        
//        // Mark the block as successfully initialised
//        block->block_initialised = 1;
//     
//       current = current->allblocks_list_next; // step to the next block in list
//       counter++;
//     } while (current != 0); // while there is a next block in this list
//   }
//   
//   //fprintf(stderr, "init done\n");
//   sim->blocks_initialised = 1;
//   
//   return 0;
// 
//   
// undo_everything :
//   // Call destructor of all initialiesed blocks
//   
//   current = sim->allblocks_list_head;
//   counter = 0;  
//   
//   if (current != 0) {
//     do { // Destroy all blocks
//       struct dynlib_block_t *block = current;
// 
//       if (counter == abort_at) // at this position there was an error last time
// 	return -1;
//       
//       int ret = (*block->comp_func)(COMPF_FLAG_DESTUCTOR, block);
//       
//       current = current->allblocks_list_next; // step to the next block in list
//       counter++;
//     } while (current != 0); // while there is a next block in this list
//   }
//   
//   return -1;
// }

int libdyn_simulation_init(struct dynlib_simulation_t * sim)
{
  int ret;
  
  // call the pre-init flag for all blocks
  ret = libdyn_simulation_callinitflag(sim, COMPF_FLAG_PREINIT, COMPF_FLAG_PREINITUNDO); 
  if (ret < 0)
    return ret;
  
  // call the init flag for all blocks
  ret = libdyn_simulation_callinitflag(sim, COMPF_FLAG_INIT, COMPF_FLAG_DESTUCTOR);  
  if (ret < 0)
    return ret;

  // call the post-init flag for all blocks. -1 means no fault handling
  libdyn_simulation_callinitflag(sim, COMPF_FLAG_POSTINIT, -1 ); 
  
  
}


int libdyn_simulation_callinitflag(struct dynlib_simulation_t * sim, int initflag, int destructorflag)
{
  struct dynlib_block_t *current;
  int counter;  
  int abort_at;
  
  
  
  
  current = sim->allblocks_list_head;
  counter = 0;
  if (current != 0) {
    do { // Destroy all blocks
      struct dynlib_block_t *block = current;
      
//      fprintf(stderr, "init id=%d\n", block->irpar_config_id);
      
      int ret = (*block->comp_func)(initflag, block);
       if (ret == -1 && destructorflag != -1) { // check for errors if a destructorflag =! -1 is given
 	 fprintf(stderr, "WARNING (for now): ERROR: libdyn_simulation_init: Computational function returned an error blockid=nn, block_irparid=%d\n", block->irpar_config_id);
         abort_at = counter;
	 goto undo_everything;
       }
       
       // Mark the block as successfully initialised
       block->block_initialised = 1;
    
      current = current->allblocks_list_next; // step to the next block in list
      counter++;
    } while (current != 0); // while there is a next block in this list
  }
  
  //fprintf(stderr, "init done\n");
  sim->blocks_initialised = 1;
  
  return 0;

  
undo_everything :
  // Call destructor of all initialiesed blocks
  
  current = sim->allblocks_list_head;
  counter = 0;  
  
  if (current != 0) {
    do { // Destroy all blocks
      struct dynlib_block_t *block = current;

      if (counter == abort_at) // at this position there was an error last time
	return -1;
      
      int ret = (*block->comp_func)(destructorflag, block);
      
      current = current->allblocks_list_next; // step to the next block in list
      counter++;
    } while (current != 0); // while there is a next block in this list
  }
  
  return -1;
}

 /*  
  * The callback function "sync_func" is called before any of the output or state-update flags
  * are called. 
  * If 0 is returned, the simulation will continue to run
  * If 1 is returned, the simulation will pause and has to be re-triggered externally.
  * e.g. by the trigger_computation input of the async nested_block.
  */

libdyn_simulation_setSyncCallback(struct dynlib_simulation_t *simulation, int (*sync_func)( struct dynlib_simulation_t * sim ), void *userdat)
{
  simulation->sync_callback.sync_func = sync_func;
  simulation->sync_callback.userdat = userdat;
  simulation->sync_callback.sync_callback_state = 0;
}

//
// Go one step further in simulation
//

int libdyn_simulation_step(struct dynlib_simulation_t *simulation, int update_states) // Einen Schritt weitergehen
{
  //
  // Check wheter blocks are initialised (in case the user forgot that)
  //
  
  if (simulation->blocks_initialised == 0)
    libdyn_simulation_init(simulation);
  
    
  struct dynlib_block_t *current; // Iterator for executionlists.
  
  //
  // Calc outputs if update_states == 0
  //   
  
  if (update_states == 0) {
    
    
//     /*
//      * Wait for synhronisation callback function
//     */
//     
//     if (simulation->sync_callback.sync_func != NULL) {
//       simulation->sync_callback.sync_callback_state = (*simulation->sync_callback.sync_func)(simulation->sync_callback.userdat);
//       if (simulation->sync_callback.sync_callback_state == 1) {
// 	return 1;
//       }
//       
//       // sync_callback returned --> output calculation.
//     }
    
    
    
    
    //
    // traverse execution list for output update
    //

    mydebug(2) fprintf(stderr, "Traversing through execution list update_states==0:\n");
    current = simulation->execution_list_head;
    
    do {
      mydebug(1) fprintf(stderr, "#%d - ", current->numID);

	// Only d-Feedthtough Blocks have to be executed if update_states = false AND since the last outcalc there was no state update,
	// since output SHOULD not change, but its still ok to execute them again (But performance) USE block->needs_output_update

	// exec comp function

	int ret;
 	if (current->d_feedthrough == 1) { // exec all dfeed blocks // FIXME: ONly call them when there is an event, so CPU could be saved in multirate systems
// 	if (current->d_feedthrough == 1 && __libdyn_event_triggered(simulation, current)) { // exec all dfeed blocks 
// 	  int block_events = libdyn_event_mapper(simulation, current); // getriggerte events updaten
// 	  current->block_events = block_events;
// 	  mydebug(1) fprintf(stderr, "Block events %d %x\n", block_events, block_events);

	  ret = (*current->comp_func)(COMPF_FLAG_CALCOUTPUTS, current);
	} else if (current->needs_output_update == 1) { // exec soly dynamic blocks (without dfeed) only of state was updated or initialy 

	  mydebug(1) fprintf(stderr, "Dynamischer Block out update fn ptr = %x\n",  (unsigned int) current->comp_func);
	  ret = (*current->comp_func)(COMPF_FLAG_CALCOUTPUTS, current);
	  current->needs_output_update = 0;
	} //else if (1 == 1) // FIXME: Introduce time dependent Blocks, whose out is regularly calculated (Function generators)
	  
      current = current->exec_list_next; // step to the next block in list
    } while (current != 0); // while there is a next block in this list

    mydebug(1) fprintf(stderr, "\n");
    
    return 0;
  } // if update_states == 0


  //
  // Update states if update_states == 1
  //

  if (update_states == 1) {
    
    
    simulation->stepcounter++; // stepcounter wird nur bei stateupdates weitergeführt

    //
    // traverse execution list for updating states
    //

    if (simulation->execution_sup_list_head == 0) { // there is no block in sup list
      mydebug(2) fprintf(stderr, "No block in SUP execution list\n");
    } else {

      mydebug(2) fprintf(stderr, "Traversing through SUP execution list  update_states==1:\n");
      current = simulation->execution_sup_list_head;


      do {
	mydebug(0) fprintf(stderr, "#%d - ", current->numID);

	if (__libdyn_event_triggered(simulation, current)) { // Call update_states only if there is an event for this block
	  int block_events = libdyn_event_mapper(simulation, current);
	  current->block_events = block_events;
	  mydebug(1) fprintf(stderr, "Block events %d %x\n", block_events, block_events);

	  // exec comp function

	  current->needs_output_update = 1; // Set before comp f is executed, so the function may decide to not update its outputs next time
	  int ret = (*current->comp_func)(COMPF_FLAG_UPDATESTATES, current);

	} else {
	  mydebug(1) fprintf(stderr, "Block received no events\n");
	}


	current = current->exec_sup_list_next; // step to the next block in list
      } while (current != 0); // while there is a next block in this list

      mydebug(0) fprintf(stderr, "\n");

    }
    // Reset all events
    libdyn_event_reset(simulation);

    return 0;
  } // if update_states == 1

}


/*
 *  Reset all Blocks in the Simulation
 */
void libdyn_simulation_resetblocks(struct dynlib_simulation_t * sim)
{
  struct dynlib_block_t *current = sim->allblocks_list_head;
  
  if (current != 0) {
    do { // Destroy all blocks
      struct dynlib_block_t *block = current;
      
//      fprintf(stderr, "init id=%d\n", block->irpar_config_id);
      
      int ret = (*block->comp_func)(COMPF_FLAG_PREPARERESET, block);
      ret = (*block->comp_func)(COMPF_FLAG_RESETSTATES, block);
/*      if (ret == -1) {
	;
      }*/
    
      current = current->allblocks_list_next; // step to the next block in list
    } while (current != 0); // while there is a next block in this list
  }
  
  // reset the  simulation step counter
  sim->stepcounter = 0;
  
  // reset the sync_callback_state
  sim->sync_callback.sync_callback_state = 0;
}



//
//
// Algorithm which sets-up the correct order of block execution for output calculation
//
//

//
// The junctionlist is a list for each outport that lists all inputs which are connected
// to the outport
//

void libdyn_junctionlist_head(struct dynlib_block_t *block, int out, struct dynlib_block_t **cblptr, int *cbliptr) // List all blocks connected with "block"'s output #out
{
    *cblptr = block->outlist[out].post;
    *cbliptr = block->outlist[out].post_input;
}

void libdyn_junctionlist_next(struct dynlib_block_t **cblptr, int *cbliptr) // List all blocks connected with "block"'s output #out
{
  struct dynlib_block_t *current = *cblptr; 
  int currenti = *cbliptr;


  struct dynlib_block_t *next; 
  int nexti; 
 
  
  next = current->inlist[currenti].next_juncture_block[0]; 
  nexti = current->inlist[currenti].next_juncture_input[0]; 
 
  *cblptr = next;
  *cbliptr = nexti;
}


// list helper
int libdyn_add_to_execution_list(struct dynlib_simulation_t *simulation, struct dynlib_block_t *block)
{
  if (block->block_executed == 1) {
    mydebug(2) fprintf(stderr, "Block aready included in list\n");
    return 0;
  }
  mydebug(2) fprintf(stderr, "Block added to exec list\n");

  if (simulation->execution_list_head != 0) { // there is a list
    struct dynlib_block_t *lastblock = simulation->execution_list_tail;

    lastblock->exec_list_next = block; // add to the tail of the list
    simulation->execution_list_tail = block; // Set new tail of list
    block->exec_list_next = 0; // Mark end of list
  } else { // begin with list
    simulation->execution_list_head = block;
    simulation->execution_list_tail = block;
    block->exec_list_next = 0; // Mark end of list
  } 

  block->block_executed = 1; // Mark as allredy included in list
  return 1;
}

// list helper
void libdyn_add_to_sup_execution_list(struct dynlib_simulation_t *simulation, struct dynlib_block_t *block)
{
  mydebug(1) fprintf(stderr, "suplist add\n");

  if (simulation->execution_sup_list_head != 0) { // there is a list
    struct dynlib_block_t *lastblock = simulation->execution_sup_list_tail;

    lastblock->exec_sup_list_next = block; // add to the tail of the list
    simulation->execution_sup_list_tail = block; // Set new tail of list
    block->exec_sup_list_next = 0; // Mark end of list
  } else { // begin with list
    simulation->execution_sup_list_head = block;
    simulation->execution_sup_list_tail = block;
    block->exec_sup_list_next = 0; // Mark end of list
  } 
}

// Check the availability of a block input,
// 
int libdyn_setup_executionlist_setandcheck_avail(struct dynlib_block_t *block, int in) // set block's input #in available and check other inputs for completeness
{ 
  block->inlist[in].input_available[0] = 1;
  block->remaining_input_availability--; 

  if (block->remaining_input_availability == 0) {
    return 1;
    mydebug(1) fprintf(stderr, "no remaining inputs\n");
  } else {
    return 0;
    //fprintf(stderr, "Block already on execlist\n");
  }
}

// the algorithm itself
int libdyn_setup_executionlist(struct dynlib_simulation_t *simulation)
{
  // SUP liste durchgehen, nur die non dfeed Blöcke nehmen -> X
  // UND Liste mit exteren inputs durchgehen -> ADD(X)

  // Liste x=ELEMENT(X) durchgehen
  //   Alle inputs verbundener Blöcke des blocks x auf available setzen
  //   Sobald alle inputs des auf available gesetzten inputs's block verfügbar -> add x to execution list. und diesen Block auf list Y

  //   X = Y, repeat

  mydebug(0) fprintf(stderr, "sup exec list head: %p\n", simulation->execution_sup_list_head);

/////////////////

mydebug(2) fprintf(stderr, "++++++++++ Setting up execution list +++++++\n");

  int i;
  struct dynlib_block_t *current;

  // Liste mit allen Blöcken durchgehen (initialer Durchgang) alle externen INS durchgehen

  mydebug(2) fprintf(stderr, "Traversing through allblock list:\n");
  current = simulation->allblocks_list_head;

  do {
    mydebug(1) fprintf(stderr, "#%d - \n", current->numID);
    current->remaining_input_availability = current->Nin; // Alle inputs noch nicht bekannt
    current->block_executed = 0;

    // setze und überprüfe input verfügbarkeit für jeden block

    int all_inputs_available = 1;  // If this stays true after the following loop the block is complete for execution
    for (i = 0; i < current->Nin; ++i) { // check current block's inputs
      //current->inlist[i];
      current->inlist[i].input_available[0] = 0; // initialise to zero for the first time
      
      mydebug(1) fprintf(stderr, "it%d\n", current->inlist[i].intype);
      if (current->inlist[i].intype == INTYPE_EXTERN) {
        mydebug(1) fprintf(stderr, "*");
        if (libdyn_setup_executionlist_setandcheck_avail(current, i) == 1) {
          mydebug(1) fprintf(stderr, "Yeah 1\n");
        } else {
//          current->inlist[i].input_available[0] = 0; // FIXME: BUG --> solved initialise to zero otherwise
        }

        //current->inlist[i].input_available[0] = 1;
      }

      if (current->inlist[i].input_available[0] == 0) // If at least one input is not available the block can't be executed
        all_inputs_available = 0;
    }

    if (all_inputs_available) { // FIXME dies bei setandcheck_avail == 1
 mydebug(2)      fprintf(stderr, "all ins available\n");
      // Block "current" kann ausgeführt werden
     mydebug(1)  fprintf(stderr, "Yeah 2\n");
      libdyn_add_to_execution_list(simulation, current);
    }

    current = current->allblocks_list_next; // step to the next block in list
  } while (current != 0); // while there is a next block in this list



  // Liste mit allen Blöcken durchgehen (zweiter initialer Durchgang) - alle puren dyn blöcke finden

  mydebug(2) fprintf(stderr, "Traversing through allblock list (second time):\n");
  current = simulation->allblocks_list_head;

  do {
    mydebug(1) fprintf(stderr, "#%d - df=%d, btyp=%d\n", current->numID, current->d_feedthrough, current->block_type);

    if (current->d_feedthrough == 0 && current->block_type == BLOCKTYPE_DYNAMIC) { // If this is a sole dynamic block (no dfeed) the to its outputs connected inputs are available
      libdyn_add_to_execution_list(simulation, current); 

mydebug(1) fprintf(stderr, "Yeah 2\n");

    }

    current = current->allblocks_list_next; // step to the next block in list
  } while (current != 0); // while there is a next block in this list

  mydebug(1) fprintf(stderr, "\n");

  

  mydebug(2) fprintf(stderr, "Traversing through execution list:\n");
  current = simulation->execution_list_head; // FIXME: BUG! Liste kann noch auf Null stehen, 
							    // wenn eine nur dynamic block da ist
  
  if (simulation->execution_list_head == 0) { // there is no list
    fprintf(stderr, "ERROR: There are no blocks on the execution list!\n");
    return -1;
  }  
    
  mydebug(2) fprintf(stderr, "head is at %x\n", (unsigned int)  current);
   
  do {
    mydebug(1) fprintf(stderr, "#%d - ", current->numID);

    // Die neu auf der executing list eingefügten blocke abarbeiten


    // Wieder alle angeschlossenen Inputs auf available setzten und checken
    // Diese werden dann wieder der exec liste hinzugefügt und von der 
    // haupschleife später iterativ aufgegriffen

      for (i = 0; i < current->Nout; ++i) { // all outports
        // get all blocks connected to the i-th output and mark the available. Further check wherter all inputs are complete

        struct dynlib_block_t *cbl;
        int cbli;

        libdyn_junctionlist_head(current, i, &cbl, &cbli);
        //fprintf(stderr, "c %d, in %d\n", cbl->numID, cbli);

        while(cbl != 0) { // While there are remaining blocks do
          mydebug(3) fprintf(stderr, "conn %d, in %d, blocksptr %x\n", cbl->numID, cbli,  (unsigned int) cbl);
          if (libdyn_setup_executionlist_setandcheck_avail(cbl, cbli) == 1) {
            // Block "cbl" kann ausgeführt werden
            mydebug(1) fprintf(stderr, "Yeah 3\n");
            if (libdyn_add_to_execution_list(simulation, cbl) == 0) {
              mydebug(1) fprintf(stderr, "tail blockid = %d\n", cbl->numID);

              if (cbl->d_feedthrough == 1) {
                fprintf(stderr, "algebraic loop containing block #%d detected\n", cbl->numID);
                return -1;
              }
            }
/*
            if (cbl->d_feedthrough == 0 && cbl->block_type == BLOCKTYPE_DYNAMIC) { // If this is a sole dynamic block (no dfeed) the to
              fprintf(stderr, "tail blockid = %d\n", cbl->numID);
            } else 
              libdyn_add_to_execution_list(simulation, cbl);
*/
          }
          libdyn_junctionlist_next(&cbl, &cbli);
//	fprintf(stderr, "test\n");
        }

      }


    current = current->exec_list_next; // step to the next block in list
  } while (current != 0); // while there is a next block in this list

  
  mydebug(2) fprintf(stderr, "++++++++++ Set-up finished +++++++\n");


  if (simulation->execution_sup_list_head == 0) { // there is no list
    fprintf(stderr, "NOTE: There are no dynamic blocks on the sup execution list!\n");
    fprintf(stderr, "      This mode of libdyn not well tested for now - You are on your own\n");
  } else {
  
    //
    // Dump the SUP Exec Liste
    //
    
    mydebug(8) {
      fprintf(stderr, "List of all sup executed block in execution order %p\n\n", simulation);
      current = simulation->execution_sup_list_head;
      do {
	mydebug(0) fprintf(stderr, "list element: %p\n", current);
	libdyn_block_dumpinfo(current);
	mydebug(0) fprintf(stderr, "*\n");
	
	current = current->exec_sup_list_next; // step to the next block in list
      } while (current != 0); // while there is a next block in this list

      fprintf(stderr, "\n++++++++++++++++++++++++++++++++++++++++++\n\n");
    
    
      fprintf(stderr, "List of all executed block in execution order\n\n");
      current = simulation->execution_list_head;
      do {
	libdyn_block_dumpinfo(current);

	current = current->exec_list_next; // step to the next block in list
      } while (current != 0); // while there is a next block in this list

    
    
      fprintf(stderr, "\n++++++++++++++++++++++++++++++++++++++++++\n\n");
    }
  
  }
  
  fprintf(stderr, "libdyn: successfully compiled schematic\n");
  
  return 0;
}


/*********************************************************/
//
// Implementation of various filters (discrete TF ...)
//
// Move this to another file -> filter.c


struct dynlib_filter_t *libdyn_new_filter(int N)
{
  struct dynlib_filter_t *p = (struct dynlib_filter_t *) malloc(sizeof(struct dynlib_filter_t));
  struct dynlib_tf_filter_t *add = ( struct dynlib_tf_filter_t * ) malloc(sizeof(struct dynlib_tf_filter_t));
  p->add = (void *) add;

//  struct dynlib_tf_filter_t *add = p->add;


  add->N = N;
  //fprintf(stderr, "libdyn: created new filter add=%d\n",  (unsigned int)  add);
  p->filter_type = LIBDYN_TF_FILTER;
  p->magic = 0xdcba;

  return p;
}

struct dynlib_filter_t *libdyn_new_shift_filter(int N)
{
  struct dynlib_filter_t *p = (struct dynlib_filter_t *) malloc(sizeof(struct dynlib_filter_t));
  struct dynlib_shift_filter_t *add = ( struct dynlib_shift_filter_t * ) malloc(sizeof(struct dynlib_shift_filter_t));
  p->add = (void *) add;
  //struct dynlib_shift_filter_t *add = p->add;

  add->N = N;
  //fprintf(stderr, "libdyn: created new shift filter add=%d\n", add);
  p->filter_type = LIBDYN_SHIFT_FILTER;

  return p;
}

struct dynlib_filter_t *libdyn_new_dejitter_filter()
{
  struct dynlib_filter_t *p = (struct dynlib_filter_t *) malloc(sizeof(struct dynlib_filter_t));
  struct dynlib_dejitter_filter_t *add = ( struct dynlib_dejitter_filter_t * ) malloc(sizeof(struct dynlib_dejitter_filter_t));
  p->add = (void *) add;
  //struct dynlib_shift_filter_t *add = p->add;

  mydebug(8) fprintf(stderr, "libdyn: created new dejitter filter\n");
  p->filter_type = LIBDYN_DEJITTER_FILTER;

  add->last_val = 0;
  add->got_first_valid_last_val = 0;
  add->steps = 0;

  return p;
}

struct dynlib_filter_t *libdyn_new_mean_filter(int N)
{
  struct dynlib_filter_t *p = (struct dynlib_filter_t *) malloc(sizeof(struct dynlib_filter_t));
  struct dynlib_tf_filter_t *add = ( struct dynlib_tf_filter_t * ) malloc(sizeof(struct dynlib_tf_filter_t));
  p->add = (void *) add;

//  struct dynlib_tf_filter_t *add = p->add;


  add->N = N;
  mydebug(8) fprintf(stderr, "libdyn: created new mean\n");
  p->filter_type = LIBDYN_TF_FILTER;

  int i;
  for (i=0; i<1000; ++i) {
    add->fill[i] = i;
  }
  p->magic = 0xdcba;

  // Load filter cf

  // (z-1) / (z*T_a): M=1
  double Qn[20];
  double Qd[20];

  for (i=1; i<= N+1; ++i) {
    Qn[i-1] = 1.0/(N+1);
    Qd[i-1] = 0;
  }
  Qd[N] = 1;

  //Qn[0] = -1/T_a; Qn[1] = 1/T_a; Qn[2]=0; Qd[0] = 0; Qd[1] = 1;
  libdyn_load_cf(p, Qn, Qd, N);

  return p;
}

struct dynlib_filter_t *libdyn_new_impulse_filter(double threshold)
{
  struct dynlib_filter_t *p = (struct dynlib_filter_t *) malloc(sizeof(struct dynlib_filter_t));
  struct dynlib_impulse_filter_t *add = ( struct dynlib_impulse_filter_t * ) malloc(sizeof(struct dynlib_impulse_filter_t));
  p->add = (void *) add;

  add->threshold = threshold;

  mydebug(8) fprintf(stderr, "libdyn: created new impulse filter\n");
  p->filter_type = LIBDYN_IMPULSE_FILTER;
  
  p->magic = 0xdcba;
  

  return p;
}

int libdyn_delete_filter(struct dynlib_filter_t *p)
{
  if (p == 0)
    return -1;
  
  if (p->add != 0) {
    free(p->add);
  }

  free(p);
  return 0;
}

// FIXME: So nicht! -> undef
#define a(i) (add->den_cf[ add->N - (i) ])
#define b(i) (add->num_cf[ add->N - (i) ])
#define z(i) (add->state_var[(i)-1])

void libdyn_print_cf(struct dynlib_filter_t *p)
{
  if (p->filter_type != LIBDYN_TF_FILTER) {
    mydebug(8) fprintf(stderr, "not a TF Filter\n");
    return;
  }

  int i;
  
  struct dynlib_tf_filter_t *add = p->add;

    fprintf(stderr, "       ");
    for (i=0; i < add->M+1; ++i) {
      fprintf(stderr, "%fz^(-%d) + ", b(i), i);
    }
    fprintf(stderr, "%fz^(-%d)\n", b(add->M+1), add->M+1);
    fprintf(stderr, "G(z) = ---------------------------------------------\n");
    fprintf(stderr, "       ");
    fprintf(stderr, "1 + ");
    for (i=1; i < add->N; ++i) {
      fprintf(stderr, "%fz^(-%d) + ", a(i), i);
    }
    fprintf(stderr, "%fz^(-%d)\n", a(add->N), add->N);
}

int libdyn_load_cf__(struct dynlib_filter_t *p, double *num, double *den, int M)
/*
 *             b0 z^(-0) + b1 z^(-1) + ... + bM z^(-M)
 * G(z) = ---------------------------------------------------
 *           1 + a1 z^(-1) + a2 z^(-2) + ... + aN z^(-N)
 *
 *
 *             b0 z^(0) + b1 z^(1) + ... + bM z^(M)
 * G(z) = ---------------------------------------------------
 *           a1 + a2 z^(1) + ... + a_(N) z^(N-1) + z^(N)
 *
 *
 * num = {b0, b1, ..., bM}
 * den = {a1, a2, ..., aN} 
 *
 * M - Grad des Zählerpoly
 */
{
  if (p->filter_type != LIBDYN_TF_FILTER) {
    mydebug(8) fprintf(stderr, "not a TF Filter\n");
    return -1;
  } else {

  struct dynlib_tf_filter_t *add = p->add;

  int i;
  add->M = M;

  mydebug(8) fprintf(stderr, "libdyn: loading filter coeff; N = %d; M = %d\n", add->N, add->M);
 

  for (i=0; i <= add->N; ++i) {
    add->num_cf[i] = (i>M) ? 0 : num[i]; // Wenn i>M, dann mit 0 aufüllen
  }
  for (i=0; i < add->N; ++i) {
    add->den_cf[i] = den[i];
  }

  mydebug(8) {

    fprintf(stderr, "       ");
    for (i=0; i < add->M+1; ++i) {
      fprintf(stderr, "%fz^(-%d) + ", b(i), i);
    }
    fprintf(stderr, "%fz^(-%d)\n", b(add->M+1), add->M+1);
    fprintf(stderr, "G(z) = ---------------------------------------------\n");
    fprintf(stderr, "       ");
    fprintf(stderr, "1 + ");
    for (i=1; i < add->N; ++i) {
      fprintf(stderr, "%fz^(-%d) + ", a(i), i);
    }
    fprintf(stderr, "%fz^(-%d)\n", a(add->N), add->N);
  }
  
  }

  return 0;
}

void libdyn_null_states(struct dynlib_filter_t *p)
{
  int i;

  mydebug(8) fprintf(stderr, "libdyn: Null states\n");
  
  if (p->filter_type == LIBDYN_TF_FILTER) {
    struct dynlib_tf_filter_t *add = p->add;

    for (i=0; i<=add->N; ++i)
      add->state_var[i] = 0;

    p->output = 0;
  } else if (p->filter_type == LIBDYN_SHIFT_FILTER) {
    struct dynlib_shift_filter_t *add = p->add;

    for (i=0; i<=add->N; ++i)
      add->state_var[i] = 0;

    p->output = 0;
  }
}

double libdyn_filter_(struct dynlib_filter_t *p, double input, int update_states) // INTERNAL
{
  // Direktform II; Script Signalverarbeitung - TU-Berlin S. 92

  double w;
  double sr = 0;
  double sf = 0;
  int i;
  struct dynlib_tf_filter_t *add = p->add;

 // fprintf(stderr, "ld: n filt\n");


  // calc output
  for (i=1; i<=add->N; ++i) {
    sr += a(i)*z(i);
    sf += b(i)*z(i);
  }
  w = input - sr;
  p->output = w*b(0) + sf;

  if (update_states) {
	// update states
	for (i=add->N; i>=2; --i) { // FIXME: DO this better by using ringbuffer
	  z(i) = z(i-1);
	}
	z(1) = w;
  }

  return p->output;
}


//#define a(i) (add->den_cf[ add->N - (i) ])
//#define b(i) (add->num_cf[ add->N - (i) ])
//#define z(i) (add->state_var[(i)-1])

double libdyn_shift_filter_(struct dynlib_filter_t *p, double input, int update_states) // INTERNAL
{
  // einfaches shift Register

  int i;

  struct dynlib_shift_filter_t *add = p->add;


  p->output = z(add->N); // letzter Wert

  if (update_states) {
	// update states
	for (i=add->N; i>=2; --i) {
	  z(i) = z(i-1);
	}
	z(1) = input;
  }

  return p->output;
}

#define isNaN(a) ( (a) == (a) ? 0 : 1 )

double libdyn_dejitter_filter_(struct dynlib_filter_t *p, double input, int update_states) // INTERNAL
{
  struct dynlib_dejitter_filter_t *add = p->add;

  int invalid = 0;
  int max_steps = 35;
  double eps = 0.04; // mehr als 8 cm pro Sample

  if (input > -9998.0) {
    if (add->got_first_valid_last_val && add->steps < max_steps) 
      if ( absf(input - add->last_val) > eps ) {
        invalid = 1;
    //    fprintf(stderr, "Sprung %d in Daten %f->%f\n", add->steps, add->last_val, input);
        if (update_states) 
	  ++add->steps;
    }
  } else {
 //   fprintf(stderr, "Fehlender Wert; %f\n", input);
    invalid = 1;
  }



  if (update_states) {
   // if (!isNaN(input))
    if (!invalid) {
      add->last_val = input;
      add->got_first_valid_last_val = 1;
      add->steps = 0;
    } else {
    //  fprintf(stderr, "Fehlender Wert - kein zst update; last_val=%f\n", add->last_val);
    }
  }
 // fprintf(stderr, "libdyn: dej: stage2\n");

//  p->output = isNaN(input) ? add->last_val : input;
  //p->output = (input < -9998.0) ? add->last_val : input;

  if (!invalid) {
    p->output = input;
  } else {
    p->output =  add->last_val;
 //   fprintf(stderr, "Fehlender Wert: using %f instead\n", p->output);
 //   fprintf(stderr, "last_val = %f\n", add->last_val);
  }


  return (p->output);
}


double libdyn_impulse_filter_(struct dynlib_filter_t *p, double input, int update_states) // INTERNAL
{
  // detection of Impulses

  int i;

  struct dynlib_impulse_filter_t *add = p->add;


   // 
  if ( absf(input - add->last_val) > add->threshold ) {
    p->output = 1; // Impulse detected
  } else {
    p->output = 0; // 
  }

  if (update_states) {
	// update states
     add->last_val = input;
  }

  return p->output;
}


double libdyn_filter(struct dynlib_filter_t *p, double input)
{
  switch (p->filter_type) {
    case LIBDYN_TF_FILTER : 
	return libdyn_filter_(p,input,1);
	break;
    case LIBDYN_SHIFT_FILTER : 
	return libdyn_shift_filter_(p,input,1);
	break;
    case LIBDYN_DEJITTER_FILTER : 
	return libdyn_dejitter_filter_(p,input,1);
	break;
    case LIBDYN_IMPULSE_FILTER : 
	return libdyn_impulse_filter_(p,input,1);
	break;  
  }
}

double libdyn_newout(struct dynlib_filter_t *p, double input)
{
  switch (p->filter_type) {
    case LIBDYN_TF_FILTER : 
	return libdyn_filter_(p,input,0);
	break;
    case LIBDYN_SHIFT_FILTER : 
	return libdyn_shift_filter_(p,input,0);
	break;
    case LIBDYN_DEJITTER_FILTER : 
	return libdyn_dejitter_filter_(p,input,0);
	break;
    case LIBDYN_IMPULSE_FILTER : 
	return libdyn_impulse_filter_(p,input,0);
	break;
  }
}

double libdyn_lastout(struct dynlib_filter_t *p)
{
  return p->output;
}

double libdyn_out(struct dynlib_filter_t *p, double input, int update_states)
{
  switch (p->filter_type) {
    case LIBDYN_TF_FILTER : 
	return libdyn_filter_(p,input,update_states);
	break;
    case LIBDYN_SHIFT_FILTER : 
	return libdyn_shift_filter_(p,input,update_states);
	break;
    case LIBDYN_DEJITTER_FILTER : 
	return libdyn_dejitter_filter_(p,input,update_states);
	break;
    case LIBDYN_IMPULSE_FILTER : 
	return libdyn_impulse_filter_(p,input,update_states);
	break;
  }
}







/*
 * Helper functions for setting up schematics via an scilab interface
 * Blocks data as well as connections are encoded via irpar.h
 *
 */


/*
 *  Create filters from parameters loaded with irpar
 *  FIXME: Move to other file -> schematic_setup.c
 */

/* FIXME: Testen! */
/*
#include <dlfcn.h>

void * libdyn_get_comp_fn(char *name)
{
    void *handle;


    int (*comp_func)(int flag, struct dynlib_outlist_t *block);  // Computational function
    char *error;

    handle = dlopen (NULL, RTLD_LAZY);
    if (!handle) {
        fputs (dlerror(), stderr);
        exit(1);
    }

    comp_func = dlsym(handle, "compu_func_bilinearint");
    if ((error = dlerror()) != NULL)  {
        fputs(error, stderr);
        
    }

    dlclose(handle);

    return comp_func;
}
*/

#include "irpar.h"
#include "libdyn_blocks.h"

double constant_nan = 0.0;

struct dynlib_filter_t *libdyn_new_tf_filter_irpar(int *ipar, double *rpar, int id)
{
  struct irpar_tf_t ret;
  struct dynlib_filter_t *filter;
  
 int err = irpar_get_tf(&ret, ipar, rpar, id);

  if (err == -1)
    return 0;
  
 // fprintf(stderr, "new irpar filter: %d, %d\n", ret.degnum, ret.degden);
  
  filter = libdyn_new_filter(ret.degden);
  libdyn_load_cf(filter, ret.num, ret.den, ret.degnum);
  libdyn_null_states(filter);
  
  return filter;
}


/* Values as in libdyn.sci */
#define LIBDYN_BLOCK_ID_SUM_PN 10
#define LIBDYN_BLOCK_ID_SUM_PP 11
#define LIBDYN_BLOCK_ID_SUM 12
#define LIBDYN_BLOCK_ID_GAIN 20
#define LIBDYN_BLOCK_ID_TF 30
#define LIBDYN_BLOCK_ID_CONST 40
#define LIBDYN_BLOCK_ID_SAT 50
#define LIBDYN_BLOCK_ID_SWITCH 60
#define LIBDYN_BLOCK_ID_MUL 70
#define LIBDYN_BLOCK_ID_FNGEN 80
#define LIBDYN_BLOCK_ID_SER2PAR 90
#define LIBDYN_BLOCK_ID_PLAY 100
#define LIBDYN_BLOCK_ID_2TO1_EVENT_SWITCH 110
#define LIBDYN_BLOCK_ID_LOOKUP 120
#define LIBDYN_BLOCK_ID_FILEDUMP 130
#define LIBDYN_BLOCK_ID_COMPARE 140
#define LIBDYN_BLOCK_ID_DELAY 150
#define LIBDYN_BLOCK_ID_FLIPFLOP 160
#define LIBDYN_BLOCK_ID_PRINTF 170

#define LIBDYN_BLOCK_INTERFACE 4000
#define LIBDYN_BLOCK_ID_GENERIC 5000

//#define LIBDYN_BLOCK_ID_SCOPE 10001

// Create Block instance from irpar
struct dynlib_block_t * irpar_get_libdynblock(struct dynlib_simulation_t *sim, int *ipar, double *rpar, 
					      int id, struct dynlib_block_t **blocklist)
{
  // searches for the block with irpar id id
  // creates it and
  // writes the newly created block pointer into blocklist
  
  int i, err;
  struct irpar_header_element_t ret_;
 
#ifdef DEBUG
  fprintf(stderr, "irpar_get_libdynblock: Trying to get block with id %d\n", id);
#endif
  
  irpar_get_element_by_id(&ret_, ipar, rpar, id);  // ret_.setindex is a unique index for each id starting from 0
  
  if (ret_.typ != IRPAR_LIBDYN_BLOCK) {
    fprintf(stderr, "irpar id %d is not of type IRPAR_LIBDYN_BLOCK!\n", id);
    
    return NULL;
  }
  
  // Make sure the block with irpar id id is not already there
  if (blocklist[ret_.setindex] != NULL) { // Block is already available; irpar should give a unique index "setindex" for each set
    mydebug(7) fprintf(stderr, "Block already created\n");
    return blocklist[ret_.setindex];
  }
  
  
  /* 
   * decode ipar structures 
   */
  
  int btype = ret_.ipar_ptr[0]; // 
  int blockid = ret_.ipar_ptr[1]; // unique identifier, refered in connection list
  // FIXME: Hier auch par anzahl behandeln
  int Nbipar = ret_.ipar_ptr[2];
  int Nbrpar = ret_.ipar_ptr[3];
  
  int eventlist_len = ret_.ipar_ptr[4];
  int *eventlist = &ret_.ipar_ptr[5];
  
  int bipar_start = 5 + eventlist_len;
  
  int *bipar = &(ret_.ipar_ptr[bipar_start]); // Blocks personal data
  double *brpar = &(ret_.rpar_ptr[0]);

  /*
   * Create new Block
   */ 
  
#ifdef DEBUG
  fprintf(stderr, "  --> OK. New Block: btype=%d, blockid=%d, eventlistlen=%d\nTring to find the computational function...\n", btype, blockid, eventlist_len);
#endif
  
  
  struct dynlib_block_t *block = 0;
  
  switch(btype) {
    case LIBDYN_BLOCK_ID_GENERIC : // 
      {
	int (*comp_func)(int flag, struct dynlib_outlist_t *block);
//	comp_func = libdyn_get_comp_fn("bla");
	(*comp_func)(1, 0);
      }
      break;
    case LIBDYN_BLOCK_ID_SUM :
      { 
	block = new_sum_block_(sim, &brpar[0]); 
      }
      break;      
    case LIBDYN_BLOCK_ID_CONST :
      { 
	block = new_const_block_(sim, &brpar[0]); 
      }
      break;
    case LIBDYN_BLOCK_ID_GAIN :
      { 
	block = new_gain_block_(sim, &brpar[0]); 
      }
      break;
    case LIBDYN_BLOCK_ID_SAT :
      { 
	//block = new_sat_block_(sim, &brpar[0]); 
	block = libdyn_new_block(sim, &compu_func_sat, &bipar[0], &brpar[0], 0,  0);
      }
      break;
    case LIBDYN_BLOCK_ID_TF :
      { int degnum = bipar[0];
        int degden = bipar[1];
        block = new_zTF_block_(sim, degnum, degden, &(brpar[0]), &(brpar[degnum+1]) ); }
      break;
    case LIBDYN_BLOCK_ID_SWITCH :
      { 
	block = new_switch_block_(sim); 
      }
      break;
    case LIBDYN_BLOCK_ID_MUL :
      { 
	block = new_mul_block_(sim, &bipar[0]); 
      }
      break;      
    case LIBDYN_BLOCK_ID_FNGEN :
      { 
	block = new_fn_gen_block_(sim, &bipar[0]); 
      }
      break;      
    case LIBDYN_BLOCK_ID_SER2PAR :
      { 
	block = new_ser2par_block(sim, &bipar[0]); 
      }
      break; 
    case LIBDYN_BLOCK_ID_PLAY :
      { 
	block = new_play_block(sim, &bipar[0], &brpar[0]);
      }
      break; 
    case LIBDYN_BLOCK_ID_2TO1_EVENT_SWITCH :
      { 
	block = new_2to1_event_switch_block(sim, &bipar[0]);
      }
      break; 
    case LIBDYN_BLOCK_ID_LOOKUP :
      { 
       block = new_lookup_block(sim, &bipar[0], &brpar[0]);
      }
      break;
    case LIBDYN_BLOCK_ID_FILEDUMP :
      { 
       block = new_filedump_block(sim, &bipar[0], &brpar[0]);
      }
      break;
    case LIBDYN_BLOCK_ID_COMPARE :
      {       
       block = libdyn_new_block(sim, &compu_func_compare, &bipar[0], &brpar[0], 0,  0);
      }
      break;
//     case LIBDYN_BLOCK_ID_DELAY :
//       {       
//        block = libdyn_new_block(sim, &compu_func_delay, &bipar[0], &brpar[0], 0,  0);
//       }
//       break;
    case LIBDYN_BLOCK_ID_FLIPFLOP :
      {       
       block = libdyn_new_block(sim, &compu_func_flipflop, &bipar[0], &brpar[0], 0,  0);
      }
      break;
    case LIBDYN_BLOCK_ID_PRINTF :
      {       
       block = libdyn_new_block(sim, &compu_func_printf, &bipar[0], &brpar[0], 0,  0);
      }
      break;
      
      
      
      
      
      
    case LIBDYN_BLOCK_INTERFACE :
      { 
	block = new_interface_block(sim, &bipar[0]); 
      }
      break;      
  }
  
  

  if (block == 0) { // kein vordefinierter Block -> in blocklisten nachschauen
    struct lindyn_comp_func_list_ele_t *cfn_ele;

    
    cfn_ele = libdyn_compfnlist_find_blockid(sim->global_comp_func_list,  btype);

    if (cfn_ele != 0) {
      void *comp_fn = cfn_ele->comp_fn;
      int type = cfn_ele->comp_fn_type;
      
      block = libdyn_new_block(sim, comp_fn, bipar, brpar, 0,  0);
    } else {


      cfn_ele = libdyn_compfnlist_find_blockid(sim->private_comp_func_list,  btype);
      
      if (cfn_ele != 0) {
        void *comp_fn = cfn_ele->comp_fn;
        int type = cfn_ele->comp_fn_type;
      
        block = libdyn_new_block(sim, comp_fn, bipar, brpar, 0,  0);
      } else {
	// Absolutely nothing was found
	fprintf(stderr, "libdyn: Could not find any block for this block type! btype = %d.\n", btype);
      }
    }
  }
  
  if (block == 0) {
    fprintf(stderr, "libdyn: Could not create block with blockid = %d!\n", btype);
    return 0;
  }
  
  block->irpar_config_id = id;
  
  /*
   *  Register events
   */
  
  mydebug(7) fprintf(stderr, "register block to events: ");
  for (i=0; i<eventlist_len; ++i) {
    mydebug(7) fprintf(stderr, "Register event %d, ", eventlist[i]);
    err = libdyn_new_event_notifier(sim, block, eventlist[i]); // register event for this block

    if (err == 1) {
       fprintf(stderr, "libdyn: error assigning event!\n");
      return 0;
    }
  }
  mydebug(7) fprintf(stderr, "\n");
  
  
  
  
  blocklist[ret_.setindex] = block;
  
#ifdef DEBUG
   libdyn_block_dumpinfo(block);
#endif
  
  return block;
}

// Traverse block connection list based on irpar structure and create blocks and connect them
// irpar encoding is done in libdyn.sci

struct libdyn_conn_list_element_t {
    int dst_type;
    int dst_blockid;
    int dst_blockparid;
    int dst_port;
    struct dynlib_block_t *dst_block;
    
    int src_type;
    int src_blockid;
    int src_blockparid;
    int src_port;
    struct dynlib_block_t *src_block;
};

struct libdyn_conn_list_element_t * libdyn_conn_list_new(int n)
{
  struct libdyn_conn_list_element_t *p = (struct libdyn_conn_list_element_t *) malloc( sizeof(struct libdyn_conn_list_element_t) * n);
  
  /*
   * initialise pointers	
   */
  
  int i;
  for (i = 0; i<n; ++i) {
    p[i].dst_block = 0;
    p[i].src_block = 0;
  }
  
  return p;
}

struct libdyn_conn_list_element_t * libdyn_conn_list_del(struct libdyn_conn_list_element_t * cl)
{
  free(cl);
}

int irpar_get_libdynconnlist(struct dynlib_simulation_t *sim, int *ipar, double *rpar, 
			     int id, struct libdyn_io_config_t *iocfg)
/*
 * Build simulation based on blocks and connections described by irpar struct
 * which contains a list of block + parameters
 * and a list for connections beween blocks and external ports
 *
 * iocfg.inptr is an array of pointers for each inport to input values
 * iocfg.outptr is an array of pointers for each outport that will be set to the 
 *              simulation outputs according to the connection list
 *
 *
 */

{
  struct irpar_header_element_t ret_;
  
  int i, err;

  int maxirparsets = irpar_get_nele(ipar, rpar);
  
  
  //
  // Setup clocks
  //

  int event1 = libdyn_new_event(sim); // standard clock
  libdyn_clock_event_generator_addclock(sim,1); //event 0

  //
  // Initially set all outputs to a defined value - for sure if there won't be a to this output connected block
  // Later check wheter there are unconnected outports and if that's the case give a warning
  //

  for (i = 0; i < iocfg->outports; ++i) {
    iocfg->outptr[i] = &constant_nan;
  }
  
  //
  // Read out irpar box
  //
  
  irpar_get_element_by_id(&ret_, ipar, rpar, id);
  
  if (ret_.typ != IRPAR_LIBDYN_CONNLIST)
    return -1;
  
  int type = ret_.ipar_ptr[0]; // Hmm
  int NrConnectionListEntries = ret_.ipar_ptr[1]; // Nr of list elements
  int ConnectionLIstElementSize = ret_.ipar_ptr[2]; // Size of list element

  int listofs = ret_.ipar_ptr[3]; // BEGIN OF connectionlist in irpar array

  mydebug(7) fprintf(stderr, "clist header: type=%d, entries=%d, listelesize=%d, listofs=%d\n", type, NrConnectionListEntries, ConnectionLIstElementSize, listofs);

  /*
   * Alloc mem for parsed copy
   */
  
  struct libdyn_conn_list_element_t *cl = libdyn_conn_list_new(NrConnectionListEntries);
  
  // DEF Blocklist arrindex->blockptr
  struct dynlib_block_t ** blocklist = (struct dynlib_block_t **) malloc( maxirparsets * sizeof(struct dynlib_block_t *) );
  for (i = 0; i < maxirparsets; ++i)
    blocklist[i] = 0;

  /*
   * Parse. Go through connection list and create and connect blocks as they appear. Also keep track of the 
   * alredy created block, so they are not created twice.
   */
  
  
#ifdef DEBUG
    fprintf(stderr, "Number of new connections: %d\n", NrConnectionListEntries);
#endif
  
  for (i = 0; i < NrConnectionListEntries; ++i) {
    int src_type = ret_.ipar_ptr[listofs + i * ConnectionLIstElementSize + 0];
    int src_blockid = ret_.ipar_ptr[listofs + i * ConnectionLIstElementSize + 1];
    int src_blockparid = ret_.ipar_ptr[listofs + i * ConnectionLIstElementSize + 2];
    int src_port = ret_.ipar_ptr[listofs + i * ConnectionLIstElementSize + 3];

    int dst_type = ret_.ipar_ptr[listofs + i * ConnectionLIstElementSize + 4]; // 0: normal block; 1: IN/OUT   2: the destination describes a totally unconnected block, to be set-up
    int dst_blockid = ret_.ipar_ptr[listofs + i * ConnectionLIstElementSize + 5]; // blocks unique id
    int dst_blockparid = ret_.ipar_ptr[listofs + i * ConnectionLIstElementSize + 6]; // blocks parameter id
    int dst_port = ret_.ipar_ptr[listofs + i * ConnectionLIstElementSize + 7]; // which port of this dest block
    
    
    cl[i].dst_type = dst_type;
    cl[i].dst_blockid = dst_blockid;
    cl[i].dst_blockparid = dst_blockparid;
    cl[i].dst_port = dst_port;

    cl[i].src_type = src_type;
    cl[i].src_blockid = src_blockid;
    cl[i].src_blockparid = src_blockparid;
    cl[i].src_port = src_port;

#ifdef DEBUG
    fprintf(stderr, "new connection: src: type=%d, blockid=%d, blockparid=%d, port=%d  -->", src_type, src_blockid, src_blockparid, src_port);
    fprintf(stderr, "  dst: type=%d, blockid=%d, blockparid=%d, port=%d\n", dst_type, dst_blockid, dst_blockparid, dst_port);
#endif
    
    /*
     * Create Blocks
     */
    // Create specioal totally unconnected blocks -- specially signed by dst_type == 2
    if (cl[i].dst_block == 0 && cl[i].dst_type == 2) {

      // the following fn creates a block if it is not already in the "blocklist"
      struct dynlib_block_t * blk = irpar_get_libdynblock(sim, ipar, rpar, dst_blockid, blocklist);
      
#ifdef DEBUG
	fprintf(stderr, "libdyn: Created special block without any in/outputs. irparid = %d\n", src_blockid);      
#endif
      
      if (blk == 0) {
	fprintf(stderr, "Error: Could not create block with irparid = %d\n", src_blockid);
	
	goto error;
      }
      
      cl[i].dst_block = blk; 
      
      goto GoOnInConnectionList; // go on in the loop for the connection list
    }
    
    if (cl[i].src_block == 0 && cl[i].src_type == 0) { // If this block wasn't already created

      // the following fn creates a block if it is not already in the "blocklist"
      struct dynlib_block_t * blk = irpar_get_libdynblock(sim, ipar, rpar, src_blockid, blocklist);
      
      if (blk == 0) {
	fprintf(stderr, "Error: Could not create block with irparid = %d\n", src_blockid);
	
	goto error;
      }
      
      cl[i].src_block = blk; 
    }

    if (cl[i].dst_block == 0 && cl[i].dst_type == 0) { // If this block wasn't already created

      // the following fn creates a block if it is not already in the "blocklist"
      struct dynlib_block_t * blk = irpar_get_libdynblock(sim, ipar, rpar, dst_blockid, blocklist);
      
      if (blk == 0) {
	fprintf(stderr, "Error: Could not create block with irparid = %d\n", src_blockid);
	
	goto error;
      }
      
      cl[i].dst_block = blk; 
    }

    
    /*
     * Connect blocks
     */
    
    double **inports = iocfg->inptr;
    double **outports = iocfg->outptr; // Contains for example scicos outptr
  //  fprintf(stderr, "outports list = %x\n", outports);
    
    if (cl[i].src_type == 0 && cl[i].dst_type == 0) {// connect two blocks
      err = libdyn_block_connect(cl[i].src_block, cl[i].src_port, cl[i].dst_block, cl[i].dst_port);
      
      if (err != 1) {
	fprintf(stderr, "Error connecting block - wrong types or invalid ports err=%d\n", err);
	
	goto error;
      }
    }
    
    // FIXME: CHeck Ports types
    if (cl[i].src_type == 1 && cl[i].dst_type == 0) {// connect external in to a block
      if (cl[i].src_port < iocfg->inports) {
	int portsize = iocfg->insizes[cl[i].src_port];
	
	mydebug(7) fprintf(stderr, "external inp to inport =%d ; addr =%x; portsize = %d\n"  , cl[i].src_port,  (unsigned int)  inports[cl[i].src_port], portsize);
        if (err = libdyn_block_connect_external(cl[i].dst_block, cl[i].dst_port, inports[cl[i].src_port], portsize) != 0) {
   	  fprintf(stderr, "Error connecting external input for block id %d - destination port not defined or wrong port sizes! Code = %d\n", cl[i].dst_block->numID, err);
	  goto error;
	}
      } else {
	fprintf(stderr, "Error connecting external input for block id %d  - not enough external inports defined\n", cl[i].dst_block->numID);
	
	goto error;
      }
      
    }

    if (cl[i].src_type == 0 && cl[i].dst_type == 1) { // connect block to external out
      if (cl[i].dst_port < iocfg->outports) {
	
	if (iocfg->outptr[ cl[i].dst_port ] != &constant_nan) {
	  fprintf(stderr, "Error connecting output: already connected\n");

	  goto error;
	}
	
	if (iocfg->outsizes[ cl[i].dst_port ] != cl[i].src_block->outlist[ cl[i].src_port ].len ) {
	  fprintf(stderr, "Libdyn Error: Invalid port when connecting output #%d of the simulation\n", cl[i].dst_port);
	  goto error;	  
	}

	// FIXME: The same for the datatype please
//         if (iocfg->outsizes[ cl[i].dst_port ] != cl[i].src_block->outlist[ cl[i].src_port ].len ) {
// 	  fprintf(stderr, "Libdyn Error: Invalid port when connecting an output of the simulation\n");
// 	  goto error;	  
// 	}

	double *output = libdyn_get_output_ptr( cl[i].src_block, cl[i].src_port  );
	if (output == 0) {
	  fprintf(stderr, "Error connecting output: sourceport not defined; port = %d, max=%d\n", cl[i].src_port, cl[i].src_block->Nout);
	  goto error;
	}
	mydebug(7) fprintf(stderr, "external out to outport =%d\n"  , cl[i].dst_port);
	//fprintf(stderr, "lahfl %x\n", iocfg->outptr[ cl[i].dst_port ]);
        iocfg->outptr[ cl[i].dst_port ] = output; // cl[i].dst_port
      } else {
	fprintf(stderr, "Error connecting external out (Tryed to connect outport %d but max number is %d) \n",  cl[i].dst_port, iocfg->outports);
	
	goto error;
      }
    }

    if (cl[i].src_type == 1 && cl[i].dst_type == 1) { // connect ex in to ex out: not possible at the moment
      fprintf(stderr, "libdyn: EXPERIMENTAL: Try to connect simulation output directly to a simulation input. This is not tested by now.\n");

      int in_portsize = iocfg->insizes[cl[i].src_port];
      int out_portsize = iocfg->outsizes[cl[i].dst_port];

      if (in_portsize != out_portsize) {
	fprintf(stderr, "Error while connecting external out %d because of incorrect port size\n",  cl[i].dst_port);	
	goto error;
      }
//       int in_porttype = iocfg->intypes[cl[i].src_port];
//       int out_porttype = iocfg->outtypes[cl[i].dst_port];
// 
//       if (in_porttype != out_porttype) {
// 	fprintf(stderr, "Error while connecting external out %d because of incorrect port type\n",  cl[i].dst_port);	
// 	goto error;
//       }
      
      // 
      iocfg->outptr[ cl[i].dst_port ] = iocfg->inptr[ cl[i].src_port ];
      
//       goto error;
    }

   
    mydebug(0) fprintf(stderr, "sup exec list head: %p\n", sim->execution_sup_list_head);

    
  GoOnInConnectionList:
    ;
    
  }
  
  //
  // 
  // check wheter there are unconnected outports 
  //

  for (i = 0; i < iocfg->outports; ++i) {
    if (iocfg->outptr[i] == &constant_nan) {
      if (iocfg->outsizes[i] > 1) {
	fprintf(stderr, "ERROR: outport %d is not connected and its size is greater than one --\nso there is also no default output provieded\n", i);
	goto error;
      } else {
	fprintf(stderr, "WARNING: outport %d is not connected, but since it is of size one NaN will be put out\n", i);
      }
    }
  }


  free(blocklist);
  libdyn_conn_list_del(cl);
  mydebug(7) fprintf(stderr, "connections are set-up\n");
  return 0;
  
 error:
  
  free(blocklist);
  libdyn_conn_list_del(cl);

  return -1;
}

int libdyn_irpar_setup(int *ipar, double *rpar, int boxid, 
		       struct dynlib_simulation_t **sim,
		       struct libdyn_io_config_t *iocfg)
{
/*  
     Fills in the simulation "sim"
     If an error occures -1 is returned and "sim" is destroyed
  */
  int err;

  mydebug(7) fprintf(stderr, "getting parameters\n");
  // Get irpar container/box
  struct irpar_header_element_t ret;
  err = irpar_get_element_by_id(&ret, ipar, rpar, boxid);
  if (err == 0) {
    fprintf(stderr, "Error getting irpar box id %d\n", boxid);
    return -1;
  }

  //
  // Create a new simulation
  // 
  *sim = libdyn_new_simulation();

//   printf("Plugin: Target is %s\n");

  //
  // Load Plugins if enabled
  // 

  
#ifdef __ORTD_PLUGINS_ENABLED
  fprintf(stderr, "Plugins are enabled\n");
  
  // Load modules, if available
  struct irpar_rvec_t *enc_libpath;
  err = irpar_get_rvec(&enc_libpath, ret.ipar_ptr, ret.rpar_ptr, 20);
  if (err == -1) {
//     fprintf(stderr, "No Plugin shall be loaded\n");
  } else ;
  {
    char *plugin_fname = "./ortd_plugin.so";
/*    int len = enc_libpath->n;
    
    irpar_getstr(&plugin_fname, enc_libpath->v, 0, len);*/
    
    fprintf(stderr, "Loading plugin: %s\n", plugin_fname);

    ortd_load_plugin(*sim, plugin_fname);
    
    
//     free(plugin_fname);
  }
#else
  fprintf(stderr, "Plugins are disabled in RTAI-compatible mode\n");
#endif

  //
  // Read all blocks and connect them
  // 
  
  err = irpar_get_libdynconnlist(*sim, ret.ipar_ptr, ret.rpar_ptr, 100, iocfg);
  if (err == -1) {
    fprintf(stderr, "Error in irpar_get_libdynconnlist\n");

    goto error; 
  }
    
  //libdyn_block_dumpinfo((*sim)->allblocks_list_head->allblocks_list_next); //->allblocks_list_next

  //
  // Check for inputs forgotten to connect
  // 

  err = libdyn_simulation_checkinputs(*sim);
  if (err < -0) {
    fprintf(stderr, "Error checking input ports of all blocks\n");
    
    goto error; 
  }
  
  
  //
  // Build exec list
  //

  err = libdyn_setup_executionlist(*sim);
  if (err == -1) {
    fprintf(stderr, "Error in libdyn_setup_executionlist\n");
    
    goto error; 
  }
 
/*  //
  // Dump all Blocks
  //
 
  libdyn_dump_all_blocks(*sim); */
  
  return 0;

 error:
 
  // Undo everything
  libdyn_del_simulation(*sim);

  return -1;
}





/*
 * 
 * 
 * Events stuff indipendend of all above (maybe move to another file)
 * 
 * 
 */

struct libdyn_event_div_t *libdyn_new_event_div(int ofs, int fak)
{
  struct libdyn_event_div_t *sim = (struct libdyn_event_div_t *) malloc(sizeof(struct libdyn_event_div_t));

  sim->fak = fak;
  sim->ofs = ofs;
  sim->counter = 0;
  
  return sim;
}
  
void libdyn_del_event_div(struct libdyn_event_div_t *div)
{
  free(div);
}

int libdyn_event_div(struct libdyn_event_div_t *div, int update_states)
{
  
  if (update_states == 1)
    div->counter++; // FIXME: Überlauf verhindern
  
  if (div->counter >= div->ofs) {
//    fprintf(stderr, "cnt %d\n", div->counter);

    int tmp = div->counter - div->ofs;
    if ((tmp % div->fak) == 0) {
      //fprintf(stderr, "event at %d!\n", div->counter);
      return 1;
    }
      
  }
  
  return 0;
}


