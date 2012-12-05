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
 * libdyn.h - Library for simple realtime controller implementations
 *
 *
 * Author: Christian Klauer 2009-2012
 *
 *
 *
 *
 */

#ifndef _LIBDYN_H
#define _LIBDYN_H 1





/*
    Uncomment this will enable some additional checks, but slow down the performance a bit
    Use, if you are using untested blocks
*/
//#define LD_USE_ONLINE_ASSERTIONS



#include "block_lookup.h"


// Max connections to an summation or multiplication point
#define INPUT_MAX_CONNECTIONS 10
#define LIBDYN_MAX_BLOCK_EVENTS 32
#define LIBDYN_MAX_EVENTS 32


/*
 * Datatype handling
*/

// Different datatypes  DATATYPE_UNCONFIGURED means Blocks PORt is not configured
#define __libdyn_create_datatype(id, size) ( (id) | (size) << 5 ) 

#define DATATYPE_UNCONFIGURED 0
#define DATATYPE_FLOAT (1 | (sizeof(double) << 5))
#define DATATYPE_SHORTFLOAT 4
#define DATATYPE_INT 2
#define DATATYPE_BOOLEAN 3
#define DATATYPE_EVENT 5


// Different types inputs UNUSED 
#define INTYPE_UNDEF -1
#define INTYPE_DIRECT 1
#define INTYPE_EXTERN 2
#define INTYPE_GAIN 3
#define INTYPE_SUMMATION 4
#define INTYPE_MULTIPLICATION 5

// Types for output ports
#define OUTTYPE_UNDEF -1
#define OUTTYPE_DIRECT 1


// Types of Blocks
#define BLOCKTYPE_DYNAMIC 1
#define BLOCKTYPE_STATIC 2

// Comp function flags
#define COMPF_FLAG_CONFIGURE 1
#define COMPF_FLAG_UPDATESTATES 2
#define COMPF_FLAG_CALCOUTPUTS 3
#define COMPF_FLAG_DESTUCTOR 4
#define COMPF_FLAG_NOTIFY 5
#define COMPF_FLAG_SETEXTRAPAR 6
#define COMPF_FLAG_INIT 7 // follows CONFIGURE
#define COMPF_FLAG_RELOAD_IRPAR 8
#define COMPF_FLAG_RESETSTATES 9 // NEW 25.6.11
#define COMPF_FLAG_PREPARERESET 10 // NEW 27.2.12
#define COMPF_FLAG_HIGHERLEVELRESET 14 // NEW as of 26.3.2012 not implemented for now
#define COMPF_FLAG_PREINIT 11
#define COMPF_FLAG_PREINITUNDO 12
#define COMPF_FLAG_POSTINIT 13 // NEW as of 18.4.2012
#define COMPF_FLAG_NOTHING 1000  // computational functon should do nothing
#define COMPF_FLAG_PRINTINFO 100


// Macros for accessing block data structures // FIXME: hier nullpointerabfrage

#ifdef LD_USE_ONLINE_ASSERTIONS 
  #define libdyn_get_output_ptr(block, out) ( ((block)->Nout <= out) ? 0 : (block)->outlist[(out)].data  )
  #define libdyn_get_input_ptr(block, in) (  ((block)->Nin <= in) ? 0 : (block)->inlist[(in)].data )
#else
  #define libdyn_get_output_ptr(block, out) ( (block)->outlist[(out)].data  )
  #define libdyn_get_input_ptr(block, in)   ( (block)->inlist[(in)].data )
#endif


#define libdyn_get_ipar_ptr(block) ((block)->ipar)
#define libdyn_get_rpar_ptr(block) ((block)->rpar)
#define libdyn_get_opar_ptr(block) ((block)->opar)
#define libdyn_get_work_ptr(block) ((block)->work)
#define libdyn_set_work_ptr(block, work_) ((block)->work = (work_))
#define libdyn_get_Ninports(block) ((block)->Nin)
#define libdyn_get_Noutports(block) ((block)->Nout)
#define libdyn_get_inportsize(block, i) ((block)->inlist[(i)].len)
#define libdyn_get_outportsize(block, i) ((block)->outlist[(i)].len)


#define libdyn_get_extraparam(block) ((block)->extraparam)
#define libdyn_set_extraparam(block, par) ((block)->extraparam = (par))



//
// macros for using the list of connected ports to one output
//

#define __libdyn_macro_junctionlist_next {  \
  struct dynlib_block_t *next; \
  int nexti; \
 \
  next = current->inlist[currenti].next_juncture_block[0]; \
  nexti = current->inlist[currenti].next_juncture_input[0]; \
 \
  current = next; \
  currenti = nexti; \
}

#define __libdyn_macro_junctionlist_head  \
      struct dynlib_block_t *current = cbl->inlist[cbli].next_juncture_block[0]; \
      int currenti = cbl->inlist[cbli].next_juncture_input[0];

      
      
//
// One element of the input port list
//
struct dynlib_inlist_t {
  int len, datatype; // Size of this input, Datatype

  int intype; // 0 twoside connecttion; 1 summation point; ... (only 0 for now)

  int nrInput; // input id

  //  intype == 0

  //
  // These two variables give a hint to the block and port to which this input is connected
  //
  struct dynlib_block_t *pre[INPUT_MAX_CONNECTIONS]; // Connected Block // Im Falle von intype == 1 müsste hier eine Liste stehen, die noch die gewichtungen der Sumanden beinhaltet; May be misstreated for a pointer to external input data in case of intype = INTYPE_EXTERN
  int outNr[INPUT_MAX_CONNECTIONS]; // Nr of ouput of connected Block for each element of "pre"

  // or user allocated memory, intype == 1

  void *data;	// Data (a shorter way to block->inlist[i].pre[0]->outlist[n].data)

  // List of connections that are also connected to this
  // junction point for every subinput (in case of internal
  // addition etc., non twoside connect) respectively.
  struct dynlib_block_t *next_juncture_block[INPUT_MAX_CONNECTIONS];
  int next_juncture_input[INPUT_MAX_CONNECTIONS];

  //
  // Used by execution list generation algorithm
  //
  int input_available[INPUT_MAX_CONNECTIONS]; // 1 if a block is on executionlist that calculates this input port
};

struct dynlib_outlist_t {  
  int len, datatype; // Length of this output, Datatype
  void *data;	// Data
  int datasize; // Length of data (Bytes)

  int outtype; // 0 es ist noch ein Block angeschlossen; 1- nicht
  int dinput_dependence; // // 1 - output dependes directly (dfeedthrough) on input

  // head of junction connection list
  struct dynlib_block_t *post; // Connected Block
  int post_input; // input # of connected block
  int post_subinput; // Subinput ...
};

struct dynlib_event_t { // Event Struktur
  int Nevents;
  int event_active; // Jedes Bit ein event: Wenn 1 dann signal in diesem Schritt
  //
  struct dynlib_event_t *more_events; // Reserved
};

struct dynlib_portlist_t { // FIXME: unused for now // usage for superblocks content
  struct dynlib_block_t *block;
  int port;

  struct dynlib_portlist_t *next;
};

struct dynlib_simIO_t { // FIXME: unused for now // usage for superblocks content
  struct dynlib_portlist_t *inlist_head; // List of simulation input connections
  struct dynlib_portlist_t *inlist_tail;
  struct dynlib_portlist_t *outlist_head; // List of simulation output connections
  struct dynlib_portlist_t *outlist_tail;
};

struct dynlib_simulation_t {
  // Simulation states
  int blocks_initialised; // Are the blocks initialised?
  
  struct dynlib_event_t events; // events, die in der Simulation verwendet werden.

  //
  // List of blocks
  // in this place the heads and the ends of all lists are defined
  //
  
  // execution lists
  struct dynlib_block_t *execution_list_head; // d feedthrough execution list.
  struct dynlib_block_t *execution_list_tail;

  struct dynlib_block_t *execution_sup_list_head; // state update execution range
  struct dynlib_block_t *execution_sup_list_tail;

  // allblocks list
  struct dynlib_block_t *allblocks_list_head;
  struct dynlib_block_t *allblocks_list_tail;

  // Synchronisation callback function
  struct {
    void *userdat; // users data
    int (*sync_func)( struct dynlib_simulation_t *sim );  // Callback function. the returned value is stored within sync_callback_state. 0 is usual, 1 means pause simulation
    int sync_callback_state;
  } sync_callback;
  
  
  // simulation I/O
  struct dynlib_simIO_t simIO; // FIXME: unused for now // usage for superblocks content

  int numID_counter; // Zähler für Block IDs


  int clock_multiplier[LIBDYN_MAX_EVENTS]; // If an element is zero its curresponding (to array indice) event will not be activated automatically
  int num_cm; // number of elements obove

  double t;
  int stepcounter; // count simulation steps
  
  struct lindyn_comp_func_list_head_t *private_comp_func_list;
  struct lindyn_comp_func_list_head_t *global_comp_func_list;
  
  // optinal: Master
  // Ist eine cpp-Klasse
  void *master;
};

struct block_config_ret_t {
  int block_type;
  int d_feedthrough;

  char identification[10];
};

struct dynlib_block_t {
  struct dynlib_simulation_t *sim; // The simulation which this block belongs to
  
  int block_type; // 0 normal dynamic; 1 static function
  int d_feedthrough; // Direct Feedthrough?

  //char identification[10];
  char *identification;
  int numID;
  int own_outcache; // output cache memory should not be allocated
  int own_outcache_toconfigure;

  int *ipar;
  double *rpar;
  void *opar;

  int Nout;
  struct dynlib_outlist_t *outlist; // Pointer auf ausgangswerte cache Liste (1. Eintrag)
  void *outdata; // Datendie für die Ausgänge angelegt werden - unused if own_outcache = 1

  int Nin;
  struct dynlib_inlist_t *inlist; // Pointer auf Eingangliste, die verknuepfungen zu anderen Blöcken herstelt.  (1. Eintrag)

  int (*comp_func)(int flag, struct dynlib_block_t *block);  // Computational function
  
  char block_initialised; // Was the INIT-FLAG of the comp_func successfully called?
  
  void *work;

  // execution lists

  struct dynlib_block_t *exec_list_prev;
  struct dynlib_block_t *exec_list_next;
  struct dynlib_block_t *exec_sup_list_prev;
  struct dynlib_block_t *exec_sup_list_next;

  // allblocks list
  struct dynlib_block_t *allblocks_list_prev;
  struct dynlib_block_t *allblocks_list_next;

  // simulator helper vars

  int needs_output_update; // After updated states is called this is set to 1 in order to indicate the need for the need of output an update 

  // Events
  int event_mask;
  int Nevents; // number of subscribed events
  char event_map[LIBDYN_MAX_BLOCK_EVENTS]; // maps block side event (index) to simulation event (array values)
  int block_events; // active events that the block receives

  // Used by execution list generation algorithm

  int remaining_input_availability;
  char block_executed;

  // comput. func. extra parameters
  void *extraparam;
  
  // 
  int irpar_config_id;
  int btype;
};


int libdyn_new_event(struct dynlib_simulation_t *simulation);
int libdyn_new_event_notifier(struct dynlib_simulation_t *simulation, struct dynlib_block_t *block, int event);
void libdyn_event_trigger(struct dynlib_simulation_t *simulation, int event);
void libdyn_event_trigger_mask(struct dynlib_simulation_t *simulation, int event_mask);
void libdyn_clock_event_generator(struct dynlib_simulation_t *simulation);

#define __libdyn_event_triggered(simulation, block) ( ( (simulation)->events.event_active & (block)->event_mask) > 0 )
#define __libdyn_event_get_block_events(block) ( (block)->block_events )


//
// usage un comp_func:
// Check if "event" is triggered for "block"
//
#define __libdyn_event_check(block, event) ( (block)->block_events & ( 1 << (event) ) )

// ALternative to above which makes event_mapper obsolete FIXME TEst that
// int libdyn_event_check(struct dynlib_block_t *block, int event);
// #define __libdyn_event_check(block, event) ( libdyn_event_check((block),  (event))  )

int libdyn_config_get_datatype_len(int datatype); // get number of bytes requires by one element of type datatype
void libdyn_block_dumpinfo(struct dynlib_block_t *block);
void libdyn_block_setid(struct dynlib_block_t *block, char *id);
void libdyn_config_block(struct dynlib_block_t *block, int block_type, int Nout, int Nin, void *work, int own_outcache);
void libdyn_config_outcache(struct dynlib_block_t *block, int Nout,  void *data);
int libdyn_config_block_input(struct dynlib_block_t *block, int in, int len, int datatype);
int libdyn_config_block_output(struct dynlib_block_t *block, int out, int len, int datatype, int dinput_dependence);

struct dynlib_simulation_t *libdyn_new_simulation();
void libdyn_del_simulation(struct dynlib_simulation_t *sim);
void libdyn_simulation_set_cfl(struct dynlib_simulation_t *sim, struct lindyn_comp_func_list_head_t *list);

struct dynlib_block_t *libdyn_new_block(struct dynlib_simulation_t *sim, void *comp_func, int *ipar, double *rpar, void *opar, void *extra_par);
void libdyn_del_block(struct dynlib_block_t *block);

void libdyn_load_param(struct dynlib_block_t *block, int *ipar, float *rpar, void *opar);
void libdyn_block_notify_extrapar(struct dynlib_block_t *block, void *par);

int libdyn_block_connect(struct dynlib_block_t *blockfrom, int outNr, struct dynlib_block_t *blockto, int inNr);
int libdyn_block_connect_external(struct dynlib_block_t *blockto, int inNr, void *external_data, int portsize);

int libdyn_simulation_checkinputs(struct dynlib_simulation_t * sim);
int libdyn_simulation_init(struct dynlib_simulation_t * sim);

// installa a callback function for synchronisation of the main loop
void libdyn_simulation_setSyncCallback(struct dynlib_simulation_t *simulation, int (*sync_func)( struct dynlib_simulation_t * sim ) , void *userdat);

int libdyn_simulation_step(struct dynlib_simulation_t *simulation, int update_states); // Einen Schritt weitergehen
void libdyn_simulation_resetblocks(struct dynlib_simulation_t * sim);

int libdyn_add_to_execution_list(struct dynlib_simulation_t *simulation, struct dynlib_block_t *block);

// Ein- und Ausgaben spreicheradressen mitteilen FIXME noch gebraucht?
int libdyn_block_setup_input(struct dynlib_block_t *blockto, int inNr, void *input_data);
int libdyn_block_setup_output(struct dynlib_block_t *blockfrom, int outNr, int external_mem, void *output_data); // Speicher allozieren für ausgänge

int libdyn_setup_executionlist(struct dynlib_simulation_t *simulation);


void libdyn_dump_all_blocks(struct dynlib_simulation_t *sim);



/*
    Basic Filters

*/
// THis is from the basic filter stuff
#define LIBDYN_TF_FILTER 10
#define LIBDYN_SHIFT_FILTER 11
#define LIBDYN_DEJITTER_FILTER 12
#define LIBDYN_IMPULSE_FILTER 13



struct dynlib_filter_t {

  int filter_type;

  double output; // Aktueller AUsgang
  void *add; // aditional filter structure

  //int N, M; // FIlter Ordnung (N); Zählerpoly Ordnung (M)

  //double num_cf[20];
  //double den_cf[20];

  //double state_var[20];

  // Zukünftig:

  // Pointer auf berechenende funktion, create und destroy
  //int N_real_in, N_int_in, N_real_out, N_int_out;
  //int dfeedthrough;

  int fill[1000]; // FIXME :-)
  int magic;

};

struct dynlib_tf_filter_t {
  int N, M; // FIlter Ordnung (N); Zählerpoly Ordnung (M)

  double num_cf[20];
  double den_cf[20];

  double state_var[20];

  int fill[1000];
};

struct dynlib_shift_filter_t {
  int N;

  double state_var[20];

  int fill[1000];
};

struct dynlib_dejitter_filter_t {
  double last_val;
  int got_first_valid_last_val;
  int steps;

  int fill[1000];
};

struct dynlib_impulse_filter_t {
  double last_val;
  double threshold;
};


struct dynlib_filter_t *libdyn_new_filter(int N);
struct dynlib_filter_t *libdyn_new_shift_filter(int N);
struct dynlib_filter_t *libdyn_new_dejitter_filter();
struct dynlib_filter_t *libdyn_new_mean_filter(int N); // Macrofilter
struct dynlib_filter_t *libdyn_new_impulse_filter(double threshold);

int libdyn_delete_filter(struct dynlib_filter_t *p);
void libdyn_print_cf(struct dynlib_filter_t *p);
int libdyn_load_cf__(struct dynlib_filter_t *p, double *num, double *den, int M);
//#define libdyn_load_cf(p, num, den, M)  { printf("called to libdyn_load_cf in %s at line %d\n", __FILE__, __LINE__);         \
//                                          libdyn_load_cf__((p), (num), (den), (M)); } 
#define libdyn_load_cf(p, num, den, M)  {        \
                                          libdyn_load_cf__((p), (num), (den), (M)); } 


void libdyn_null_states(struct dynlib_filter_t *p);
double libdyn_filter(struct dynlib_filter_t *p, double input);
double libdyn_lastout(struct dynlib_filter_t *p);
double libdyn_newout(struct dynlib_filter_t *p, double input);
double libdyn_out(struct dynlib_filter_t *p, double input, int update_states);

struct dynlib_filter_t *libdyn_new_tf_filter_irpar(int *ipar, double *rpar, int id);



/*
 * Create Simulations from irpar datasets
 */

// FIXME move to other file
struct libdyn_io_config_t {
  int inports, outports; // Nr in and out ports
  int *insizes, *outsizes; // Size of each port
  // FIXME: Add intypes and outtypes, but this will require to recompile user software
  double **inptr;
  double **outptr;
  
  char provided_outcaches; // If set to 1 libdyn does not need to alloc memory for outcaches, because the client already did
			    // *outptr will be preset FIXME NEW 25.6.
};

int irpar_get_libdynconnlist(struct dynlib_simulation_t *sim, int *ipar, double *rpar, int id, struct libdyn_io_config_t *iocfg);

struct libdyn_user_blocks_list_ele_t {
  char *name;
  int btype;
  
  int (*comp_func)(int flag, struct dynlib_outlist_t *block);  // Computational function
  
  struct libdyn_user_blocks_list_ele_t *next, *prev;
};


// Helper Marcros for a fixed number of io ports

/*  
    #define Ninp_ACC 2
    #define Noutp_ACC 1
    #define INsizes_ACC {1,1}
    #define OUTsizes_ACC {1}
*/

// defines a simulation struct containing portsizes etc...
#define __libdyn_irpar_def_sim_vars(structname, Ninp, Noutp) \
  struct {  \
    struct dynlib_simulation_t *sim;  \
    struct libdyn_io_config_t iocfg;  \
  \
    double *inptr_mem[Ninp];  \
    double *outptr_mem[Noutp];  \
  } structname;
  
  
// Initialisation of simulation struct
#define __libdyn_irpar_cfgports(varname, Ninp, Noutp, INsizes, OUTsizes) \
  {    \
  int insizes83637[] = INsizes;    \
  int outsizes83637[] = OUTsizes;    \
      \
  (varname).iocfg.provided_outcaches = 0; \
      \
  (varname).iocfg.inports = (Ninp);    \
  (varname).iocfg.outports = (Noutp);    \
    \
  (varname).iocfg.insizes = insizes83637;    \
  (varname).iocfg.outsizes = outsizes83637;   \
     \
  (varname).iocfg.inptr = (varname).inptr_mem;    \
  (varname).iocfg.outptr = (varname).outptr_mem;    \
}    


// set input pointer (to double array)
#define __libdyn_irpar_cfg_inptr(varname, inport, inptr__) \
  (varname).iocfg.inptr[(inport)] = (inptr__);
  
// Returns the element's entry of outport
#define __libdyn_irpar_getout(varname, outport, element) \
  (varname).iocfg.outptr[outport][element];
  
  
int libdyn_irpar_setup(int *ipar, double *rpar, int boxid, 
		       struct dynlib_simulation_t **sim,
		       struct libdyn_io_config_t *iocfg);
		       
	
// extracts irpar box, creates simulation, connects and creates blocks, and sets up connectionlist
// return 0 or -1 (error)
#define __libdyn_irpar_setup(varname, ipar, rpar, boxid)     \
  libdyn_irpar_setup((ipar), (rpar), boxid, &(varname).sim, &(varname).iocfg);
  
  
/*
 * Events stuff FIXME not really used
 */

struct libdyn_event_div_t {
  int fak;
  int ofs;
  int counter;
};


struct libdyn_event_div_t *libdyn_new_event_div(int ofs, int fak);
void libdyn_del_event_div(struct libdyn_event_div_t *div);
int libdyn_event_div(struct libdyn_event_div_t *div, int update_states);


//
// malloc
//

void * libdyn_malloc(struct dynlib_simulation_t *sim, unsigned int size, int type);
void libdyn_free(struct dynlib_simulation_t *sim, void * p);

#endif
