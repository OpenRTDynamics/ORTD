//

#include <stdio.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

#include "libdyn.h"
#include "plugin_loader.h"



#ifdef __ORTD_PLUGINS_ENABLED


void *ortd_load_plugin(struct dynlib_simulation_t *sim, char *name)
{
   void *lib_handle;
   int (*init_fn)(struct dynlib_simulation_t *sim);
   
   int x;
   char *error;

   lib_handle = dlopen(name, RTLD_LAZY);
   if (!lib_handle) 
   {
      fprintf(stderr, "%s\n", dlerror());

     return -1;
  }

//    printf("lib loaded\n");

   init_fn = dlsym(lib_handle, "ortd_plugin_init");
   if ((error = dlerror()) != NULL)  
   {
      fprintf(stderr, "%s\n", error);
      dlclose(lib_handle);

     return -1;
  }

// printf("got init fn\n");

   // call init function
   int ret = (*init_fn)(sim);

   if (ret != 0) {
     fprintf(stderr, "libdyn: Init of plugin %s faild with return values %d\n", name, ret);
//      exit(1);
     dlclose(lib_handle);
     return -1;
   }
//       printf("called init fn\n");

   return lib_handle;

//    dlclose(lib_handle);
}

#endif