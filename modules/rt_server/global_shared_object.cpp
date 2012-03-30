#include <pthread.h>

#include <libdyn_cpp.h>
#include "directory.h"
#include "global_shared_object.h"

ortd_global_shared_object::ortd_global_shared_object(const char* identName, libdyn_master *master)
{
  this->identName = identName; // FIXME MAKE A COPY INSTEAD!
  this->usage_counter = 0;  
  this->ldmaster = master;
  
  if (this->ldmaster == NULL) {
    fprintf(stderr, "stderr: ortd_global_shared_object: needs a libdyn master\n");
    // FIXME: throuw exception
    
    return;
  }
  
  directory_tree *dtree = ldmaster->dtree;
  if (dtree == NULL) {
    fprintf(stderr, "stderr: ortd_global_shared_object: needs a root directory_tree\n");
    // FIXME: throuw exception
     
  }
  
  
  if (dtree->add_entry((char*) identName, ORTD_DIRECTORY_ENTRYTYPE_PERSISTENTMEMORY, this, this) == false) {
     fprintf(stderr, "stderr: ortd_global_shared_object: cound not allocate the filename %s\n", identName);
  }
  
  // alloc memory for the buffer

  pthread_mutex_init(&counter_mutex, NULL);
  
  init();
}


ortd_global_shared_object::~ortd_global_shared_object()
{
  directory_tree *dtree = ldmaster->dtree;
  dtree->delete_entry((char*) identName);
  
    pthread_mutex_destroy(&counter_mutex);

}

