#include <pthread.h>

#include <libdyn_cpp.h>
#include "directory.h"
#include "global_shared_object.h"

#include <stdlib.h>
#include <assert.h>


void * get_ortd_global_shared_object(char *identName, libdyn_master* master)
{
  directory_tree *dtree = master->dtree;
  if (dtree == NULL) {
    fprintf(stderr, "stderr: ortd_global_shared_object: needs a root directory_tree\n");
    // FIXME: throuw exception
     
  }
  
  
  // Check wheter the dir entry is already available

   ortd_global_shared_object * gsho = (ortd_global_shared_object *) dtree->access_userptr(identName, NULL);

   return (void*) gsho;
//   if (dtree->add_entry((char*) identName_.c_str(), ORTD_DIRECTORY_ENTRYTYPE_PERSISTENTMEMORY, this, this) == false) { // remove ORTD_DIRECTORY_ENTRYTYPE_PERSISTENTMEMORY
//      fprintf(stderr, "stderr: ortd_global_shared_object: cound not allocate the filename %s\n", identName);
//   }

}




ortd_global_shared_object::ortd_global_shared_object(const char* identName, libdyn_master *master)  // FIXME: Add type of file like ORTD_DIRECTORY_ENTRYTYPE_PERSISTENTMEMORY
{
//   this->identName = identName; // FIXME MAKE A COPY INSTEAD!
  this->identName_ = identName;
  
  
  this->usage_counter = 0;  
  this->ldmaster = master;

      fprintf(stderr, "libdyn: new ortd_global_shared_object <%s>\n", identName_.c_str());

  
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
  
  
  // Check wheter the dir entry is already available
  
  if (dtree->add_entry((char*) identName_.c_str(), ORTD_DIRECTORY_ENTRYTYPE_SHAREDOBJECT, 0x0, this) == false) { // remove ORTD_DIRECTORY_ENTRYTYPE_PERSISTENTMEMORY
     fprintf(stderr, "stderr: ortd_global_shared_object: cound not allocate the filename %s\n", identName);
      // FIXME: throuw exception
  }
  
  // alloc memory for the buffer

  pthread_mutex_init(&counter_mutex, NULL);
}


ortd_global_shared_object::~ortd_global_shared_object()
{
  if (!isUnused()) {
    fprintf(stderr, "ortd: ortd_global_shared_object: ASSERTION FAILD: Detructing an instance that i still in use!\n");
    exit(0);
    //assert();
  }
  
  // remove the directory entry
  directory_tree *dtree = ldmaster->dtree;
  dtree->delete_entry((char*) identName_.c_str());
  
    pthread_mutex_destroy(&counter_mutex);

}

