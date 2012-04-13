/* Code prototype for standalone use  */
/*     Generated by Code_Generation toolbox of Scicos with scilab-4.1.2 */
/*     date : 12-Apr-2012 */

/* ---- Headers ---- */
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <memory.h>
#include <scicos_block4.h>
#include <machine.h>

#ifdef linux
#define __CONST__
#else
#define __CONST__ static const
#endif

double rtai_generic_exec_get_tsamp()
{
  return(0.1);
}

double rtai_generic_exec_get_tsamp_delay()

{
  return(0);
}

/* ---- Clock code ---- */
int rtai_generic_exec_useInternTimer(void) {
	return 1;
}

void rtextclk(void) { }

/* ---- Internals functions declaration ---- */
int rtai_generic_exec_init(void);
int rtai_generic_exec_isr(double);
int rtai_generic_exec_end(void);

/* prototype of 'rt_hart_libdyn' (type 4) */
void rt_hart_libdyn(scicos_block *, int );

/* prototype of 'bidon' (type 0) */
void C2F(bidon)(int *, int *, double *, double *, double *, int *, double *, \
                int *, double *, int *, double *, int *,int *, int *, \
                double *, int *, double *, int *);


/* Some general static variables */
static double zero=0;
static double w[1];
void **rtai_generic_exec_block_outtbptr;

scicos_block block_rtai_generic_exec[2];

double RPAR[1];

#ifdef linux
int NRPAR = 0;
int NTOTRPAR = 0;
char * strRPAR;
int lenRPAR[1] = {0};
#endif

/* def integer parameters */
__CONST__ int IPAR[ ] = {
/* Routine name of block: rt_hart_libdyn
 * Gui name of block: hart_libdyn
 * Compiled structure index: 1
 * Exprs: []
 * Identification: 
ipar= {1,1,9000,1,900,0,0,0,0,0,9,109,97,105,110,95,111,114,116,100};
*/
1,1,9000,1,900,0,0,0,0,0,9,109,97,105,110,95,111,114,116,100,
/* Routine name of block: bidon
 * Gui name of block: EVTGEN_f
 * Compiled structure index: 2
 * Exprs: 1
 * Identification: 
ipar= {1};
*/
1,
};

#ifdef linux
int NIPAR = 2;
int NTOTIPAR = 21;
char * strIPAR[2] = {"IPARAM[1]","IPARAM[2]"};
int lenIPAR[2] = {20,1};
#endif


  /* Initial values */

  /* Note that z[]=[z_initial_condition;outtbptr;work]
     z_initial_condition={0};
     outtbptr={};
     work= {0,0};
  */

  static double z[]={0,0,0};


double rtai_generic_exec_1_evout[1];

/*----------------------------------------  Initialisation function */
int rtai_generic_exec_init()
{
  double t;
  int local_flag;


 /* Routine name of block: rt_hart_libdyn
    Gui name of block: hart_libdyn
   Compiled structure index: 1
   Exprs: []
   z={0};
 */
  /* Get work ptr of blocks */
  void **work;
  work = (void **)(z+1);

  /* set blk struc. of 'rt_hart_libdyn' (type 4 - blk nb 1) */
  block_rtai_generic_exec[0].type   = 4;
  block_rtai_generic_exec[0].ztyp   = 0;
  block_rtai_generic_exec[0].ng     = 0;
  block_rtai_generic_exec[0].nz     = 1;
  block_rtai_generic_exec[0].noz    = 0;
  block_rtai_generic_exec[0].nrpar  = 0;
  block_rtai_generic_exec[0].nopar  = 0;
  block_rtai_generic_exec[0].nipar  = 20;
  block_rtai_generic_exec[0].nin    = 0;
  block_rtai_generic_exec[0].nout   = 0;
  block_rtai_generic_exec[0].nevout = 0;
  block_rtai_generic_exec[0].nmode  = 0;
  block_rtai_generic_exec[0].z = &(z[0]);
  block_rtai_generic_exec[0].ipar=&(IPAR[0]);
  block_rtai_generic_exec[0].work = (void **)(((double *)work)+0);
  /* set blk struc. of 'bidon' (type 0 - blk nb 2) */
  block_rtai_generic_exec[1].type   = 0;
  block_rtai_generic_exec[1].ztyp   = 0;
  block_rtai_generic_exec[1].ng     = 0;
  block_rtai_generic_exec[1].nz     = 0;
  block_rtai_generic_exec[1].noz    = 0;
  block_rtai_generic_exec[1].nrpar  = 0;
  block_rtai_generic_exec[1].nopar  = 0;
  block_rtai_generic_exec[1].nipar  = 1;
  block_rtai_generic_exec[1].nin    = 0;
  block_rtai_generic_exec[1].nout   = 0;
  block_rtai_generic_exec[1].nevout = 1;
  block_rtai_generic_exec[1].nmode  = 0;
  block_rtai_generic_exec[1].evout = rtai_generic_exec_1_evout;
  block_rtai_generic_exec[1].z = &(z[1]);
  block_rtai_generic_exec[1].work = (void **)(((double *)work)+1);
   /* Initialization */

  /* Call of 'rt_hart_libdyn' (type 4 - blk nb 1) */
  block_rtai_generic_exec[0].nevprt = 0;
  local_flag = 4;
  rt_hart_libdyn(&block_rtai_generic_exec[0],local_flag);
  return(local_flag);
}

/*----------------------------------------  ISR function */
int rtai_generic_exec_isr(double t)
{
  int local_flag;
  int i;
  /* Output computation */
  /* Discrete activations */
  /* Blocks activated on the event number 1 */
      /* Call of 'rt_hart_libdyn' (type 4 - blk nb 1) */
      block_rtai_generic_exec[0].nevprt = 1;
      local_flag = 1;
      rt_hart_libdyn(&block_rtai_generic_exec[0],local_flag);
  

  /* Discrete state computation */
  /* Discrete activations */
  /* Blocks activated on the event number 1 */
      /* Call of 'rt_hart_libdyn' (type 4 - blk nb 1) */
      block_rtai_generic_exec[0].nevprt = 1;
      local_flag = 2;
      rt_hart_libdyn(&block_rtai_generic_exec[0],local_flag);
  


  return 0;
}
/*----------------------------------------  Termination function */
int rtai_generic_exec_end()
{
  double t;
  int local_flag;

  /* Ending */

  /* Call of 'rt_hart_libdyn' (type 4 - blk nb 1) */
  block_rtai_generic_exec[0].nevprt = 0;
  local_flag = 5;
  rt_hart_libdyn(&block_rtai_generic_exec[0],local_flag);
  return 0;
}

