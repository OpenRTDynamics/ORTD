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

#include <malloc.h>

extern "C" {
#include "libdyn.h"
#include "libdyn_scicos_macros.h"
#include <math.h>
#include "irpar.h"

}
#include <libdyn_cpp.h>

 #include "directory.h"
#include "nested_onlineexchange.h"


extern "C" int compu_func_nested_exchange_fromfile(int flag, struct dynlib_block_t *block);






class compu_func_nested_exchange_fromfile_class {
public:
    compu_func_nested_exchange_fromfile_class(struct dynlib_block_t *block);
    void destruct();
    void io(int update_states);
    int init();
private:
   struct dynlib_block_t *block;
   
   char *ifname, *rfname;
   char *nested_simname;

};

compu_func_nested_exchange_fromfile_class::compu_func_nested_exchange_fromfile_class(dynlib_block_t* block)
{
    this->block = block;
}

int compu_func_nested_exchange_fromfile_class::init()
{
    double *rpar = libdyn_get_rpar_ptr(block);
    int *ipar = libdyn_get_ipar_ptr(block);

    double Nin = ipar[0];
    double Nout = ipar[1];

//     this->ifname = "ident1_exch.ipar";
//     this->rfname = "ident1_exch.rpar";

    int ofs = 3;
  
    int len_s1 = ipar[ofs+0];
    int len_s2 = ipar[ofs+1];
    int len_s3 = ipar[ofs+2];
    
    int ofs2 = 3; // # of strings
    
    int *s1 = &ipar[ofs+ofs2+0];
    int *s2 = &ipar[ofs+ofs2+len_s1];
    int *s3 = &ipar[ofs+ofs2+len_s1+len_s2];
    
    irpar_getstr(&this->ifname, s1, 0, len_s1);
    irpar_getstr(&this->rfname, s2, 0, len_s2);
    irpar_getstr(&this->nested_simname, s3, 0, len_s3);
    
    fprintf(stderr, "compu_func_nested_exchange_fromfile_class: %s %s %s\n", ifname, rfname, nested_simname);
    
    return 0;
}


void compu_func_nested_exchange_fromfile_class::io(int update_states)
{
    if (update_states==0) {
        double *output = (double*) libdyn_get_output_ptr(block, 0);
        double *in_compresult = (double *) libdyn_get_input_ptr(block,0);
//         double *in_slot = (double *) libdyn_get_input_ptr(block,1);

	
	
	if (*in_compresult < 0.5) {
	  *output = -1;
	  
	  return; 
	}
	
        libdyn_master *master = (libdyn_master *) block->sim->master;
        if (master == NULL || master->dtree == NULL) {  // no master available
           fprintf(stderr, "WARNING: compu_func_nested_exchange_fromfile_class: block requires a libdyn master\n");
 	   *output = -2;
	   
	   return;
	}
	
	// get the identifier
// 	directory_entry::direntry *dentr = master->dtree->access("nested_exchange_test", NULL);
	directory_entry::direntry *dentr = master->dtree->access(nested_simname, NULL);

	if (dentr->type != ORTD_DIRECTORY_ENTRYTYPE_NESTEDONLINEEXCHANGE) {
	  fprintf(stderr, "WARNING: compu_func_nested_exchange_fromfile_class: wrong type for %s\n", "xxx");
	  *output = -3;
	  
	  return;
	}
	
	nested_onlineexchange *exch = (nested_onlineexchange *) dentr->userptr;
	
	// load irpar files and replace a simulation
	// the irpar instance is automatically deleted by nested_onlineexchange
	irpar *par = new irpar();
	par->load_from_afile(this->ifname, this->rfname);

//  	int slot = *in_slot - 1;
	if (exch->replace_second_simulation(par, 100) < 0) {
	  fprintf(stderr, "WARNING: compu_func_nested_exchange_fromfile_class: initialisation of simulation failed %s\n", "xxx");
	  *output = -4;
	  
	  return;
	}

	// ok
	*output = 1;
	fprintf(stderr, "WARNING: compu_func_nested_exchange_fromfile_class: successfully exchanged schematic for <%s>\n", "xxx");
	
    }
}

void compu_func_nested_exchange_fromfile_class::destruct()
{
    free(ifname);
    free(rfname);
    free(nested_simname);
}


int compu_func_nested_exchange_fromfile(int flag, struct dynlib_block_t *block)
{

//     printf("comp_func nested_exchange_fromfile: flag==%d\n", flag);

    double *in;
    double *rpar = libdyn_get_rpar_ptr(block);
    int *ipar = libdyn_get_ipar_ptr(block);

    int Nin = 2;
    int Nout = 1;


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        in = (double *) libdyn_get_input_ptr(block,0);
        compu_func_nested_exchange_fromfile_class *worker = (compu_func_nested_exchange_fromfile_class *) libdyn_get_work_ptr(block);

        worker->io(0);
	
	

    }
    return 0;
    break;
    case COMPF_FLAG_UPDATESTATES:
    {
        in = (double *) libdyn_get_input_ptr(block,0);
        compu_func_nested_exchange_fromfile_class *worker = (compu_func_nested_exchange_fromfile_class *) libdyn_get_work_ptr(block);

        worker->io(1);

    }
    return 0;
    break;
    case COMPF_FLAG_CONFIGURE:  // configure. NOTE: do not reserve memory or open devices. Do this while init instead!
    {
        int i;
        libdyn_config_block(block, BLOCKTYPE_STATIC, Nout, Nin, (void *) 0, 0);

        libdyn_config_block_input(block, 0, 1, DATATYPE_FLOAT);
        libdyn_config_block_input(block, 1, 1, DATATYPE_FLOAT);
        libdyn_config_block_output(block, 0, 1, DATATYPE_FLOAT, 1);


    }
    return 0;
    break;
    case COMPF_FLAG_INIT:  // init
    {
        compu_func_nested_exchange_fromfile_class *worker = new compu_func_nested_exchange_fromfile_class(block);
        libdyn_set_work_ptr(block, (void*) worker);

        int ret = worker->init();
        if (ret < 0)
            return -1;
    }
    return 0;
    break;
    case COMPF_FLAG_DESTUCTOR: // destroy instance
    {
        compu_func_nested_exchange_fromfile_class *worker = (compu_func_nested_exchange_fromfile_class *) libdyn_get_work_ptr(block);

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


