#include <stdio.h>
#include "libdyn.h"
#include "libdyn_blocks.h"
//#include "libdyn.c"
//#include "libdyn_blocks.c"
//#include "irpar.c"


int main()
{
  int ipar[] = { 

      1,    
    4,    
    200,  
    100,  
    0,    
    0,    
    2,    
    2,    
    201,  
    100,  
    2,    
    2,    
    4,    
    7,    
    202,  
    100,  
    6,    
    9,    
    2,    
    2,    
    100,  
    101,  
    8,    
    11,   
    52,   
    0,    
    12,   
    200,  
    30,   
    201,  
    2,    
    3,    
    12,   
    202,  
    0,    
    6,    
    8,    
    4,    
    1,    
    0,    
    0,    
    0,    
    0,    
    200,  
    200,  
    0,    
    0,    
    200,  
    200,  
    0,    
    0, 
    
    
      202,  
    202,  
    0,    
    0,    
    202,  
    202,  
    0,    
    0,    
    201,  
    201,  
    0,    
    0,    
    201,  
    201,  
    0,    
    0,    
    200,  
    200,  
    1,    
    0,    
    201,  
    201,  
    0,    
    0,    
    202,  
    202,  
    1,    
    0,    
    202,  
    202,  
    0,    
    1,    
    0,    
    0,    
    0    
 

    
 

   




  };
  double rpar[] = {

    
        1,  
  - 1,  
    9,  
    4,  
    3,  
    4,  
    8,  
    7,  
    2,  
    2,  
    3  
    
  };
  
  
  int i;
  struct dynlib_simulation_t *sim;

  sim = libdyn_new_simulation();

  double inp1[1];
  double inp2[1];
  
  printf("Inports %x and %x\n", inp1, inp2);
  
  double *inptrlist[2] = {inp1, inp2}; // Ein Array mit pointern auf double


  int insizes[] = {1,2};
  int outsizes[] = {1,1,1};
  
  
  
  struct libdyn_io_config_t iocfg;
  
  iocfg.inports = 2;
  iocfg.insizes = insizes;
  iocfg.outports = 3;
  iocfg.outsizes = outsizes;
  
  int sum = 0;
  for (i=0; i<iocfg.inports; ++i)
    sum += iocfg.insizes[i];
  
  printf("allocate %d in elements\n", sum);
  double *p = (double *) malloc(sizeof(double) * sum);
  iocfg.inptr = (double **) malloc(sizeof(double *) * iocfg.inports);
  
  //printf("p=%x\n", p);
  
  int tmp = 0;
  for (i=0; i<iocfg.inports; ++i) {
    iocfg.inptr[i] = &p[tmp];
    printf("- %d\n", tmp);
    tmp += iocfg.insizes[i];
   
  }
  
  
  iocfg.outptr = (double **) malloc(sizeof(double *) * iocfg.outports);
  
  
  //printf("p_=%x\n", iocfg.inptr[1]);
  
  //iocfg.inptr = inptrlist;
  
  irpar_get_libdynconnlist(sim, ipar, rpar, 100, &iocfg);

  iocfg.inptr[0][0] = 0.1;

  //
  // Build exec list
  //
  libdyn_setup_executionlist(sim);


  
  libdyn_del_simulation(sim);
  
}
