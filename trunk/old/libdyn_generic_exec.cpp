#include <stdio.h>
#include <stdlib.h>
#include "libdyn_cpp.h"


extern "C" {
#include "irpar.h"
#include <getopt.h>
#include <string.h>
#include <stdlib.h>

#include <stdio.h>
#include <time.h>
#include <sched.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>
#include <pthread.h>

}


// RT stuff
#define NSEC_PER_SEC    1000000000
#define USEC_PER_SEC	1000000

static volatile int end = 0;
static double T = 0.0;

/* Options presettings */
static char rtversion[] = "0.9";
static int prio = 99;
static int verbose = 0;
static int extclock = 0;
static int wait = 0;
double FinalTime = 0.0;

double get_scicos_time()
{
  return(T);
}

static inline void tsnorm(struct timespec *ts)
{
  while (ts->tv_nsec >= NSEC_PER_SEC) {
    ts->tv_nsec -= NSEC_PER_SEC;
    ts->tv_sec++;
  }
}

static inline double calcdiff(struct timespec t1, struct timespec t2)
{
  long diff;
  diff = USEC_PER_SEC * ((int) t1.tv_sec - (int) t2.tv_sec);
  diff += ((int) t1.tv_nsec - (int) t2.tv_nsec) / 1000;
  return (1e-6*diff);
}


void *rt_task(void *p)
{
  struct timespec t, interval, curtime, T0;
  struct sched_param param;

  param.sched_priority = prio;
  if(sched_setscheduler(0, SCHED_FIFO, &param)==-1){
    perror("sched_setscheduler failed");
    exit(-1);
  }

  mlockall(MCL_CURRENT | MCL_FUTURE);

  double Tsamp = 0.1; // NAME(MODEL,_get_tsamp)();

  interval.tv_sec =  0L;
  interval.tv_nsec = (long)1e9*Tsamp;
  tsnorm(&interval);

  /* get current time */
  clock_gettime(CLOCK_MONOTONIC,&t);

  /* start after one Tsamp */
  t.tv_sec+=interval.tv_sec;
  t.tv_nsec+=interval.tv_nsec;
  tsnorm(&t);

  T0 = t;
  T=0;

 // NAME(MODEL,_init)();

  while(!end){
   /* wait untill next shot */
    if(!extclock)
      clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);

    /* Task time T */
    clock_gettime(CLOCK_MONOTONIC,&curtime);
    T = calcdiff(curtime,T0);

    /* periodic task */
  //  NAME(MODEL,_isr)(T);

    /* Check task end */
    if((FinalTime >0) && (T >= FinalTime)) pthread_exit(0);

    /* calculate next shot */
    t.tv_sec+=interval.tv_sec;
    t.tv_nsec+=interval.tv_nsec;
    tsnorm(&t);
  }
//  NAME(MODEL,_end)();  
}

void endme(int n)
{
  end = 1;
}


// end RT stuff


void usage(void)
{
	printf("Usage: libdyn_generic_exec [<options>]\n"
		" \n"
		"  --baserate <rate/ms>\n"
		"  -d <divider> new baserate divider\n"
		"  -s <name> name of schematic irpar files. .ipar and .rpar will be added to name\n"
		"  -i schematic id\n"
		"  --simlen <len> number of simulation steps. 0 is endless\n"
		" \n"
		"Example: libdyn_generic_exec --baserate 0 -d 2 -d 10 -s schematic -i 1001 --simlen 1000\n"
		"\n");
		
	exit(0);
}







int main(int argc, char *argv[])
{
  struct {
    int baserate;
    int simlen;
    
    int schematic_id;
    char schematic_fname_ipar[256];
    char schematic_fname_rpar[256];
    
    int dividers[256];
    int num_dividers;
  } args;
  
  // defaults
  args.num_dividers = 0;
  args.schematic_id = 0;
  args.baserate = 0;
  strcpy(args.schematic_fname_ipar, "generic.ipar");
  strcpy(args.schematic_fname_rpar, "generic.rpar");
  
  args.simlen = 0;
  
  {
     int idx,opt,ret;
    
   	while (1) {
		char *endptr = NULL;

		static struct option long_options[] = {
			{ "schematic_id", required_argument, 0, 'i' },
			{ "schematic", required_argument, 0, 's' },
			{ "baserate", required_argument, 0, 'b' },
			{ "divider", no_argument, 0, 'd' },
			{ "simlen", no_argument, 0, 'l' },
			{ "verbose", no_argument, NULL, 'v' },
			{ "help", no_argument, NULL, 'h' },
			{ NULL, no_argument, NULL, 0 }
		};

		opt = getopt_long(argc, argv, "s:d:i:l:b:d:vh", long_options, &idx);

		if (opt == -1)
			break;

		switch (opt) {
		case 's':
			strcpy(args.schematic_fname_ipar, optarg);
			strcat(args.schematic_fname_ipar, ".ipar");
			strcpy(args.schematic_fname_rpar, optarg);
			strcat(args.schematic_fname_rpar, ".rpar");

			printf("fnames ipar = %s\n", args.schematic_fname_ipar);
			printf("fnames rpar = %s\n", args.schematic_fname_rpar);
			break;

		case 'i':
			if (strnlen(optarg, 10) > 10) {
/*				printf("disksize_kb: %s\n", strerror(EOVERFLOW));
				ret = -EOVERFLOW;*/
				goto out;
			}

			args.schematic_id = strtoul(optarg, &endptr, 10);
			printf("Using schematic_id %d\n", args.schematic_id);
			break;


		case 'b':
			if (strnlen(optarg, 10) > 10) {
/*				printf("disksize_kb: %s\n", strerror(EOVERFLOW));
				ret = -EOVERFLOW;*/
				goto out;
			}

			args.baserate = strtoul(optarg, &endptr, 10);
			printf("Baserate set to %d\n", args.baserate);
			break;

		case 'd':
			if (strnlen(optarg, 10) > 10) {
/*				printf("disksize_kb: %s\n", strerror(EOVERFLOW));
				ret = -EOVERFLOW;*/
				goto out;
			}

			args.dividers[args.num_dividers] = strtoul(optarg, &endptr, 10);
			printf("divider #%d div = %d\n", args.num_dividers, args.dividers[args.num_dividers]);
			args.num_dividers++;
			
			break;
		case 'l':
			if (strnlen(optarg, 10) > 10) {
/*				printf("disksize_kb: %s\n", strerror(EOVERFLOW));
				ret = -EOVERFLOW;*/
				goto out;
			}

			args.simlen = strtoul(optarg, &endptr, 10);
			printf("simlen set to %d\n", args.simlen);
			break;
			


		case 'v':
		//	verbose_flag = 1;
			break;

		case 'h':
			usage();
			break;

		case '?':
			usage();
			//ret = -EINVAL;
			goto out;

		default:
			/* We never reach here */
			break;
		}
	}
	
	out:
	 ;
 
  }
  
  
  // Define sizes of in- and outports 
  int insizes[] = {1,1};
  int outsizes[] = {1,1};
 
  // make new instance of libdyn
  // 2 inports
  // 2 outports
  libdyn * simbox = new libdyn(2, insizes, 2, outsizes);
  
    printf("*1\n");
  //
  // Variables for simulation inputs
  //
  
  double inputs[2];
  double outputs[2];
  
 
  int *ipar_cpy; // pointer to integer parameter list
  double *rpar_cpy; // pointer to double parameter list
  int Nipar;  // length of ipar list
  int Nrpar;  // length of rpar list  
  
     /*
     *  Load parameters (irpar encoding) from file
     */
    
    char *fname_i = args.schematic_fname_ipar;
    char *fname_r = args.schematic_fname_rpar;
  
    int err; // variable for errors

    err = irpar_load_from_afile(&ipar_cpy, &rpar_cpy, &Nipar, &Nrpar, fname_i, fname_r);
    if (err == -1) {
      //printf("Error in libdyn\n");
      exit(1);
    }
    
    
    //
    // Get two vectors from scilab through irpar encoding
    // that will be used as input for simulation
    //
/*    
    struct irpar_rvec_t u1, u2;

    err = irpar_get_rvec(&u1, ipar_cpy, rpar_cpy, 920);
    err = irpar_get_rvec(&u2, ipar_cpy, rpar_cpy, 921);*/
    
  printf(":: Setting up Simulation\n----------------------\n");
  
  //
  // Set pointers to simulation input variables
  //
  simbox->cfg_inptr(0, &inputs[0]);
  simbox->cfg_inptr(1, &inputs[1]);

  int schematic_id = args.schematic_id; // The id under which the schematic is encoded
  err = simbox->irpar_setup(ipar_cpy, rpar_cpy, schematic_id); // compilation of schematic

  if (err == -1) {
      // There may be some problems during compilation. 
      // Errors are reported on stdout    
    printf("Error in libdyn\n");
    exit(1);
  } else {

    
  // Open a file for saving simulation output
//   FILE *fd = fopen ( "generic_out.dat", "w" );
  
  int simlen = args.simlen; // length of simulated steps is equal to the length of the input vectors
  
  int i,j, stepc;
  stepc = 0;
  
  // Pyeudo inputs  
  inputs[0] = 1.0; //u1.v[stepc];
  inputs[1] = 2.0; // u2.v[stepc];

  
  i = 0;
  do {
    //printf("step\n");
    
    int eventmask = (1 << 0) + 0;
    
    for (j=0; j < args.num_dividers; ++j) {
      if (i % args.dividers[j] == 0) {
	eventmask += (1 << j);
	//printf("ev %d\n",j);
      }
    }
      
//     printf("mask = %x\n", eventmask);
    
    simbox->event_trigger_mask(eventmask);    
    
    simbox->simulation_step(0);  // calc outputs
    simbox->simulation_step(1);  // update states
    
    if (args.baserate != 0) {
      // optional wait
    }
    
    ++i;
  } while ((i < simlen) || (simlen == 0));
  
  /*
  for (i = 0; i < simlen*2; ++i) {
    
  int update_states  = i % 2; // loop starts at update_states = 0, then
                              // alternates between 1 and 0
  int low_freq_update = i % (2*3) == 1; // frequency division by 3
  */
  
 // printf("upds = %d, low_freq_update=%d\n", update_states, low_freq_update);
  
  //
  // Set input values to the values given by the vectors u1 and u2
  //
//   inputs[0] = 1.0; //u1.v[stepc];
//   inputs[1] = 2.0; // u2.v[stepc];
// 
// 
//   
//   if (update_states == 1) {    
//     simbox->simulation_step(update_states);
//   } else {
//     // map scicos events to libdyn events
//     int eventmask = (1 << 0) +  // one event
// 		    (low_freq_update << 1);  // another event
//     simbox->event_trigger_mask(eventmask);    
//     
//     simbox->simulation_step(update_states);
//   }
//    
//     
//     //
//     // read out outputs
//     //
//   outputs[0] = simbox->get_skalar_out(0);
//   outputs[1] = simbox->get_skalar_out(1);
// 
//  // printf("a1,2 = [%f, %f]\n", a1, a2);
//   
//   // write outputs to file
//     if (update_states == 0)
//       fprintf(fd, "%f %f\n", outputs[0], outputs[1]);
//     else
//       ++stepc;
//     
//   }
  
  
  // Destruct
//   fclose(fd);
  simbox->destruct();
  }
}
