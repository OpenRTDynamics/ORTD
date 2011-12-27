// Decoder that reads out the fifo file
// 

#include <pthread.h>
#include "rt_preempt_scope_decoder.h"

extern "C" {
  #include "rt_preempt_scope.h"

  #include <string.h>
  #include <stdio.h>
  
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <fcntl.h>
  #include <unistd.h>
  #include <stdio.h>
  
  #include <malloc.h>
  
  #include <sys/time.h>
  

}

 /**
  * \brief  New scope; called internally
  */
rt_preempt_scope_decoder_scope::rt_preempt_scope_decoder_scope(int type__, int scope_id__, char* name__, int veclen__)
{
  type = type__;
  scope_id = scope_id__;
  strcpy(name, name__);
  veclen = veclen__;
  
  buffer_len = sizeof(double)*veclen;
  buffer = (double *) malloc(buffer_len);
  
//   pthread_mutex_init(&buffer_mutex, NULL);
//   pthread_mutex_init(&buffer_mutex_writeprotect, NULL);
  
  pthread_mutex_init(&condition_mutex, NULL);
  pthread_cond_init(&thread_condition, NULL);

  buffer_signal = 0;
//   pthread_mutex_unlock(&buffer_mutex);  
}

void rt_preempt_scope_decoder_scope::destruct()
{
//   pthread_mutex_destroy(&buffer_mutex);
//   pthread_mutex_destroy(&buffer_mutex_writeprotect);
  pthread_mutex_destroy(&condition_mutex);
  pthread_cond_destroy(&thread_condition);
}



int rt_preempt_scope_decoder_scope::read_buffer(double* data)
{
  int retval;
  //pthread_mutex_lock(&buffer_mutex); // wait until data is available
  do {
    pthread_cond_wait(&thread_condition, &condition_mutex);
  } while (buffer_signal == 0);
 // printf("log_ringbuffer_avait_signal: sigal received %d\n", buffer_signal);

  retval = 0;
  
  if (buffer_signal == 1)  {
//     pthread_mutex_lock(&buffer_mutex_writeprotect);
    memcpy(data, buffer, buffer_len);
//     pthread_mutex_unlock(&buffer_mutex_writeprotect);
  }

  if (buffer_signal == 2)  { // abort signal
    printf("scope aborting\n");
  }
  
  retval = buffer_signal;
  
  buffer_signal = 0; // reset signal nr
  pthread_mutex_unlock(&condition_mutex);  

  return retval;
}


 /**
  * \brief  Send command to the scope readout thread. function read_buffer will return
  * \param cmd Command number
  */
void rt_preempt_scope_decoder_scope::send_cmd(int cmd)
{
  printf("scope send_cmd to spec scope\n");
	    pthread_mutex_lock(&this->condition_mutex);
	    
	    // let the specific scope thread continue
	    this->buffer_signal = cmd;
	    pthread_mutex_unlock(&this->condition_mutex);
	    pthread_cond_signal(&this->thread_condition); // Notify reader thread  
}


 /**
  * \brief  Constructor
  * \param read_fifo_fname__ Filename of read fifo
  * \param write_fifo_fname__ Filename of write fifo UNUSED
  */
rt_preempt_scope_decoder::rt_preempt_scope_decoder(char* read_fifo_fname__, char* write_fifo_fname__)
{
  strcpy(read_fifo_fname, read_fifo_fname__);
  strcpy(write_fifo_fname, write_fifo_fname__);
  
  read_fifo_handle = open(read_fifo_fname, O_RDONLY);

  read_position = 0;
  
  get_next_scope_iter = 0; // Initialise iteration for get_next_scope
  scope_list_head = 0;
  last_created_scope = 0;
  
  callback = 0;
  
  T_a = 0.1; // default main sample rate
}

 /**
  * \brief  Return the main sample time used by the realtime programm
  */
double rt_preempt_scope_decoder::get_main_sample_time()
{
  return T_a;
}


 /**
  * \brief  Register callback that is called when new scope values arrive
  */
void rt_preempt_scope_decoder::set_callback(void (*fn)(rt_preempt_scope_decoder_scope*))
{
  callback =  fn;
}


 /**
  * \brief  Get new byte from stream
  *
  *         
  *
  * \param	n   How much bytes to retrive
  * \param 	data Pointer where to write the data
  * \return	            bytes actually read
  *
  */
int rt_preempt_scope_decoder::get_bytes(int n, void* data)
{
  int read_counter = 0;
  
  // Cannot read more than 1000 bytes at once
  if (n > 1000)
    n = 1000;
  
  int bytes_left = n;
  
  
  do { // FIXME: Do optimisation for speed when this function is called for a low number of bytes
    
//     fd_set rfds;
//     struct timeval tv;
//     int retval;
//     
//     /* Watch stdin (fd 0) to see when it has input. */
//     FD_ZERO(&rfds);
//     FD_SET(read_fifo_handle, &rfds);
//     /* Wait up to five seconds. */
//     tv.tv_sec = 1;
//     tv.tv_usec = 0;
// 
//     retval = select(read_fifo_handle+1, &rfds, NULL, NULL, &tv);
//     
//     if (retval)
//         printf("Data is available now.\n");
//         /* FD_ISSET(0, &rfds) will be true. */
//     else
//         printf("No data within five seconds.\n");
// 
//     
    int numRead = read(read_fifo_handle, (void *) ( ((char*)&read_buffer) + read_counter), bytes_left);
    read_counter += numRead;
    bytes_left -= numRead;
  } while (read_counter != n);
  
//  printf("n=%d, read_counter=%d\n", n, read_counter);


  memcpy(data, read_buffer, read_counter);
  
  return read_counter;
}

 /**
  * \brief  Send command to the scope readout thread for all scopes. The functions read_buffer will return
  * \param cmd Command number
  */

void rt_preempt_scope_decoder::send_cmd_to_all_scopes(int cmd)
{
  rt_preempt_scope_decoder_scope *current, *tmp;
  
  printf("scope cmd\n");
  
  current = scope_list_head;
  if (current != 0)
    do {
      tmp = current;
      
      tmp->send_cmd(cmd);
      
      current = tmp->scope_list_next;
    } while (current != 0);

}


 /**
  * \brief  Internal loop for processing commands
  */
int rt_preempt_scope_decoder::process_commands(int stage)
{
  
  // stage = 1: get scopes and return
  // stage = 2: get scopes data
  
  
  int command;
  
  int break_loop = 0;
  bool avait_synchronisation = true;
  
  do {
  
    get_bytes(sizeof(int), &command);
  
   // printf("got command = %d\n", command);

    if (avait_synchronisation == false) {
      switch(command) {
	case SCOPE_SETUP_CMD :
	{
	    struct init_scope_packet scope;
	    
	    get_bytes(sizeof(struct init_scope_packet), &scope);

	    printf("got a scope named %s\n", scope.name);
	    
	    rt_preempt_scope_decoder_scope *scope_class = new rt_preempt_scope_decoder_scope(scope.type, scope.scopeid, scope.name, scope.veclen);
	    if (scope_list_head == 0) { // new head of list
	      scope_list_head = scope_class;
	      scope_class->scope_list_next = 0;
	    } else { // insert into head of list
	      scope_class->scope_list_next = scope_list_head;
	      scope_list_head = scope_class;
	    }
	    scope_map.insert(std::make_pair(scope.scopeid, scope_class));
	}
	break;
	case SCOPE_SETMAINSAMPLERATE :
	{
	    double T_a_;	    
	    get_bytes(sizeof(double), &T_a_);
	    T_a = T_a_;
	    printf("Got main samplerate %f\n", T_a);
	}
	break;
	case SCOPE_SETUP_FINISHED :
	{
          printf("scope setup finished: loop end\n");
	  break_loop = 1;
	}
	break;
	case SCOPE_SYNCHRONISATION :
	{
	  ; // nop
	}
	break;
	case SCOPE_ENDOFSTREAM :
	{
	  if (stage == 2) {
	    break_loop = 3;
	    printf("loop end\n");
	  }
	}
	break;
	case SCOPE_UNREG_CMD :
	{
	  int scopeid__;
	  get_bytes(sizeof(int), &scopeid__);
	  
	  printf("Got unregister command\n");
	  if (stage == 2) {
	    break_loop = 2;
	    printf("loop end\n");
	  }
	}
	break;
	case SCOPE_SEND_CMD : // scope new data
	{
	  int scopeid__;
	  get_bytes(sizeof(int), &scopeid__);
	    
	  // Lookup scopeid
	  scope_map_t::iterator iter = scope_map.find(scopeid__);
	
	  if (iter != scope_map.end()) {
	    rt_preempt_scope_decoder_scope *scope_class = iter->second;
	    //printf("got a scope update for scopeif=%d, name=%s\n", scopeid__, scope_class->name);

	    
	    // copy vector to buffer
	    //printf("copy %d bytes to %x\n", scope_class->buffer_len, scope_class->buffer);
	    int numRead = 0;
	    int toRead = scope_class->buffer_len;
	    
	    pthread_mutex_lock(&scope_class->condition_mutex);
	    
	    do {
	      int nRead = get_bytes(toRead, (void*) (((char*) (scope_class->buffer)) + numRead) );

	      numRead += nRead;
	      toRead -= nRead;
	    } while(toRead > 0);

	    // let the specific scope thread continue
	    scope_class->buffer_signal = 1;
	    pthread_mutex_unlock(&scope_class->condition_mutex);
	    pthread_cond_signal(&scope_class->thread_condition); // Notify reader thread  
	    

	    // call callback function if there is one
	    if (callback != 0 && stage == 2) {
	      (*callback) (scope_class);
	    }
	      
	    if (stage == 1) { // BREAK scope collection loop as there is the first scope data available
	      printf("Got first data - break loop\n");

	      break_loop = 1;
	    }

	  } else {
	    printf("Received unknow scopeid %d\n", scopeid__);
	    avait_synchronisation = true;
	  }
	}
	break;
	default :
	{
	  printf("got nonsense\n");
	  avait_synchronisation = true;
	}
	break;  
      }
      
    } else { // synchronisation needed
      if (command == SCOPE_SYNCHRONISATION) {
	avait_synchronisation = false;
	printf("Sychronised Stream\n");
      }
    }
  
    
  } while (break_loop == 0);
  
  if (break_loop == 2)
    send_cmd_to_all_scopes(2);
  
  return break_loop;
}


 /**
  * \brief  Collect scopes from stream. Exit when first scope data 
  */
bool rt_preempt_scope_decoder::get_scopes()
{
  int stage = 1;
  
  this->process_commands(stage); // stage = 1: get scopes and return
}

 /**
  * \brief  Main loop function that receives data from scope and calls user callbacks
  */
bool rt_preempt_scope_decoder::main_loop()
{
  int stage = 2;
  
  this->process_commands(stage); // stage = 2: get scopes data until end of stream
}

 /**
  * \brief  Called after get_scopes(); Every call returns one detected scope until all scopes were returned
  * \return Scope class
  */
rt_preempt_scope_decoder_scope* rt_preempt_scope_decoder::get_next_scope()
{
  if (get_next_scope_iter == 0) { // zum ersten mal aufgerufen
    if (scope_list_head == 0) {
      printf("no scope in list\n");
      // No scope in list
      return 0;
    } else
      get_next_scope_iter = scope_list_head;
  } else { // go further in scope list
    get_next_scope_iter = get_next_scope_iter->scope_list_next; // the end of list this is zero
  }

  return get_next_scope_iter;
}

 /**
  * \brief Destruct class
  */
void rt_preempt_scope_decoder::destruct()
{
  scope_map.clear();
  
  rt_preempt_scope_decoder_scope *current, *tmp;
  
  current = scope_list_head;
  if (current != 0)
    do {
      tmp = current;
      delete(current);
      current = tmp->scope_list_next;
    } while (current != 0);
}

