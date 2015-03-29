#include <stdio.h>

extern "C" {

  int libdyn_module_Random_siminit(struct dynlib_simulation_t *sim, int bid_ofs);

  int ortd_plugin_init(struct dynlib_simulation_t *sim)
  {
    printf("ORTD Template plugin init\n");

//     Register my blocks to the given simulation

//     int blockid_ofs = 100001;
//     
// //     printf("\n");
//     
//     libdyn_compfnlist_add(sim->private_comp_func_list, blockid_ofs, LIBDYN_COMPFN_TYPE_LIBDYN, &compu_func_sensor_actor);

    libdyn_module_Random_siminit(sim, 0);
  
    return 0;
  }

} // END EXTERN C