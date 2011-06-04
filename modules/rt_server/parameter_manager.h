#include "rt_server.h"


class parameter_manager;



class parameter_manager {
  public:
    // register set and get commands to rt_server_threads_manager
    parameter_manager( rt_server_threads_manager * rts_thmng );
    
    directory_leaf * root_directory;
    
    rt_server_threads_manager * rts_thmng;
    
//     parameter_directory::direntry *access
};