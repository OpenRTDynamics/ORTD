//

#include "rt_preempt_scope.h"

#include <string.h>
#include <stdio.h>
#include <malloc.h>


#define STREAM_BUFFERSIZE (10000*100)
#define STREAM_FLUSH_INTERVAL 100



/*
 * Necessary global variables
 */
  
int rt_preempt_scope_initialised = 0;
struct {
    int scope_id_counter;
    struct streamtrans_t *stream;
    int active_scopes;
    
    struct rt_preempt_scope_t * scopelist_head;
} rt_preempt_scope_global;

//
// Setup this scope system
//

 /**
  * \brief new instance of this lib
  */
struct rt_preempt_scope_t * rt_preempt_scope_new()
{
  
  if (rt_preempt_scope_initialised == 0) {
    rt_preempt_scope_global.stream = log_streamtrans_new(STREAM_BUFFERSIZE, STREAM_FLUSH_INTERVAL, "rt_preempt_output");
    
    rt_preempt_scope_global.scope_id_counter = 0;
    rt_preempt_scope_initialised = 1;
    rt_preempt_scope_global.active_scopes = 0;
  }
  
  struct rt_preempt_scope_t * scope = (struct rt_preempt_scope_t * ) malloc(sizeof(struct rt_preempt_scope_t));
  scope->scope_id = rt_preempt_scope_global.scope_id_counter++;
  
  printf("New rt_preempt scope\n");
  
  return scope;
}

//
// Setup the main sample rate
//
void rt_preempt_scope_setup_main_sampleRate(struct rt_preempt_scope_t *scope, double T_a)
{
  int command = SCOPE_SETMAINSAMPLERATE;
  log_streamtrans_log(rt_preempt_scope_global.stream, &command, sizeof(int));  
  double T_a_ = T_a;
  log_streamtrans_log(rt_preempt_scope_global.stream, &T_a_, sizeof(double));    
}

 /**
  * \brief Send synchronisation packet to which the client can synchronise in case of stream errors
  */
void rt_preempt_scope_send_synchronisation()
{ // Send synchronisation command
  int command = SCOPE_SYNCHRONISATION;
  log_streamtrans_log(rt_preempt_scope_global.stream, &command, sizeof(int));  
}


//
// Register a new scope name "name"
// type may SCOPE_TYPE_SCOPE, SCOPE_TYPE_METER etc.
// A vector of length veclen_ will be transfered
//
 /**
  * \brief Register a new scope 
  * \param scope Instance as returned by rt_preempt_scope_new()
  * \param type SCOPE_TYPE_SCOPE, SCOPE_TYPE_METER etc.
  * \param veclen__ lengeh of dataset sent to this scope
  * \param name name of the new scope
  */
void rt_preempt_scope_setup(struct rt_preempt_scope_t *scope, int type, int veclen__, char* name__)
{
  rt_preempt_scope_global.active_scopes++; // one more scope

  rt_preempt_scope_send_synchronisation();

  scope->veclen = veclen__;
  strcpy(scope->name, name__);
  
  struct init_scope_packet set_packet;
  
  //set_packet.command = SCOPE_SETUP_CMD;
  int command = SCOPE_SETUP_CMD;
  log_streamtrans_log(rt_preempt_scope_global.stream, &command, sizeof(int));
  
  set_packet.type = type;
  set_packet.veclen = veclen__;
  set_packet.scopeid = scope->scope_id;
  strcpy(set_packet.name, name__);
  
  log_streamtrans_log(rt_preempt_scope_global.stream, &set_packet, sizeof(struct init_scope_packet));
}

 /**
  * \brief Notify about a completed scope setup
  */
void rt_preempt_scope_finish_setup()
{ // Send info that all scopes are now completed
  int command = SCOPE_SETUP_FINISHED;
  log_streamtrans_log(rt_preempt_scope_global.stream, &command, sizeof(int));  
  
  rt_preempt_scope_send_synchronisation();
}


 /**
  * \brief Send a vector to a given scope
  * \param scope scope instance
  * \param vec pointer to the vector to send
  */
void rt_preempt_scope_send_double(struct rt_preempt_scope_t *scope, double *vec)
{
  int command = SCOPE_SEND_CMD;
  log_streamtrans_log(rt_preempt_scope_global.stream, &command, sizeof(int));
  int scopeid__ = scope->scope_id;
  log_streamtrans_log(rt_preempt_scope_global.stream, &scopeid__, sizeof(int));
 
  //vec[0] = 1.1234;
  log_streamtrans_log(rt_preempt_scope_global.stream, vec, scope->veclen * sizeof(double));
  
  log_streamtrans_flush(rt_preempt_scope_global.stream);
  
  rt_preempt_scope_send_synchronisation();
}

 /**
  * \brief Remove a scope
  */
void rt_preempt_scope_unregister(struct rt_preempt_scope_t *scope)
{
  // FIXME: Send info
  // printf("unregister scope\n");
  
  rt_preempt_scope_global.active_scopes--;
  
  int command = SCOPE_UNREG_CMD;
  log_streamtrans_log(rt_preempt_scope_global.stream, &command, sizeof(int));
  int scopeid__ = scope->scope_id;
  log_streamtrans_log(rt_preempt_scope_global.stream, &scopeid__, sizeof(int));
  
  rt_preempt_scope_send_synchronisation();
  
  free(scope); 

    //printf("unregister2\n");

}

 /**
  * \brief Unload
  */
void rt_preempt_scope_destruct()
{
  if (rt_preempt_scope_initialised == 1 && rt_preempt_scope_global.active_scopes == 0) {
    // printf("unloading rt_preempt_scope system\n");
    
    log_streamtrans_flush(rt_preempt_scope_global.stream);
    log_streamtrans_del(rt_preempt_scope_global.stream);
    
    rt_preempt_scope_initialised = 0;
  }
}

//}