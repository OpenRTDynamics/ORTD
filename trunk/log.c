/*
    Copyright (C) 2010, 2011  Christian Klauer

    This file is part of OpenRTDynamics, the Real Time Dynamic Toolbox

    OpenRTDynamics is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenRTDynamics is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with OpenRTDynamics.  If not, see <http://www.gnu.org/licenses/>.
*/


//


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "log.h"

#define mydebug(level) if ((level) >= 10)



//
// Ringbuffer for separate reader and writer threads
// Reader thread must be started by the user of this buffer
//


//
// create new ringbuffer
//
// one element will consume "element_size" whereby memory for "num_elements"
// will be allocated. After "autoflushInterval" written elements to that
// buffer the reader thread will be notified
//
struct ringbuffer_t *log_ringbuffer_new(int element_size, int num_elements, int autoflushInterval)
{
  struct ringbuffer_t *rb = (struct ringbuffer_t *) malloc(sizeof(struct ringbuffer_t));
  
  rb->buf = (void *) malloc(element_size*num_elements);
  printf("ringbuffer: allocated %d elements of size %d. %d bytes in total\n", num_elements, element_size, element_size*num_elements);
  rb->num_elements = num_elements;
  rb->element_size = element_size;
  rb->write_cnt = 0;  // pointer to the next dataset to write
  rb->read_cnt = 0; // pointer to the next dataset to read
  rb->elements_pending = 0;
  
  pthread_mutex_init(&rb->thread_mutex, NULL);
  pthread_cond_init(&rb->thread_condition, NULL);
  rb->thread_command = 0;
  
  // init the signal ring buffer
  rb->special_signal = 0; // FIXME REMOVE THIS
  rb->cntrl_signal_ring.bufsize = 100;
  rb->cntrl_signal_ring.readpos = 0;
  rb->cntrl_signal_ring.writepos = 0;
  rb->cntrl_signal_ring.elements_pending = 0;
  rb->cntrl_signal_ring.buffer = (int*) malloc(rb->cntrl_signal_ring.bufsize*sizeof(int));
  
  rb->wakeup_slave_counter = 0;
  rb->wakeup_slave_elements = autoflushInterval; // signal slave after n new elements
  
  pthread_mutex_init(&rb->elements_pending_mutex, NULL);
  
  return rb;
}

//
// Notify the reader thread
//
void log_ringbuffer_flush(struct ringbuffer_t *rb)
{
  // Flush remaining elements
  pthread_cond_signal(&rb->thread_condition); // Notify reader thread  
  
}

//
// remove ringbuffer
//
int log_ringbuffer_del(struct ringbuffer_t *rb)
{ 
  // FIXME join thread
  //pthread_join(rb->t, NULL);


  pthread_mutex_destroy(&rb->thread_mutex);
  pthread_cond_destroy(&rb->thread_condition);

  mydebug(0) printf("destroyed mutexs\n");
  
  free(rb->buf);
  mydebug(0) printf("freed buffer\n");

  free(rb);
  mydebug(0) printf("freed rb\n");
  
  free(rb->cntrl_signal_ring.buffer );
}

//
// Send a special signal to the read such as terminate signals and so on
//
void log_ringbuffer_send_signal(struct ringbuffer_t *rb, int signal)
{
  // send signal
  pthread_mutex_lock(&rb->thread_mutex);
  rb->special_signal = signal;
  pthread_mutex_unlock(&rb->thread_mutex);
  pthread_cond_signal(&rb->thread_condition);  // Notify reader thread
  
 // printf("Send signal %d\n", signal);
}

//
// write "numElementsToWrite" elements from "data" to the ringbuffer 
//
int log_ringbuffer_write(struct ringbuffer_t *rb, void *data, int numElementsToWrite)
{
  pthread_mutex_lock(&rb->thread_mutex);
  int overflow_check = rb->elements_pending == rb->num_elements;
  int freeSlots = rb->num_elements - rb->elements_pending;
  pthread_mutex_unlock(&rb->thread_mutex);

  mydebug(0) printf("writer: free slots: %d write_cnt=%d eletowrite = %d\n", freeSlots, rb->write_cnt, numElementsToWrite);
  
  if (freeSlots < numElementsToWrite) {
    mydebug(1) printf("buf full\n");
    return -1; // Buf full
  }

  // write all elements
  int i;
  for (i = 0; i < numElementsToWrite; ++i) { // for each element to be written
  
     // get current write position
    int ofs = rb->write_cnt * rb->element_size;

    mydebug(0) printf("writer: writing byte ofs=%d, \n", ofs);

    // copy one element
    void * element_ptr = data + i*rb->element_size; // calc position of src element
    memcpy( (void *) &rb->buf[ofs], element_ptr, rb->element_size);
  
    // increase write counter; take care of the end of the buffer
    rb->write_cnt++;
    if (rb->write_cnt >= rb->num_elements) // wrap to first position when end is reached
      rb->write_cnt = 0;
  
  }

  // increase elemtents pending
  pthread_mutex_lock(&rb->elements_pending_mutex);
  rb->elements_pending += numElementsToWrite; // increase number of elements that are currently stored in buffer
  pthread_mutex_unlock(&rb->elements_pending_mutex);
  
  mydebug(0) printf("writer: wakeup counter = %d\n", rb->wakeup_slave_counter);
  
  // Notify the reader thread
  if (rb->wakeup_slave_counter > rb->wakeup_slave_elements) { // is it time for a notify of the slave?
    mydebug(0) printf("writer: flush; notify reader\n");
    log_ringbuffer_flush(rb);
    mydebug(0) printf("Flush done\n");
    // pthread_cond_signal(&rb->thread_condition); // Notify reader thread
    rb->wakeup_slave_counter = 0; // reset counter
  } else
    rb->wakeup_slave_counter++;
  
    mydebug(0) printf("data written read_cnt=%d, write_cnt=%d, stored ele=%d\n", rb->read_cnt, rb->write_cnt, rb->elements_pending);
  
  return 0;
}


//
// Wait for news from the writer
//
int log_ringbuffer_avait_signal(struct ringbuffer_t *rb)
{
  //
  // Wait for signal from writer process
  //  
  
  pthread_mutex_lock(&rb->thread_mutex);

  mydebug(0) printf("log_ringbuffer_avait_signal - mutex locked\n");
  
  while (rb->elements_pending == 0 & rb->special_signal == 0) { // FIXME do { .. } while() ???
    pthread_cond_wait(&rb->thread_condition, &rb->thread_mutex);
  }
  mydebug(0) printf("log_ringbuffer_avait_signal: sigal received\n");
  int sig = rb->special_signal;
  rb->special_signal = 0; // Reset signal information - which indicates that this sigal was processed

  pthread_mutex_unlock(&rb->thread_mutex);
  
  return sig;
}

//
// Read out one element from ringbuffer either blocking or nonblocking
//
// in case of nonblocking = 0:
//   Wait for signal 
//   Returns a special signal instead of data, if there is one
// in case of nonblocking = 1:
//   Check wheter there is data
//   return write this data to *data in case there is some date
//   return immideadly in any case

int log_ringbuffer_read(struct ringbuffer_t *rb, void *data, int nonblocking)
{
  int sig = 0;
 
  
  mydebug(0) printf("log_ringbuffer_read entered, elesize=%d\n", rb->element_size);
  
  pthread_mutex_lock(&rb->elements_pending_mutex);
  int elements_pending = rb->elements_pending;
  pthread_mutex_unlock(&rb->elements_pending_mutex);

  mydebug(0) printf("ele pend %d non blocking %d\n", elements_pending, nonblocking);
  if (elements_pending == 0 && nonblocking == 1) {
    // There is nothing to read and we should not wait for something
    mydebug(0) printf("reader: there is nothing to read - return -1\n");
    return -1;
  }
  
  if (elements_pending == 0) {
    // There is nothing to read
  
    //
    // Wait for signal from writer process
    //  
    mydebug(0) printf("reader has to wait\n");
  
    if (nonblocking == 0) { // FIXME This is not needed
      sig = log_ringbuffer_avait_signal(rb);
 
      // Got a signal
      mydebug(0) printf("reader end wait\n");
  
      if (sig > 0) { // check wheter it is a special signal instead of data
        // ringbuf receinved special signal sig
	mydebug(0) printf("special signal\n");
	
        return sig;
      }
    }
  } 
  
  
  mydebug(0) printf("reader: reading. read_cnt=%d\n", rb->read_cnt);
  
  // at least one element available
  
  int memofs = rb->read_cnt * rb->element_size; // calc position in buffer
  memcpy(data, (void *) &rb->buf[memofs], rb->element_size);
  
  rb->read_cnt++;
  if (rb->read_cnt >= rb->num_elements) // read_cnt zurücksetzen
    rb->read_cnt = 0;
  

  pthread_mutex_lock(&rb->elements_pending_mutex);
  rb->elements_pending--;
  pthread_mutex_unlock(&rb->elements_pending_mutex);

  mydebug(0) printf("data read: read_cnt=%d, write_cnt=%d, stored ele=%d\n", rb->read_cnt, rb->write_cnt, rb->elements_pending);

  
  
  return 0;
}



//
// Sink infrastructure that uses the ringbuffer defined above
//

#include "realtime.h"

void *log_sink_thread(void *data)
{
  struct sink_t *sink = data;
  
  // change thread's priority to a normal one
  ortd_rt_ChangePriority(0, 0); 
  
  //printf("Sink thread started\n");
  int ret = (*sink->callback_func)(NULL, sink->callback_userdat, 1);
  if (ret != 0) {
    printf("ERROR: log.c log_sink_thread: cannot call flag 1\n");
    sink->error = -1;
    pthread_exit(NULL);
  }
  
  double testout;
  int signal = 0;
  
  mydebug(0) printf("thread ready\n");
  
  do {
    while ((signal = log_ringbuffer_read(sink->rb, sink->oneelebuf, 0)) == 0) { // Blocking read
      // FIXME: Could be done faster if log_ringbuffer_read gives back a pointer to data
      //        Instead of copying
    
      mydebug(0)  printf("Thread: got data\n");
     
      ret = (*sink->callback_func)(sink->oneelebuf, sink->callback_userdat, 2);
      
      mydebug(0) printf("testout= %f\n", testout);
    }

    // There was a control signal

    if (signal >= 2) { // 2 means flush buffers
      mydebug(0) printf("reader: control signal\n");
      // Flush buffers
      int tmp;
      while ((tmp = log_ringbuffer_read(sink->rb, sink->oneelebuf, 1)) != -1) { // Non Blocking read
	 mydebug(0) printf("flush... %d\n", tmp);
         ret = (*sink->callback_func)(sink->oneelebuf, sink->callback_userdat, 2);	 
      }
       
      switch (signal) {
	case 2:
          ret = (*sink->callback_func)(sink->oneelebuf, sink->callback_userdat, 4); // flush
	  break;
	case 3:
          ret = (*sink->callback_func)(sink->oneelebuf, sink->callback_userdat, 5); // reset
	  break;
      }
	  
    }


  } while (signal != 1); // if signal == 1 then abort
  
  //
  // Terminate
  //
  
 // printf("Terminate sink thread\n");

  // Flush buffers
  
  int tmp;
  while ((tmp = log_ringbuffer_read(sink->rb, sink->oneelebuf, 1)) != -1) { // Blocking read
    //printf("flush... %d\n", tmp);
    ret = (*sink->callback_func)(sink->oneelebuf, sink->callback_userdat, 2);	 
  }
       
  ret = (*sink->callback_func)(sink->oneelebuf, sink->callback_userdat, 4); // flush


  ret = (*sink->callback_func)(NULL, sink->callback_userdat, 3);
  
  sink->error = 0;
  pthread_exit(NULL);
}

struct sink_t *log_sink_new(int element_size, int num_elements, void *callback_func, void *callback_userdat, int numElementsToWrite)
{
  struct sink_t *sink = (struct sink_t *) malloc(sizeof(struct sink_t));
  
  mydebug(0) printf("new sink; element size =%d\n", element_size);
  
  sink->callback_func = callback_func;
  sink->rb = log_ringbuffer_new(element_size, num_elements,numElementsToWrite); // create new ringbuffer
  sink->oneelebuf = (void *) malloc(element_size);
  sink->callback_userdat = callback_userdat;
  
  int rc = pthread_create(&sink->sink_thread, NULL, log_sink_thread, (void *) sink);
  if (rc){
    printf("ERROR; return code from pthread_create() is %d\n", rc);
    return 0;
  }

  // FIXME Wait for the thread to be ready

  return sink;
}

void log_sink_flush(struct sink_t *sink)
{
  mydebug(0) printf("sink flush\n");
  log_ringbuffer_flush(sink->rb);  
  log_ringbuffer_send_signal(sink->rb, 2); // thread flush signal  
}

void log_sink_reset(struct sink_t *sink)
{
  // send reset signal to the calback function
  
  log_ringbuffer_send_signal(sink->rb, 3); // thread reset signal  
}

int log_sink_del(struct sink_t *sink)
{
  mydebug(0) printf("delete sink: flush\n");
  log_ringbuffer_flush(sink->rb); // send all cached data to reader thread

  mydebug(0) printf("delete sink: send signal\n");
  log_ringbuffer_send_signal(sink->rb, 1); // thread termination signal
  mydebug(0) printf("joining thread...\n");
  pthread_join(sink->sink_thread, NULL);
  mydebug(0) printf("joined!\n");
  
  log_ringbuffer_del(sink->rb);
  mydebug(0) printf("ringbuf deleted\n");
  free(sink);
}


//
// A threaded filewriter that uses sinks defined above
//



int log_dfilewriter_callback(void *data, void *calbdata, int flag)
{
  struct filewriter_t *fw = (struct filewriter_t *) calbdata;
  
  switch(flag) {
    case 1:
      printf("filewriter: open logfile %s\n", fw->fname);
      fw->fd = fopen ( fw->fname, "w" );
      if (fw->fd == NULL) {
	printf("ERROR: log.c: cannot open file\n");
	return -1;
      }
      return 0;
      break;
    case 2: {  
        mydebug(0) printf("dfilewriter wrinting data\n");
      
        double *vec = data;
        int i;
        if (fw->fd != NULL) {
          for (i = 0; i < fw->vlen; ++i) {
            fprintf(fw->fd, "%f ", vec[i]);
          }
          fprintf(fw->fd, "\n");
	
  	 // fflush(fw->fd);
	}
      }
      return 0;
      break;
    case 3:
      if (fw->fd != NULL) {
        printf("filewriter: close logfile %s\n", fw->fname);
        fclose(fw->fd);
      }
      return 0;
      break;
    case 5: // reset
      {
	printf("filewriter: reset\n");
	
	// Close the old file
	if (fw->fd != NULL) {
	  printf("filewriter: close logfile %s\n", fw->fname);
	  fclose(fw->fd);
	}
	
	// Rename the written file
	char new_file_name[1024 + 10];
	snprintf(new_file_name, sizeof(new_file_name), "%s.finished", fw->fname);
	if (rename(fw->fname, new_file_name) != 0) {
	  printf("filewriter: unable to rename file to %s\n", new_file_name); 
	}
	
	// Open the new file
	printf("filewriter: open logfile %s\n", fw->fname);
	fw->fd = fopen ( fw->fname, "w" );
	if (fw->fd == NULL) {
	  printf("ERROR: log.c: cannot open file\n");
	  return -1;
	}
	
	
      }
      return 0;
      break;
  }
}

struct filewriter_t *log_dfilewriter_new(int vlen, int bufsize, char *fname)
{
  struct filewriter_t *fw = (struct filewriter_t *) malloc(sizeof(struct filewriter_t));
 
  if (strlen(fname) > sizeof(fw->fname)) {
    printf("log.c dfilewriter: Filename too long\n");
    return NULL;
  }
  
  strcpy(fw->fname, fname); // FIXME: Possible Buffer overflow
  fw->sink = log_sink_new(sizeof(double)*vlen, bufsize, &log_dfilewriter_callback, (void*) fw, 20);
  if (fw->sink == 0) {
    free(fw);
    return NULL;
  }
  
  fw->vlen = vlen;
  
  return fw;
}

int log_dfilewriter_del(struct filewriter_t *fw)
{
  mydebug(0) printf("about to delete sink fw=%p\n", fw);
  log_sink_del(fw->sink);
   mydebug(0) printf("sink deleted\n");
  free(fw);
   mydebug(0) printf("dfilew deleted\n");
}

int log_dfilewriter_reset(struct filewriter_t *fw)
{
  // close the current file and rename it to *.finished
  // then open a new one
  
  log_sink_reset(fw->sink);
}

int log_dfilewriter_log(struct filewriter_t *fw, double *vec)
{
  log_ringbuffer_write(fw->sink->rb, vec, 1);
  
}




struct filewriter_t *log_scope_new(int vlen, int bufsize, char *fname)
{
  struct filewriter_t *fw = (struct filewriter_t *) malloc(sizeof(struct filewriter_t));
 
  strcpy(fw->fname, fname); // FIXME: Possible Buffer overflow
  fw->sink = log_sink_new(sizeof(double)*vlen, bufsize, &log_dfilewriter_callback, (void*) fw, 1);
  if (fw->sink == 0) {
    free(fw);
    return 0;
  }
  
  fw->vlen = vlen;
  
  return fw;
}

int log_scope_del(struct filewriter_t *fw)
{
  log_sink_del(fw->sink);
  free(fw);
}


int log_scope_log(struct filewriter_t *fw, double *vec)
{
  log_ringbuffer_write(fw->sink->rb, vec, 1);
}


//
// A threaded bindata to socket writer that uses sinks defined above
//


int log_streamtrans_callback(void *data, void *calbdata, int flag)
{
  struct streamtrans_t *st = (struct streamtrans_t *) calbdata;
  int ret;
  
  switch(flag) {
    case 1: {
      char makefifocmd[256];
      sprintf(makefifocmd, "mknod %s p", st->fname);
      ret = system(makefifocmd);
      
      printf("stream transmission: open fifo %s\n", st->fname);
      st->fd = fopen ( st->fname, "w" );
      if (st->fd == NULL) {
	printf("ERROR: log.c: cannot open file\n");
	return -1;
      }
    }
      return 0;
      break;
    case 2: {  
        char *vec = (char *) data;
        int i;
        if (st->fd != NULL) {
          fprintf(st->fd, "%c", vec[0]);
	
  	  //fflush(fw->fd);
	}
      }
      return 0;
      break;
    case 4: {  
        if (st->fd != NULL) {
          fflush(st->fd);
	}
      }
      return 0;
      break;
    case 3:
      if (st->fd != NULL) {
        printf("stream transmission: close fifo %s\n", st->fname);
        fclose(st->fd);
      }
      return 0;
      break;
  }
}

struct streamtrans_t *log_streamtrans_new(int bufsize, int numElementsToWrite, char *fname)
{
  struct streamtrans_t *st = (struct streamtrans_t *) malloc(sizeof(struct streamtrans_t));
 
  strcpy(st->fname, fname); // FIXME: Possible Buffer overflow
  st->sink = log_sink_new(sizeof(char), bufsize, &log_streamtrans_callback, (void*) st, numElementsToWrite);
  if (st->sink == 0) {
    free(st);
    return 0;
  }
  
 // printf("new streamtrans created\n");
  
  return st;
}

void log_streamtrans_flush(struct streamtrans_t *st)
{
  log_sink_flush(st->sink);
}

int log_streamtrans_del(struct streamtrans_t *st)
{
  log_sink_del(st->sink);
  free(st);
}


int log_streamtrans_log(struct streamtrans_t *st, void *bindata, int NumBytes)
// write NumBytes from bindata to the stream refered to by st
{
  log_ringbuffer_write(st->sink->rb, bindata, NumBytes);
}



