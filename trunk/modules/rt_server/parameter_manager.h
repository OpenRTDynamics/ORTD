#ifndef _PARAMETER_MANAGER_H
#define _PARAMETER_MANAGER_H 1

#include "rt_server.h"
#include "directory.h"

class parameter_manager;

// // copy from libdyn.h
// #define DATATYPE_UNCONFIGURED 0
// #define DATATYPE_FLOAT (1 | (sizeof(double) << 5))
// #define DATATYPE_SHORTFLOAT 4
// #define DATATYPE_INT 2
// #define DATATYPE_BOOLEAN 3
// #define DATATYPE_EVENT 4


extern "C" {
#include "log.h"
#include "libdyn.h"  // uses datatypes
}

class ortd_ringbuffer {
private:
  ringbuffer_t *rb;
  
public:
  ortd_ringbuffer(int element_size, int num_elements, int autoflushInterval);
  ~ortd_ringbuffer();
  
  int read_nonblock(void* data);
  int read_block(void *data);
  void write(void* data, int numElements);
  void flush();
  
};


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


  /**
    * \brief Parameter management class
    */
class parameter_manager {
  public:
    // register set and get commands to rt_server_threads_manager
    parameter_manager( rt_server_threads_manager* rts_thmng, directory_tree* root_directory );
    void destruct();
    
    directory_tree * directory;
    
    rt_server_threads_manager * rts_thmng;
    
    // create a new parameter within root_directory
    parameter * new_parameter( char *name, int type, int size );
    
    // create a new parameter within directory
    parameter * new_parameter( char *name, int type, int size, directory_tree* directory );

    void delete_parameter(parameter *par);



    void callback_set(rt_server_command *cmd, rt_server *rt_server_src);
    void callback_get(rt_server_command *cmd, rt_server *rt_server_src);
    
};







class ortd_stream {
  public:
    ortd_stream(int datatype, int const_len, int numBufferElements );
    
    
    int parse_and_return(rt_server_command* cmd, rt_server* rt_server_src, char * line);
    void write_to_stream(void *data);
    
    // ortd datatype
    int datatype;
    
    // length of the vector
    int nElements; 
    
    // number of byte required by the vector (depends on datatype and nElements)
    int byte_size;

    
    void destruct();
    
    
  private:
    ortd_ringbuffer *rb;
    void *oneElementBuf;
    int numBytes; // number of bytes for the oneElementBuf
};

  /**
    * \brief Stream management class
    */
class ortd_stream_manager {
  public:
    // register set and get commands to rt_server_threads_manager
    ortd_stream_manager( rt_server_threads_manager* rts_thmng, directory_tree* directory );
    void destruct();
    
    directory_tree * directory;
    
    rt_server_threads_manager * rts_thmng;
    
    ortd_stream * new_stream( char* name, int datatype, int const_len, int numBufferElements );
    void delete_stream(ortd_stream *stream);

    
    void callback_get(rt_server_command *cmd, rt_server *rt_server_src);
};


#endif