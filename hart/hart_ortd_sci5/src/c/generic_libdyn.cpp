

#ifdef __cplusplus
extern "C" {
#endif

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
//#include <scicos/scicos_block.h>
#include <scicos_block4.h>

#include "irpar.h"
#include "libdyn.h"

/*

*/


#define ipar_p (&block->ipar[0])
#define rpar_p (&block->rpar[0])


#define in_p(i) ( &(GetRealInPortPtrs(block, ((i)+1) ))[0] )
#define out_p(i) ( &(GetRealOutPortPtrs(block, ((i)+1) ))[0] )
//#define out_vec ( &(GetRealOutPortPtrs(block,1))[0] )


struct generic_libdynDev {
  double test;
  
  int *ipar_cpy;
  double *rpar_cpy;
  
  struct dynlib_simulation_t *sim;
  struct libdyn_io_config_t iocfg;
  
  double *indata;
};

struct calc_stuff_t {
  double ret;
};

void generic_libdyn_bloc_calc_stuff(scicos_block *block, struct generic_libdynDev * comdev , struct calc_stuff_t *ret, int update_states) //
{
  int i,j;
  
 // printf("calc_stuff\n");
 // printf("ptr = %x rpar=[%f,%f,%f,%f]\n", rpar_p , rpar_p[0], rpar_p[1], rpar_p[2], rpar_p[3]);
  
  // update inputs
  for (i=0; i<comdev->iocfg.inports; ++i) {
    //printf("copying in #%d size %d\n", i, comdev->iocfg.insizes[i]);
    double *in = in_p(i);

    //printf("a insizes ptr = %x\n", comdev->iocfg.insizes);
    
    for (j = 0; j < comdev->iocfg.insizes[i]; ++j) {// copy in values
    /*  printf("** %x \n", comdev->iocfg.inptr);
      printf(" %f to ptr %x ;\n", 0.1, comdev->iocfg.inptr[i]); //in[j],
      printf("in[0] = %f\n", in[0]); */
      comdev->iocfg.inptr[i][j] = in[j]; 
    }
  }

  printf("next sim step\n");
  
  
  if (update_states == 1) {
    //libdyn_clock_event_generator(comdev->sim); // autogeneration of events
    
    // map scicos events to libdyn events
    int eventmask = GetNevIn(block);
    libdyn_event_trigger_mask(comdev->sim, eventmask);
  }
  
  libdyn_simulation_step(comdev->sim, update_states);

  //printf("ptr = %x rpar=[%f,%f,%f,%f]\n", rpar_p , rpar_p[0], rpar_p[1], rpar_p[2], rpar_p[3]);
  

}

int generic_libdyn_bloc_init(scicos_block *block,int flag)
{
  struct generic_libdynDev * comdev = (struct generic_libdynDev *) malloc(sizeof(struct generic_libdynDev));

//  comdev->dyn1_a = block->rpar[10];


/**
   \brief Get number of integer parameters.
*/
  int Nipar = GetNipar(block);

/**
   \brief Get pointer of the integer parameters register
*/
//GetIparPtrs(blk) 

/**
   \brief Get number of real parameters.
*/
  int Nrpar = GetNrpar(block);

/**
   \brief Get pointer of the real parameters register.
*/
//GetRparPtrs(blk) 

  int i;


  // Make a copy of r and ipar
  comdev->ipar_cpy = (int*) malloc(sizeof(int) * Nipar);
  comdev->rpar_cpy = (double*) malloc(sizeof(double) * Nrpar);
  
  for (i=0; i<Nipar; ++i)
    comdev->ipar_cpy[i] = ipar_p[i];

  for (i=0; i<Nrpar; ++i)
    comdev->rpar_cpy[i] = rpar_p[i];


  comdev->sim = libdyn_new_simulation();
  
  /*
   * Configure Port sizes in iocfg
   */ 
  
  comdev->iocfg.inports = GetNin(block);
  comdev->iocfg.outports = GetNout(block);

  printf("Nin %d, Nout %d\n", GetNin(block), GetNout(block));
  
  int *insizes = (int*) malloc(sizeof(int) * comdev->iocfg.inports);
  int *outsizes = (int*) malloc(sizeof(int) * comdev->iocfg.outports);
  
/*  
  for (i=0; i<comdev->iocfg.inports; ++i) {
    printf("insize[%d]=%d\n", i, GetInPortRows(block,i+1));
    insizes[i] = GetInPortRows(block,i+1);
  }
  for (i=0; i<comdev->iocfg.outports; ++i) {
    printf("outsize[%d]=%d\n", i, GetOutPortRows(block,i+1));
    outsizes[i] = GetOutPortRows(block,i+1);
  }
  */

  comdev->iocfg.insizes = insizes;
  comdev->iocfg.outsizes = outsizes;
  
  int sum, tmp;
  double *p;
  
  // Allocate mem for all input ports
  sum = 0;
  for (i=0; i<comdev->iocfg.inports; ++i)
    sum += comdev->iocfg.insizes[i];
  
  p = (double *) malloc(sizeof(double) * sum);
  comdev->indata = p;
  
  // List of Pointers to in vectors
  comdev->iocfg.inptr = (double **) malloc(sizeof(double *) * comdev->iocfg.inports);
  
  // Distribute mem p among all in vectors
  tmp = 0;
  for (i=0; i<comdev->iocfg.inports; ++i) {
    comdev->iocfg.inptr[i] = &p[tmp];
    tmp += comdev->iocfg.insizes[i];
  }
  
  //
  // Alloc list of pointers for outvalues comming from libdyn
  // These pointers will be set by irpar_get_libdynconnlist
  //
  
  comdev->iocfg.outptr = (double **) malloc(sizeof(double *) * comdev->iocfg.outports);

  for (i=0; i<comdev->iocfg.outports; ++i)
    comdev->iocfg.outptr[i] = (double*) 0x0;
  
  
  int err;
  
  err = irpar_get_libdynconnlist(comdev->sim, comdev->ipar_cpy, comdev->rpar_cpy, 100, &comdev->iocfg);
  if (err == -1) {
    printf("Error in irpar_get_libdynconnlist\n");
    // FIXME
  }
  
 // libdyn_block_dumpinfo(comdev->sim->allblocks_list_head->allblocks_list_next->allblocks_list_next);

  
  //
  // Build exec list
  //

  err = libdyn_setup_executionlist(comdev->sim);
  if (err == -1) {
    printf("Error in libdyn_setup_executionlist\n");
    // FIXME
  }
 
  //
  // Dump all Blocks
  //
 
  libdyn_dump_all_blocks(comdev->sim);
 


  *block->work = (void *) comdev;

  return 0;
}

int generic_libdyn_bloc_zupdate(scicos_block *block,int flag)
{
  struct generic_libdynDev * comdev = (struct generic_libdynDev *) (*block->work);

  struct calc_stuff_t ret;
  generic_libdyn_bloc_calc_stuff(block, comdev, &ret, 1);
  
  return 0;
}

int generic_libdyn_bloc_outputs(scicos_block *block,int flag)
{
  struct generic_libdynDev * comdev = (struct generic_libdynDev *) (*block->work);

  struct calc_stuff_t ret;
  generic_libdyn_bloc_calc_stuff(block, comdev, &ret, 0);

  printf("copy to outputs\n");
  int i,j;
  for (i=0; i<comdev->iocfg.outports; ++i) {
    //printf("copying in #%d size %d\n", i, comdev->iocfg.insizes[i]);
    double *out = out_p(i);

    //printf("a insizes ptr = %x\n", comdev->iocfg.insizes);

    if (comdev->iocfg.outptr[i] != 0) {

    
      for (j = 0; j < comdev->iocfg.outsizes[i]; ++j) {// copy in values
      /*  printf("** %x \n", comdev->iocfg.inptr);
      printf(" %f to ptr %x ;\n", 0.1, comdev->iocfg.inptr[i]); //in[j],
      printf("in[0] = %f\n", in[0]); */
        out[j] = comdev->iocfg.outptr[i][j]; 
      }
    
    } else {
      printf("ASSERTION FAILED: output %d not connected!\n", i);
    }

    
  }
  
  return 0;
}

int generic_libdyn_bloc_ending(scicos_block *block,int flag)
{
  struct generic_libdynDev * comdev = (struct generic_libdynDev *) (*block->work);
  
  free(comdev->iocfg.insizes);
  free(comdev->iocfg.outsizes);
  free(comdev->ipar_cpy);
  free(comdev->rpar_cpy);
  
  free(comdev->indata);
  free(comdev->iocfg.inptr);
  free(comdev->iocfg.outptr);
  
  free(comdev);

  return 0;
}


void rt_generic_libdyn(scicos_block *block,int flag)
{
 // printf("called Flag %d\n", flag);

  if (block->nevprt) { //certify that clock port was activated
   // printf("clock port aktivated\n");
  }

  if (flag == 2) { // state update
   //set_block_error(generic_libdyn_bloc_zupdate(block,flag));
   if (block->nevprt) { //certify that clock port was activated
     set_block_error(generic_libdyn_bloc_zupdate(block,flag));  // update states
   }
  }

  if (flag == 4) { /* initialization */
    generic_libdyn_bloc_init(block,flag);
  } else if(flag == 1 || flag == 6) { /* output computation*/
    set_block_error(generic_libdyn_bloc_outputs(block,flag));
  } else  if (flag == 5) { /* ending */
     set_block_error(generic_libdyn_bloc_ending(block,flag));
  }
}

#ifdef __cplusplus
} // end of extern "C" scope
#endif
