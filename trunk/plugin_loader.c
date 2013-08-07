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
#ifdef DEBUG
    fprintf(stderr, "%s\n", dlerror());
#endif      

     return NULL;
  }

//    printf("lib loaded\n");

   init_fn = dlsym(lib_handle, "ortd_plugin_init");
   if ((error = dlerror()) != NULL)  
   {
      // could not open shared lib
//#ifdef DEBUG
      fprintf(stderr, "Could not find entry point ortd_plugin_init: %s\n", error);
      fprintf(stderr, "Make sure you have a C-function called like this.\n");
      fprintf(stderr, "Maybe you forgot to put this function into   extern \"C\" { .... }.\n");
//#endif      
      dlclose(lib_handle);

     return NULL;
  }

// printf("got init fn\n");

   // call init function
   int ret = (*init_fn)(sim);

   if (ret != 0) {
     fprintf(stderr, "libdyn: Init of plugin %s faild with return value %d\n", name, ret);
//      exit(1);
     dlclose(lib_handle);
     return NULL;
   }
//       printf("called init fn\n");

  fprintf(stderr, "libdyn: Initialisation of plugin %s finished\n", name);

   return lib_handle;

//    dlclose(lib_handle);
}

#endif