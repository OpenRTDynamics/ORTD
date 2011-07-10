/*
    Copyright (C) 2010, 2011  Christian Klauer

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

#include <malloc.h>

extern "C" {

#include "libdyn_scicos_macros.h"
#include "irpar.h"
}

#include "libdyn_cpp.h"





extern "C" {
    int libdyn_module_nested_siminit(struct dynlib_simulation_t *sim, int bid_ofs);
}

class compu_func_nested_class {
public:
    compu_func_nested_class(struct dynlib_block_t *block);
    void destruct();
    void io(int update_states);
    int init();
private:
    struct dynlib_block_t *block;


    libdyn_nested * simnest;
    libdyn_master * master;
    irpar *param;

    int Nin;
    int Nout;
    int *insizes, *outsizes;
    int *intypes, *outtypes;

    int error;

};

compu_func_nested_class::compu_func_nested_class(dynlib_block_t* block)
{
    this->block = block;
}

int compu_func_nested_class::init()
{
    double *rpar = libdyn_get_rpar_ptr(block);
    int *ipar = libdyn_get_ipar_ptr(block);

    struct irpar_ivec_t insizes_irp, outsizes_irp, intypes_irp, outtypes_irp, param;


    irpar_get_ivec(&insizes_irp, ipar, rpar, 10);
    irpar_get_ivec(&outsizes_irp, ipar, rpar, 11);
    irpar_get_ivec(&intypes_irp, ipar, rpar, 12);
    irpar_get_ivec(&outtypes_irp, ipar, rpar, 13);
    irpar_get_ivec(&param, ipar, rpar, 20);



    this->insizes = insizes_irp.v;
    this->outsizes = outsizes_irp.v;
    this->intypes = intypes_irp.v;
    this->outtypes = outtypes_irp.v;


// 	int dfeed = param.v[1];

    Nin = insizes_irp.n;
    Nout = outsizes_irp.n;

    /*



      */


    bool use_buffered_input = true;  // in- and out port values are buffered
    simnest = new libdyn_nested(Nin, insizes, intypes, Nout, outsizes, outtypes, use_buffered_input);

    printf("simnest = %p\n", simnest);

    // If there is a libdyn master : use it
    master = (libdyn_master *) block->sim->master;
    if (master == NULL) {  // no master available
//       fprintf(stderr, "WARNING: libdyn: parameter block requires a libdyn master\n");
    }

    simnest->set_master(master);

    int shematic_id = 900;

    printf("loading shematic id %d\n", 900);
    if (simnest->add_simulation(ipar, rpar, shematic_id) < 0) {
        return -1;  // An error
    }

    printf("added schematic\n");



    return 0;
}


void compu_func_nested_class::io(int update_states)
{
  
  int i,j;

  
  for (i=0; i< Nin ; ++i) {
    double *in_p = (double*) libdyn_get_input_ptr(block, i);
//     printf(".. %p %d\n", in_p, block->Nin);
    simnest->copy_inport_vec(i, in_p);
  }
  
  // map scicos events to libdyn events
  // convert scicos events to libdyn events (acutally there is no conversion)
  int eventmask = __libdyn_event_get_block_events(block);
  simnest->event_trigger_mask(eventmask);
  
  if (update_states == 1) {
    simnest->simulation_step(1);
//        printf("neszed sup\n");
  } else {
    simnest->simulation_step(0);
    
    for (i=0; i< Nout ; ++i) {
      double *out_p = (double*) libdyn_get_output_ptr(block, i);
      simnest->copy_outport_vec(i, out_p);
      
//       printf("nested outp %f\n", out_p[0]);
    }
    
  }

}

void compu_func_nested_class::destruct()
{
    simnest->destruct();
    delete simnest;
}


int compu_func_nested(int flag, struct dynlib_block_t *block)
{

//     printf("comp_func nested: flag==%d\n", flag);

    double *in;
    double *rpar = libdyn_get_rpar_ptr(block);
    int *ipar = libdyn_get_ipar_ptr(block);

    int Nin = ipar[0];
    int Nout = ipar[1];


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        in = (double *) libdyn_get_input_ptr(block,0);
        compu_func_nested_class *worker = (compu_func_nested_class *) libdyn_get_work_ptr(block);

        worker->io(0);

    }
    return 0;
    break;
    case COMPF_FLAG_UPDATESTATES:
    {
        in = (double *) libdyn_get_input_ptr(block,0);
        compu_func_nested_class *worker = (compu_func_nested_class *) libdyn_get_work_ptr(block);

        worker->io(1);

    }
    return 0;
    break;
    case COMPF_FLAG_CONFIGURE:  // configure. NOTE: do not reserve memory or open devices. Do this while init instead!
    {
//         int irpar_get_ivec(struct irpar_ivec_t *ret, int *ipar, double *rpar, int id);

        struct irpar_ivec_t insizes, outsizes, intypes, outtypes, param;

        irpar_get_ivec(&insizes, ipar, rpar, 10);
        irpar_get_ivec(&outsizes, ipar, rpar, 11);
        irpar_get_ivec(&intypes, ipar, rpar, 12);
        irpar_get_ivec(&outtypes, ipar, rpar, 13);
        irpar_get_ivec(&param, ipar, rpar, 20);

        int dfeed = param.v[1];

        int Ninports = insizes.n;
        int Noutports = outsizes.n;

		
        int i;
        libdyn_config_block(block, BLOCKTYPE_DYNAMIC, Noutports, Ninports + 2, (void *) 0, 0);

        for (i = 0; i < Ninports; ++i) {
            libdyn_config_block_input(block, i, insizes.v[i], DATATYPE_FLOAT);
	}

	libdyn_config_block_input(block, Ninports, 1, DATATYPE_FLOAT); // switch signal input
	libdyn_config_block_input(block, Ninports+1, 1, DATATYPE_FLOAT); // reset signal input

        for (i = 0; i < Noutports; ++i) {
            libdyn_config_block_output(block, i, outsizes.v[i], DATATYPE_FLOAT, dfeed);
	}


    }
    return 0;
    break;
    case COMPF_FLAG_INIT:  // init
    {
        compu_func_nested_class *worker = new compu_func_nested_class(block);
        libdyn_set_work_ptr(block, (void*) worker);

        int ret = worker->init();
        if (ret < 0)
            return -1;
    }
    return 0;
    break;
    case COMPF_FLAG_DESTUCTOR: // destroy instance
    {
        compu_func_nested_class *worker = (compu_func_nested_class *) libdyn_get_work_ptr(block);

        worker->destruct();
	delete worker;

    }
    return 0;
    break;
    case COMPF_FLAG_PRINTINFO:
        printf("I'm a nested block\n");
        return 0;
        break;

    }
}

//#include "block_lookup.h"

int libdyn_module_nested_siminit(struct dynlib_simulation_t *sim, int bid_ofs)
{

    // Register my blocks to the given simulation

    int blockid = 15001;
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &compu_func_nested);

    printf("libdyn module nested initialised\n");

}


//} // extern "C"
