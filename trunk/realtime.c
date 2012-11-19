#ifdef _TTY_POSIX_
  #include <time.h>
#endif


#include "stdio.h"

#ifdef __ORTD_TARGET_LINUX
// normal linux with optional rt_preempt

  #include <stdlib.h>
  #include <stdio.h>
  #include <time.h>
  #include <sched.h>
  #include <sys/mman.h>
  #include <string.h>

#else 
// RTAI, etc.

#endif


#include "realtime.h"


#ifdef _TTY_POSIX_

long int ortd_mu_time()
{

  struct timeval mytime;
  struct timezone myzone;

  gettimeofday(&mytime, &myzone);
  return (1000000*mytime.tv_sec+mytime.tv_usec);

} /* mu_time */
#endif



#ifdef __ORTD_TARGET_LINUX
// normal linux with optional rt_preempt

  #define MAX_SAFE_STACK (8*1024) /* The maximum stack size which is
				    guaranteed safe to access without
				    faulting */

  #define NSEC_PER_SEC    (1000000000) /* The number of nsecs per sec. */

  void ortd_rt_stack_prefault(void) {

	  unsigned char dummy[MAX_SAFE_STACK];

	  memset(dummy, 0, MAX_SAFE_STACK);
	  return;
  }


  int ortd_rt_ChangePriority(unsigned int flags, int priority)
  {

  //  printf("********************************************\n");
    
    if (flags & ORTD_RT_REALTIMETASK) {

      /* Declare ourself as a real time task */

        struct sched_param param;
	
        param.sched_priority = priority;
        if(sched_setscheduler(0, SCHED_FIFO, &param) == -1) {
                perror("sched_setscheduler failed");
                return(-1);
        }

        /* Lock memory */

        if(mlockall(MCL_CURRENT|MCL_FUTURE) == -1) {
                perror("mlockall failed");
                return(-2);
        }

        /* Pre-fault our stack */

        ortd_rt_stack_prefault();
	
	fprintf(stderr, "realtime.c: initialised a real-time thread\n");
	
    } else {
      
        struct sched_param param;
	
        param.sched_priority = 0;
        if(sched_setscheduler(0, SCHED_OTHER, &param) == -1) {
                perror("sched_setscheduler failed");
                return -1;
        }

	fprintf(stderr, "realtime.c: initialised a non real-time thread\n");
        
    }
    
    
  }

#else 
// RTAI, etc.

int ortd_rt_ChangePriority(unsigned int flags, int priority)
{
  printf("realtime.c: ortd_rt_ChangePriority not implemented by now\n");
}

#endif
