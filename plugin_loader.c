/*
    Copyright (C) 2010, 2011  Christian Klauer

    This file is part of OpenRTDynamics, the Real-Time Dynamics Framework

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