#include <pthread.h>

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
    
    ~ortd_global_shared_object();
    
    void init() {
      
    };
    
    
  private:
    const char *identName;

    libdyn_master* ldmaster; // extracted from simnest
      
      void *buffer;
      
      int usage_counter;
      pthread_mutex_t counter_mutex;
};

