//

#include "rt_preempt_scope_decoder.h"



void callback(rt_preempt_scope_decoder_scope * scope)
{
  // printf("callbac: private is %d scope name %s, veclen=%d\n", scope->callback_private, scope->name, scope->veclen); 
   int i;
   
   void * work_ptr =  scope->callback_private;
   
   printf("sid = %d, vec=[", scope->scope_id);
   for (i=0; i < scope->veclen; ++i) { //scope->veclen
     printf("%f ", scope->buffer[i]);
     //printf("\n ");
   }
   printf("]\n");
}

int main() {
  for (;;) {
  
    printf("######################\n");
    printf("######################\n");
    printf("######################\n");
    printf("######################\n");
    
  
  rt_preempt_scope_decoder *rtpd = new rt_preempt_scope_decoder("rt_preempt_output", "");
  rtpd->set_callback(&callback);
  
  // collect scopes
  rtpd->get_scopes();

  rt_preempt_scope_decoder_scope * scope;
  
  // list all scopes
  
  printf("============ List of scopes =============\n");
  do {
    scope = rtpd->get_next_scope();
    if (scope != 0) {
      printf("--> ");
      printf("  %d: %s\n", scope->scope_id, scope->name);
      
      scope->callback_private = (void*) 123; // Assign an user defined pointer to each scope
      
    }
  } while (scope != 0);
  
  printf("Main sample time = %f\n", rtpd->get_main_sample_time());
    
  rtpd->main_loop();
  
  printf("Exiting\n");
  
  rtpd->destruct();
  
  
  }
  
  return 0;
}
