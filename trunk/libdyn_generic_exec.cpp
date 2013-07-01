/*
    Copyright (C) 2010, 2011, 2012  Christian Klauer

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
//#include <sys/io.h>

#ifdef __ORTD_TARGET_ANDROID
// Declaration is missing in time.h in android NDK for some targets levels
extern int clock_settime(clockid_t, const struct timespec *);
extern int clock_nanosleep(clockid_t, int, const struct timespec *, struct timespec *);
#endif

}




// The libdyn simulation


struct global_t {
    struct {
        int baserate;
        int simlen;
        int master_tcpport;

        int schematic_id;
        char schematic_fname_ipar[256];
        char schematic_fname_rpar[256];

        int dividers[256];
        int num_dividers;

        bool mode;
    } args;

    libdyn * simbox;
    libdyn_master * master;

    double inputs[2];
    double outputs[2];


    int *ipar_cpy; // pointer to integer parameter list
    double *rpar_cpy; // pointer to double parameter list
    int Nipar;  // length of ipar list
    int Nrpar;  // length of rpar list

    int stepc;

};

int siminit(struct global_t *global_p)
{
    // set-up a new libdyn master with an optional tcpserver on a specified port
    global_p->master = new libdyn_master(RTENV_UNDECIDED, global_p->args.master_tcpport);

    fprintf(stderr, "Created new master ptr=%p\n", global_p->master); // FIXME: Remove / DEBUG

    // Define sizes of in- and outports
    int insizes[] = {1,1};
    int outsizes[] = {1,1};

    // stepcounter
    global_p->stepc = 0;

    // make new instance of libdyn
    // 2 inports
    // 2 outports
    global_p->simbox = new libdyn(2, insizes, 2, outsizes);

    // assin a master to the simulation.
    // one master can be assigned to multiple simulations
    global_p->simbox->set_master(global_p->master);

    //
    // Load parameters (irpar encoding) from file
    //

    // Filename strings
    char *fname_i = global_p->args.schematic_fname_ipar;  // *.ipar
    char *fname_r = global_p->args.schematic_fname_rpar;  // *.rpar

    int err; // variable for errors

    err = irpar_load_from_afile(&global_p->ipar_cpy, &global_p->rpar_cpy, &global_p->Nipar, &global_p->Nrpar, fname_i, fname_r);
    if (err == -1) {
        //printf("Error in libdyn\n");
        exit(1);
    }


    printf(":: Setting up Simulation\n----------------------\n");

    //
    // Set pointers to simulation input variables
    //

    global_p->simbox->cfg_inptr(0, &global_p->inputs[0]); // Input 0
    global_p->simbox->cfg_inptr(1, &global_p->inputs[1]); // Input 1

    //
    // Set-up and compile schematic
    //

    int schematic_id = global_p->args.schematic_id; // The id under which the schematic is encoded
    err = global_p->simbox->irpar_setup(global_p->ipar_cpy, global_p->rpar_cpy, schematic_id); // compilation of schematic

    if (err == -1) {
        // There may be some problems during compilation.
        // Errors are reported on stdout
        printf("Error in libdyn\n");
        exit(1);
    }


}

int simperiodic(struct global_t *global_p)
{
    int simlen = global_p->args.simlen; // length of simulated steps is equal to the length of the input vectors

    int j;

    // Pseudo inputs
    global_p->inputs[0] = 1.0; //u1.v[stepc];
    global_p->inputs[1] = 2.0; // u2.v[stepc];


    // set event mask
    int eventmask = (1 << 0) + 0;

    for (j=0; j < global_p->args.num_dividers; ++j) {
        if (global_p->stepc % global_p->args.dividers[j] == 0) {
            eventmask += (1 << j);
        }
    }

    global_p->simbox->event_trigger_mask(eventmask);

    // run one simulation step including output calculation and state update
    global_p->simbox->simulation_step(0);  // calc outputs
    global_p->simbox->simulation_step(1);  // update states

    if (global_p->args.baserate != 0) {
        // optional wait
    }

    ++global_p->stepc;



    if ((global_p->stepc < simlen) || (simlen == 0)) {
        return 0;
    } else {
        printf("Maximum number of simulation steps reached\n");
        return -1;
    }


}

int simend(struct global_t *global_p)
{
  printf("libdyn_generic_exec: Destructing the simulations\n");
  
    global_p->simbox->destruct();
    delete global_p->simbox;

    global_p->master->destruct();
    delete global_p->master;
}



//
// RT stuff
//

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
    struct global_t *global_p = (struct global_t *) p;

    struct timespec t, interval, curtime, T0;
    struct sched_param param;

    if (global_p->args.mode == 0) {

        param.sched_priority = prio;
        if (sched_setscheduler(0, SCHED_FIFO, &param)==-1) {
            perror("sched_setscheduler failed");
            fprintf(stderr, "Running without RT-Preemption\n");
//         exit(-1);
        }
    } else {
        fprintf(stderr, "Running without RT-Preemption\n");
    }

#ifdef __ORTD_TARGET_ANDROID
    fprintf(stderr, "WARNING: mlockall is not provided by Android\n"); // Android does not provide mlockall    
#else
    mlockall(MCL_CURRENT | MCL_FUTURE);
#endif
    
    double Tsamp = global_p->args.baserate * 0.001; // NAME(MODEL,_get_tsamp)();

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
    siminit(global_p);

    while (!end) {
        /* wait untill next shot */
        if (!extclock)
            clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);

        /* Task time T */
        clock_gettime(CLOCK_MONOTONIC,&curtime);
        T = calcdiff(curtime,T0);

        /* periodic task */
        //  NAME(MODEL,_isr)(T);
        if ( simperiodic(global_p) < 0) {
	  end = 1;
	}

        /* Check task end */
        if ((FinalTime >0) && (T >= FinalTime)) pthread_exit(0);

        /* calculate next shot */
        t.tv_sec+=interval.tv_sec;
        t.tv_nsec+=interval.tv_nsec;
        tsnorm(&t);
    }
//  NAME(MODEL,_end)();
    simend(global_p);
}

void endme(int n)
{
  printf("libdyn_generic_exec: Received signal to terminate\n");
    end = 1;    
}


// end RT stuff


void usage(void)
{
    printf("Usage: libdyn_generic_exec [<options>]\n"
           " \n"
           "  --baserate <rate/ms> A value of zero means as fast as possible (simulation mode)\n"
           "                       For baserates != 0 real-time execution using\n"
           "                       the given rate will be used.\n"
           "  -d <divider> new baserate divider\n"
           "  -s <name> name of schematic irpar files. .ipar and .rpar will be added to name\n"
           "  -i schematic id\n"
           "  -l <len> number of simulation steps. 0 means endless\n"
           "  -p / --master_tcpport <port> the portnumber of the remote control interface; default is 0,\n"
           "                               which means no remote control is enabled\n"
	   "  -m / --rtmode <val>  Set the real-time mode to <val>. 0 (default) means get real-time priority whenever\n"
	   "                       possible, 1 means to run with normal priority\n"
           " \n"
           "Example: libdyn_generic_exec --baserate 0 -s schematic -i 901 -l 1000\n"
           "         this will load schematic.ipar and schematic.rpar and simulate 1000 steps\n"
           "\n");

    exit(0);
}

int main(int argc, char *argv[])
{
    struct global_t global;
    struct global_t *global_p = &global;

    // defaults
    global_p->args.num_dividers = 0;
    global_p->args.schematic_id = 901;
    global_p->args.baserate = 0;
    global_p->args.master_tcpport = 0;
    global_p->args.mode = 0;
    strcpy(global_p->args.schematic_fname_ipar, "generic.ipar");
    strcpy(global_p->args.schematic_fname_rpar, "generic.rpar");

    global_p->args.simlen = 0;

    {
        int idx,opt,ret;

        while (1) {
            char *endptr = NULL;

            static struct option long_options[] = {
                { "schematic_id", required_argument, 0, 'i' },
                { "schematic", required_argument, 0, 's' },
                { "baserate", required_argument, 0, 'b' },
                { "divider", required_argument, 0, 'd' },
                { "simlen", required_argument, 0, 'l' },
                { "master_tcpport", required_argument, 0, 'p' },
                { "rtmode", required_argument, 0, 'm' },
                { "verbose", no_argument, NULL, 'v' },
                { "help", no_argument, NULL, 'h' },
                { NULL, no_argument, NULL, 0 }
            };

            opt = getopt_long(argc, argv, "s:d:i:l:p:b:m:d:vh", long_options, &idx);

            if (opt == -1)
                break;

            switch (opt) {
            case 's':
                strcpy(global_p->args.schematic_fname_ipar, optarg);
                strcat(global_p->args.schematic_fname_ipar, ".ipar");
                strcpy(global_p->args.schematic_fname_rpar, optarg);
                strcat(global_p->args.schematic_fname_rpar, ".rpar");

                printf("fnames ipar = %s\n", global_p->args.schematic_fname_ipar);
                printf("fnames rpar = %s\n", global_p->args.schematic_fname_rpar);
                break;

            case 'i':
                if (strnlen(optarg, 10) > 10) {
                    /*				printf("disksize_kb: %s\n", strerror(EOVERFLOW));
                    				ret = -EOVERFLOW;*/
                    goto out;
                }

                global_p->args.schematic_id = strtoul(optarg, &endptr, 10);
                printf("Using schematic_id %d\n", global_p->args.schematic_id);
                break;


            case 'b':
                if (strnlen(optarg, 10) > 10) {
                    /*				printf("disksize_kb: %s\n", strerror(EOVERFLOW));
                    				ret = -EOVERFLOW;*/
                    goto out;
                }

                global_p->args.baserate = strtoul(optarg, &endptr, 10);
                printf("Baserate set to %d\n", global_p->args.baserate);
                break;

            case 'd':
                if (strnlen(optarg, 10) > 10) {
                    /*				printf("disksize_kb: %s\n", strerror(EOVERFLOW));
                    				ret = -EOVERFLOW;*/
                    goto out;
                }

                global_p->args.dividers[global_p->args.num_dividers] = strtoul(optarg, &endptr, 10);
                printf("added new divider #%d div = %d\n", global_p->args.num_dividers, global_p->args.dividers[global_p->args.num_dividers]);
                global_p->args.num_dividers++;

                break;
            case 'l':
                if (strnlen(optarg, 10) > 10) {
                    /*				printf("disksize_kb: %s\n", strerror(EOVERFLOW));
                    				ret = -EOVERFLOW;*/
                    goto out;
                }

                global_p->args.simlen = strtoul(optarg, &endptr, 10);
                printf("simlen set to %d\n", global_p->args.simlen);
                break;
            case 'p':
                if (strnlen(optarg, 10) > 10) {
                    goto out;
                }

                global_p->args.master_tcpport = strtoul(optarg, &endptr, 10);
                printf("master_tcpport set to %d\n", global_p->args.master_tcpport);
                break;

            case 'm':
                if (strnlen(optarg, 10) > 10) {
                    goto out;
                }
                global_p->args.mode = strtoul(optarg, &endptr, 10);	        
                printf("RT mode set to %d\n", global_p->args.mode);
                
		
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

    if (global_p->args.baserate > 0) {
        // realtime run
        pthread_t thrd;
        pthread_attr_t t_att;
        int ap;


        signal(SIGINT,endme);
        signal(SIGKILL,endme);

     //   iopl(3);

        ap=pthread_create(&thrd,NULL,rt_task,global_p);

        pthread_join(thrd,NULL);
    } else {
        // run as fast as possible
        siminit(global_p);
        if (global_p->args.simlen != 0) {
            int i;
            for (i=0; i<global_p->args.simlen; ++i) {
                simperiodic(global_p);
            }
        } else {
            for (;;) // FIXME
                simperiodic(global_p);
        }
        simend(global_p);
    }
}
