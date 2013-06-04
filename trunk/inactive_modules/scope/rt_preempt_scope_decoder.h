//

#include <pthread.h>
#include <stdio.h>
#include <map>

class rt_preempt_scope_decoder_scope {
  
  
  public:
    int type;
    int scope_id;
    char name[30];
    int veclen;    

    rt_preempt_scope_decoder_scope(int type__, int scope_id__, char* name__, int veclen__);
    
    rt_preempt_scope_decoder_scope *scope_list_next;
    
    void *callback_private;
    
    double *buffer;
    int buffer_len; // number of bytes in buffer
    pthread_mutex_t buffer_mutex;
    pthread_mutex_t buffer_mutex_writeprotect;
    
    pthread_mutex_t condition_mutex;
    pthread_cond_t thread_condition;
    int buffer_signal;
    
    int read_buffer(double *data);
    
    void send_cmd(int cmd);
    void destruct();
};

class rt_preempt_scope_decoder {
  private:
    int get_bytes(int n, void *data);
    int process_commands(int stage);
    char read_fifo_fname[256];
    char write_fifo_fname[256];
    int read_fifo_handle, write_fifo_handle;
    
    char read_buffer[1024];
    int read_position;
    
    typedef std::map<int, rt_preempt_scope_decoder_scope *> scope_map_t;
    scope_map_t scope_map;
    
    
    rt_preempt_scope_decoder_scope *scope_list_head;
    rt_preempt_scope_decoder_scope *last_created_scope;
    rt_preempt_scope_decoder_scope *get_next_scope_iter; // Iterator für get_next_scope()
    
    void (*callback)(rt_preempt_scope_decoder_scope*);
    
    double T_a; // Main sample frequency
  public:
    rt_preempt_scope_decoder(char *read_fifo_fname__, char *write_fifo_fname__);
    bool get_scopes();
    double get_main_sample_time();
    bool main_loop();
    rt_preempt_scope_decoder_scope * get_next_scope(); // Multiple calls return a list of all scopes
    void destruct();
    void set_callback(void (*fn)(rt_preempt_scope_decoder_scope*));
    
    void send_cmd_to_all_scopes(int cmd);
};
