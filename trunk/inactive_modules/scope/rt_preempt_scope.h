//

//extern "C" {
  #include "log.h"
//}

/*
class rt_preempt_scope {
  private:
    static int scope_id_counter;
    static int initialised;
    static struct streamtrans_t *stream;
    
    int scope_id;
    char name[30];
    int veclen;
  public:
    rt_preempt_scope();
    
    void setup(int veclen__, char *name__);
    void unregister();
    void send_double_values(double *vec);
};
*/

//extern "C" {

#define SCOPE_SETUP_CMD 0xe597fe12
#define SCOPE_SEND_CMD 0xe597fe13
#define SCOPE_UNREG_CMD 0xe597fe14

#define SCOPE_SETUP_FINISHED 0xe597fe15
#define SCOPE_ENDOFSTREAM 0x37ef49c3
#define SCOPE_SYNCHRONISATION 0x9aef534b
#define SCOPE_SYNCHRONISATION1 0x9aef534b
#define SCOPE_SYNCHRONISATION2 0x9aef534b
#define SCOPE_SETMAINSAMPLERATE 0x3658e8c2

#define SCOPE_TYPE_SCOPE 64
#define SCOPE_TYPE_METER 65
#define SCOPE_TYPE_LOG 66

struct init_scope_packet { // Stream packet for configuring a new scope
 // int command;
  int type;
  int veclen;
  int scopeid;
  char name[30];
};

  
  struct rt_preempt_scope_t {
    int type;
    int scope_id;
    char name[30];
    int veclen;   
    
    struct rt_preempt_scope_t * scopelist_next;
  };
  
  
  struct rt_preempt_scope_t * rt_preempt_scope_new();
  void rt_preempt_scope_setup(struct rt_preempt_scope_t *scope, int type, int veclen__, char* name__);
  void rt_preempt_scope_finish_setup();
  void rt_preempt_scope_send_double(struct rt_preempt_scope_t *scope, double *vec);
  void rt_preempt_scope_unregister(struct rt_preempt_scope_t *scope);
  void rt_preempt_scope_destruct();
  
//}