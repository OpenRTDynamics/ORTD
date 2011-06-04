#include <stdio.h>
#include "libdyn.h"
#include "libdyn_blocks.h"
#include "irpar.h"


int main()
{
  
  
  #define Ninp_ADAPT 2
  #define Noutp_ADAPT 7
  #define INsizes_ADAPT {1,1}
  #define OUTsizes_ADAPT {1,1,1,1 ,1,1,1}
  __libdyn_irpar_def_sim_vars(dyn_adapt, Ninp_ADAPT, Noutp_ADAPT);

  
  struct {
    double r;
    double y;
    
  } acc_inputs;
  
  
  double *ipar_cpy;
  int *rpar_cpy;
  int Nipar;
  int Nrpar;
  
     /*
     *  Load parameters from file
     */
    
    char *fname_i = "ldtest.ipar";
    char *fname_r = "ldtest.rpar";
  
    irpar_load_from_afile(&ipar_cpy, &rpar_cpy, &Nipar, &Nrpar, fname_i, fname_r);

  
  
  
    int err;
    struct irpar_rvec_t u1, u2;

    err = irpar_get_rvec(&u1, ipar_cpy, rpar_cpy, 920);
    err = irpar_get_rvec(&u2, ipar_cpy, rpar_cpy, 921);
    
  printf(":: Setting up Box\n----------------------\n");
  __libdyn_irpar_cfgports(dyn_adapt, Ninp_ADAPT, Noutp_ADAPT, INsizes_ADAPT, OUTsizes_ADAPT);
  __libdyn_irpar_cfg_inptr(dyn_adapt, 0, &acc_inputs.r);
  __libdyn_irpar_cfg_inptr(dyn_adapt, 1, &acc_inputs.y);
  err = __libdyn_irpar_setup(dyn_adapt, ipar_cpy, rpar_cpy, 901);

  if (err == -1) {
    printf("Error in libdyn\n");
    exit(1);
  } else {

  FILE *fd = fopen ( "ldtest_out.dat", "w" );
  
  
  printf("inptr = %x %x\n", &acc_inputs.r, &acc_inputs.y);
  
  int simlen = u1.n;
  
  int i, stepc;
  stepc = 0;
  
  for (i = 0; i < simlen*2; ++i) {
    
  int update_states  = i % 2;
  int low_freq_update = i % (2*3) == 1;
  
  
 // printf("upds = %d, low_freq_update=%d\n", update_states, low_freq_update);
    
  acc_inputs.r = u1.v[stepc];
  acc_inputs.y = u2.v[stepc];
  
  
//  if (update_states == 0)
//    printf("in(%d): %f %f\n", stepc, acc_inputs.r, acc_inputs.y);

  double a1, a2, a3, a4, a5, a6, a7; 
  
  if (update_states == 1) {
      // map scicos events to libdyn events
    int eventmask = (1 << 0) + (low_freq_update << 1);
    libdyn_event_trigger_mask(dyn_adapt.sim, eventmask);
    
    libdyn_simulation_step(dyn_adapt.sim, update_states);
  } else {
    libdyn_simulation_step(dyn_adapt.sim, update_states);  
  }
   
    
  
  
    a1 = __libdyn_irpar_getout(dyn_adapt, 0, 0);
    a2 = __libdyn_irpar_getout(dyn_adapt, 1, 0);
    a3 = __libdyn_irpar_getout(dyn_adapt, 2, 0);
    a4 = __libdyn_irpar_getout(dyn_adapt, 3, 0);
    a5 = __libdyn_irpar_getout(dyn_adapt, 4, 0);
    a6 = __libdyn_irpar_getout(dyn_adapt, 5, 0);
    a7 = __libdyn_irpar_getout(dyn_adapt, 6, 0);

//  printf("OUT port 0 PTR %x\n", (dyn_adapt).iocfg.outptr[0]);
 // printf("a1,2 = [%f, %f]\n", a1, a2);
    if (update_states == 0)
      fprintf(fd, "%f %f %f %f %f %f %f\n", a1, a2, a3, a4, a5, a6, a7);
    else
      ++stepc;
  
    
  }
  
  
  
  
  
  
  
  fclose(fd);
  libdyn_del_simulation(dyn_adapt.sim);

  }
}
