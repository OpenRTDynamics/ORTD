#include <stdio.h>

#include <libdyn.h>

#include "gpio.h"

int compu_func_BB_sensor_actor(int flag, struct dynlib_block_t *block)
{
  //  printf("comp_func mux: flag==%d; irparid = %d\n", flag, block->irpar_config_id);
    int *ipar = libdyn_get_ipar_ptr(block);
    double *rpar = libdyn_get_rpar_ptr(block);

    int Nout = 1;
    int Nin = 1;


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
            double *out = (double *) libdyn_get_output_ptr(block,0);
            double *in = (double *) libdyn_get_input_ptr(block, 0);
	    
	    printf("Actuation value %f\n", in[0]);
	    
	    if (*in > 0) {
	      digitalWrite(USR3, HIGH);
	    } else {
	      digitalWrite(USR3, LOW);
	    }
	    
	    
            out[0] = 12.34 ; // dummy output

    }
    return 0;
    break;
    case COMPF_FLAG_UPDATESTATES:
        return 0;
        break;
    case COMPF_FLAG_CONFIGURE:  // configure
    {
        libdyn_config_block(block, BLOCKTYPE_STATIC, Nout, Nin, (void *) 0, 0);

        libdyn_config_block_input(block, 0, 1, DATATYPE_FLOAT); 
        libdyn_config_block_output(block, 0, 1, DATATYPE_FLOAT, 1);
    }
    return 0;
    break;
    case COMPF_FLAG_INIT:  // init
        {
	  init(); 
	}

        return 0;
        break;
    case COMPF_FLAG_DESTUCTOR: // destroy instance
        return 0;
        break;
    case COMPF_FLAG_PRINTINFO:
        printf("I'm an beaglbone example block\n");
        return 0;
        break;

    }
}


int ortd_plugin_init(struct dynlib_simulation_t *sim)
{
  printf("ORTD BB test plugin init\n");

//     Register my blocks to the given simulation

    int blockid_ofs = 100001;
    
//     printf("\n");
    
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs, LIBDYN_COMPFN_TYPE_LIBDYN, &compu_func_BB_sensor_actor);
  
  return 0;
}
