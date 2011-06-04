//

#include <stdio.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

#include "libdyn.h"

void *load_solib(struct dynlib_simulation_t *sim, char *name)
{
   void *lib_handle;
   int (*init_fn)(struct dynlib_simulation_t *sim);
   
   int x;
   char *error;

   lib_handle = dlopen(name, RTLD_LAZY);
   if (!lib_handle) 
   {
      fprintf(stderr, "%s\n", dlerror());
      exit(1);
   }

   init_fn = dlsym(lib_handle, "ld_plugin_init");
   if ((error = dlerror()) != NULL)  
   {
      fprintf(stderr, "%s\n", error);
      exit(1);
   }

   // call init function
   int ret = (*init_fn)(sim);

   if (ret != 0) {
     fprintf(stderr, "libdyn: Init of plugin %s faild with return values %d\n", name, ret);
     exit(1);
   }
   
   return lib_handle;

//    dlclose(lib_handle);
}