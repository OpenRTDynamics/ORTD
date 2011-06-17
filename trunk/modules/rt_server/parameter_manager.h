#ifndef _PARAMETER_MANAGER_H
#define _PARAMETER_MANAGER_H 1

#include "rt_server.h"
#include "directory.h"

class parameter_manager;

// from libdyn.h
#define DATATYPE_UNCONFIGURED -1
#define DATATYPE_FLOAT 1
#define DATATYPE_SHORTFLOAT 4
#define DATATYPE_INT 2
#define DATATYPE_BOOLEAN 3


class parameter {
  public:
    parameter();
    parameter(int type, int const_size);
    
    int parse_and_set(char * line);
    
    int datatype;
    int nElements, byte_size;
    void *data;
    void *data_loadbuffer; // buffer
    
    void destruct();
    
    void lock_buffer();
    void unlock_buffer();
    void *get_data_ptr();
    
    void atomic_buffer_copy_d2b();  // FIXME mutex protection
    void atomic_buffer_copy(void *dest);
    void atomic_buffer_copy_e2d(void *source);
    
  private:
    void atomic_buffer_copy_b2d();  // FIXME mutex protection
};


class parameter_manager {
  public:
    // register set and get commands to rt_server_threads_manager
    parameter_manager( rt_server_threads_manager* rts_thmng, directory_tree* root_directory );
    void destruct();
    
    directory_tree * directory;
    
    rt_server_threads_manager * rts_thmng;
    
    parameter * new_parameter( char *name, int type, int size );
//     parameter_directory::direntry *access


    void callback_set(rt_server_command *cmd, rt_server *rt_server_src);
    void callback_get(rt_server_command *cmd, rt_server *rt_server_src);
    
};

#endif