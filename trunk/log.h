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

#ifndef _ORTD_LOG_H
#define _ORTD_LOG_H 1


#include <pthread.h>
//#include <stdlib.h>
#include <stdio.h>

/*
 * A ringbuffer using mutexes
 */

struct ringbuffer_t {
  char *buf;
  int element_size;
  int num_elements;
  
  int write_cnt; // index to the next free write position
  int read_cnt; // index to element that can be read
  int elements_pending; // number of elements in the buffer. Protected by elements_pending_mutex

  pthread_mutex_t thread_mutex;
  pthread_cond_t thread_condition;
  int thread_command;
  
  int special_signal; // buffer for a special information for the reader thread
  
  pthread_mutex_t elements_pending_mutex;
  
  int wakeup_slave_elements; // wakeup readerthread after "wakeup_slave_elements"
  int wakeup_slave_counter; // counter for mechanism above
  
};

extern struct ringbuffer_t *log_ringbuffer_new(int element_size, int num_elements, int autoflushInterval);
extern void log_ringbuffer_flush(struct ringbuffer_t *rb);
extern int log_ringbuffer_del(struct ringbuffer_t *rb);
extern int log_ringbuffer_write(struct ringbuffer_t *rb, void *data, int numElements);
extern int log_ringbuffer_read(struct ringbuffer_t *rb, void *data, int nonblocking);

/*
 * A Sink that calls a userdefined callback for data storage in a separate thread
 */

struct sink_t {
  struct ringbuffer_t *rb;
  int (*callback_func)(void *data, void *callback_userdat, int flag);
  pthread_t sink_thread;
  void *oneelebuf;
  void *callback_userdat;
  
  int error;
};


struct sink_t *log_sink_new(int element_size, int num_elements, void *callback_func, void *callback_userdat, int numElementsToWrite);
void log_sink_flush(struct sink_t *sink);
int log_sink_del(struct sink_t *sink);


/*
 * A datalogger that writes doubles to files
 */

struct filewriter_t {
  FILE *fd;
  char fname[256];
  struct sink_t *sink;
  int vlen;
};

struct filewriter_t *log_dfilewriter_new(int vlen, int bufsize, char *fname);
int log_dfilewriter_del(struct filewriter_t *fw);
int log_dfilewriter_log(struct filewriter_t *fw, double *vec);



//
// Send binary stream to socket
//


struct streamtrans_t {
  FILE *fd;
  char fname[256];
  struct sink_t *sink;
};

struct streamtrans_t *log_streamtrans_new(int bufsize, int numElementsToWrite, char *fname);
void log_streamtrans_flush(struct streamtrans_t *st);
int log_streamtrans_del(struct streamtrans_t *st);
int log_streamtrans_log(struct streamtrans_t *st, void *bindata, int NumBytes);

#endif