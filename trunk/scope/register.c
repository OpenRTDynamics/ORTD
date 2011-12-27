#include "libdyn.h"

int compu_func_scope(int flag, struct dynlib_block_t *block);

int libdyn_module_scope_siminit(struct dynlib_simulation_t *sim, int bid_ofs)
{
  // printf("libdyn module siminit function called\n"); 
  
  // Register my blocks to the given simulation
  
  int blockid = 10001;
  libdyn_compfnlist_add(sim->private_comp_func_list, blockid, LIBDYN_COMPFN_TYPE_LIBDYN, &compu_func_scope);
}

