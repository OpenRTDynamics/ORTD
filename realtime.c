/*
    Copyright (C) 2012, 2013  Christian Klauer

    This file is part of OpenRTDynamics, the Real-Time Dynamics Framework

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


/*
 *  Steps to port ORTD to a new target:
 * 
 *  1) Edit the Makefile & and add a new section for your target
 *  2) Configure your new target in target.conf
 *  3) Add an #ifdef / #endif combination for your target in this
 *     file to implement the neccessary platform specific functions.
 *     (see below for a list)
 * 
 *
 * 
 *  Currently, possible targets are:
 *  __ORTD_TARGET_LINUX, __ORTD_TARGET_ANDROID, __ORTD_TARGET_RTAI (only partial support)
 * 
 *  The following functions must be defined for each target
 *  
 *  int ortd_rt_SetThreadProperties(int *par, int Npar)
 *    Set the properties of the calling thread 
 * 
 *  int ortd_rt_ChangePriority(unsigned int flags, int priority)
 *    The obsolete version of ortd_rt_SetThreadProperties
 *    may be a dummy function.
 * 
 *  long int ortd_mu_time()
 *    return the system time in microseconds
 * 
 * 
 * Please note: ORTD relies on the pthread library for creating threads.
 *              Of the target OS / RTOS doesn't support this library,
 *              an emulation of a small subset of the pthread library
 *              may help. I guess Xenomai has support for pthreads.
 * 
 *              In the long term however, all dependencies on the specific
 *              operating systems will be in this file.
 * 
 * 
 * Currently the following functions of libpthread are used:
 * 
 *                pthread_cancel
 *                pthread_cond_destroy
 *                pthread_cond_init
 *                pthread_cond_signal
 *                pthread_cond_wait
 *                pthread_create
 *                pthread_exit
 *                pthread_join
 *                pthread_kill
 *                pthread_mutex_destroy
 *                pthread_mutex_init
 *                pthread_mutex_lock
 *                pthread_mutex_trylock
 *                pthread_mutex_unlock
 *                pthread_self
 *                pthread_setaffinity_np
 *
 * 
 * 
 */

  
//  
//  Headers for the individual target systems
// 

#ifdef __ORTD_TARGET_LINUX || __ORTD_TARGET_ANDROID
// normal linux with optional rt_preempt
  #define _GNU_SOURCE
  
  #include <sched.h>

  #include <time.h>
  #include <sys/time.h>

  #include <pthread.h>
  #include <stdio.h>
  #include <stdlib.h>
  #include <errno.h>
  #include <string.h>

  #include <sys/mman.h>
  #include <pthread.h>
  #include <sys/sysinfo.h>

  
#endif
  
#ifdef __ORTD_TARGET_RTAI
// RTAI.
  #define _GNU_SOURCE
  
  #include <sched.h>

  #include <time.h>
  #include <sys/time.h>

  #include <pthread.h>
  #include <stdio.h>
  #include <stdlib.h>
  #include <errno.h>
  #include <string.h>

  #include <sys/mman.h>
  #include <pthread.h>
  #include <sys/sysinfo.h>

#endif

// Add other targets
// as needed here





// #ifdef __ORTD_TARGET_ANDROID
//   #include <time.h>
//   #include <sched.h>
//   #include <sys/mman.h>
// #endif

#include "realtime.h"





#ifdef __ORTD_TARGET_LINUX || __ORTD_TARGET_ANDROID
// normal linux with optional rt_preempt OR ANDROID



  #define MAX_SAFE_STACK (8*1024) /* The maximum stack size which is
  guaranteed safe to access without
  faulting */

  #define NSEC_PER_SEC    (1000000000) /* The number of nsecs per sec. */

  long int ortd_mu_time()
  {

      struct timeval mytime;
      struct timezone myzone;

      gettimeofday(&mytime, &myzone);
      return (1000000*mytime.tv_sec+mytime.tv_usec);

  } /* mu_time */

  int ortd_rt_SetThreadProperties(int *par, int Npar)
  {
      if (Npar >= 3) {
	  // ok got prio
	  fprintf(stderr, "Task Prio1 (flags) would be %d (1 means ORTD_RT_REALTIMETASK)\n", par[0]);
	  fprintf(stderr, "Task Prio2 would be %d\n", par[1]);
	  fprintf(stderr, "Task CPU would be %d\n", par[2]);

	  // Set CPU
	  ortd_rt_SetCore(par[2]);

	  // set the tasks priority
	  ortd_rt_ChangePriority(par[0], par[1]);

	  ortd_rt_stack_prefault();
      }

  }


  
  int ortd_rt_SetCore(int core_id) {

      if (core_id < 0)
	return;
    
      int num_cores = get_nprocs();
  //    int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
      if (core_id >= num_cores) {
	  fprintf(stderr, "WARNING: Cannot run on CPU%d because only %d CPU(s) are available!\n", core_id, num_cores);

	  return -1;
      }

      cpu_set_t cpuset;
      CPU_ZERO(&cpuset);
      CPU_SET(core_id, &cpuset);

      pthread_t current_thread = pthread_self();
      int ret = pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset);
      if (ret != 0) {
	fprintf(stderr, "WARNING: Cannot run on CPU%d, call to pthread_setaffinity_np failed!\n", core_id);
  //         handle_error_en(ret, "pthread_setaffinity_np");
      } else {
	  fprintf(stderr, "Running on CPU%d out of %d CPU(s)\n", core_id, num_cores);
      }

      return ret;
  }

  void ortd_rt_stack_prefault(void) {

      unsigned char dummy[MAX_SAFE_STACK];

      memset(dummy, 0, MAX_SAFE_STACK);
      return;
  }


  int ortd_rt_ChangePriority(unsigned int flags, int priority)
  {

      //  printf("********************************************\n");

      if (flags & ORTD_RT_REALTIMETASK) {

	  /* Declare ourself as a real-time task */

	  struct sched_param param;

	  param.sched_priority = priority;
	  if(sched_setscheduler(0, SCHED_FIFO, &param) == -1) {
	      perror("sched_setscheduler failed");
	      return(-1);
	  }


	  /* Lock memory */
  #ifdef __ORTD_TARGET_ANDROID
	  fprintf(stderr, "WARNING: mlockall is not provided by Android\n"); // Android does not provide mlockall
  #else
	  if(mlockall(MCL_CURRENT|MCL_FUTURE) == -1) {
	      perror("mlockall failed");
	      return(-2);
	  }
  #endif


	  /* Pre-fault our stack */

	  ortd_rt_stack_prefault();

	  fprintf(stderr, "realtime.c: initialised a real-time thread\n");

      } else {

	  struct sched_param param;

	  param.sched_priority = priority;
	  if(sched_setscheduler(0, SCHED_OTHER, &param) == -1) {
	      perror("sched_setscheduler failed");
	      return -1;
	  }

	  fprintf(stderr, "realtime.c: initialised a non real-time thread\n");

      }
  }

#endif
 
 
#ifdef __ORTD_TARGET_RTAI
// RTAI.


  // These are dummy entries by now
  int ortd_rt_ChangePriority(unsigned int flags, int priority)
  {
      printf("realtime.c: ortd_rt_ChangePriority not implemented by now for this target\n");
  }

  int ortd_rt_SetThreadProperties(int *par, int Npar)
  {
      printf("realtime.c: ortd_rt_SetThreadProperties not implemented by now for this target\n");
  }

  long int ortd_mu_time()
  {

      struct timeval mytime;
      struct timezone myzone;

      gettimeofday(&mytime, &myzone);
      return (1000000*mytime.tv_sec+mytime.tv_usec);

  } /* mu_time */


#endif
