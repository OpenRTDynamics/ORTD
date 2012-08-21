#include <pthread.h>
#include <string>

class ortd_global_shared_object;

// get an instance of an global shared object 
void * get_ortd_global_shared_object(char *identName, libdyn_master *master);


class ortd_global_shared_object {
/*  
 * A generic class for shared objects in ORTD
 * that are refered to by a directory entry
 * 
 * 
 * 
 *    
  */
  public:
    ortd_global_shared_object(const char *identName, libdyn_master *master);
    
    void register_usage() {
      pthread_mutex_lock(&counter_mutex);		
      ++usage_counter;
      pthread_mutex_unlock(&counter_mutex);
    }
    void unregister_usage() {
      pthread_mutex_lock(&counter_mutex);		
      --usage_counter;
      pthread_mutex_unlock(&counter_mutex);
    }
    bool isUnused() {
      bool isUs = true;
      pthread_mutex_lock(&counter_mutex);		
      if (usage_counter == 0) {
	isUs = false;
      }
      pthread_mutex_unlock(&counter_mutex);
      return isUs;
    }
    
    ~ortd_global_shared_object();
    

    
    
  private:
    const char *identName;
    std::string identName_;
    
    libdyn_master* ldmaster; // extracted from simnest
      
      void *buffer;
      
      int usage_counter;
      pthread_mutex_t counter_mutex;
};

