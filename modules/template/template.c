#include "libdyn.h"
#include "libdyn_scicos_macros.h"


int compu_func_template(int flag, struct dynlib_block_t *block)
{
  
  //printf("comp_func gain: flag==%d\n", flag);
  int Nout = 0;
  int Nin = 1;

  double *in;

  double *rpar = libdyn_get_rpar_ptr(block);
  int *ipar = libdyn_get_ipar_ptr(block);

  int vlen = ipar[0];
  int namelen = ipar[1];
  int *codedname = &ipar[2];
  
  
  switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {  
    }
      return 0;
      break;
    case COMPF_FLAG_UPDATESTATES:
    {
      in = (double *) libdyn_get_input_ptr(block,0);
    
    } 
      return 0;
      break;
    case COMPF_FLAG_CONFIGURE:  // configure
    {
      
      libdyn_config_block(block, BLOCKTYPE_DYNAMIC, Nout, Nin, (void *) 0, 0); 
      libdyn_config_block_input(block, 0, vlen, DATATYPE_FLOAT); 
    } 
      return 0;
      break;
    case COMPF_FLAG_INIT:  // init
    {
//      char filename[250];
      char *str = (char *) malloc(namelen+1);
      
      // Decode filename
      int i;
      for (i = 0; i < namelen; ++i)
	str[i] = codedname[i];
      
      str[i] = 0; // String termination
      
      
      printf("New scope %s\n", str);
      
      
      free(str);
      
    }
      return 0;
      break;
    case COMPF_FLAG_DESTUCTOR: // destroy instance
    {

    }
      return 0;
      break;      
    case COMPF_FLAG_PRINTINFO:
      printf("I'm a template block\n");
      return 0;
      break;
      
  }
}

//#include "block_lookup.h"

int libdyn_module_template_siminit(struct dynlib_simulation_t *sim, int bid_ofs)
{
  // printf("libdyn module siminit function called\n"); 
  
  // Register my blocks to the given simulation
  
  int blockid = 990001;
  libdyn_compfnlist_add(sim->private_comp_func_list, blockid, LIBDYN_COMPFN_TYPE_LIBDYN, &compu_func_template);
}


//} // extern "C"
