#include <stdio.h>
#include "libdyn.h"
#include "libdyn_blocks.h"
//#include "libdyn.c"
//#include "libdyn_blocks.c"
//#include "irpar.c"

int compu_func_1(int flag, struct dynlib_block_t *block)
{
  printf("comp_func 1: flag==%d\n", flag);
  int Nout = 2;
  int Nin = 2;

  switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
      return 0;
      break;
    case COMPF_FLAG_UPDATESTATES:
      return 0;
      break;
    case COMPF_FLAG_CONFIGURE:  // configure
      libdyn_config_block(block, BLOCKTYPE_DYNAMIC, Nout, Nin, (void *) 0, 0);  //int block_type, int d_feedthrough, int Nout, int Nin, void *work
      libdyn_config_block_input(block, 0, 10, DATATYPE_FLOAT); // in, intype, len, datatype
      libdyn_config_block_input(block, 1, 1, DATATYPE_FLOAT);

      libdyn_config_block_output(block, 0, 10, DATATYPE_FLOAT, 0);
      libdyn_config_block_output(block, 1, 1, DATATYPE_FLOAT, 0);
      return 0;
      break;
  }
}

int compu_func_2(int flag, struct dynlib_block_t *block)
{
  printf("comp_func 2: flag==%d\n", flag);
  int Nout = 1;
  int Nin = 1;

  switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
      return 0;
      break;
    case COMPF_FLAG_UPDATESTATES:
      return 0;
      break;
    case COMPF_FLAG_CONFIGURE:  // configure
      libdyn_config_block(block, BLOCKTYPE_DYNAMIC, Nout, Nin, (void *) 0, 0);  //int block_type, int d_feedthrough, int Nout, int Nin, void *work
      libdyn_config_block_input(block, 0, 1, DATATYPE_FLOAT); // in, intype, len, datatype
     // libdyn_config_block_input(block, 1, INTYPE_DIRECT, 1, DATATYPE_FLOAT);

      libdyn_config_block_output(block, 0, 1, DATATYPE_FLOAT, 0);
//      libdyn_config_block_output(block, 1, 1, DATATYPE_FLOAT);
      return 0;
      break;
  }
}



int main()
{
  int i;
  struct dynlib_simulation_t *sim;
  double tp1_rpar = 0.7;

  //
  // Setup simulation
  //

  sim = libdyn_new_simulation();
  int event1 = libdyn_new_event(sim);
  int event2 = libdyn_new_event(sim);

  
  double const_c = 0.3;
  struct dynlib_block_t *const1 = new_const_block_(sim, &const_c);

  
  struct dynlib_block_t *block1 = libdyn_new_block(sim, &compu_func_1, 0, 0, 0,  0);
   libdyn_block_setid(block1, "Block1");
 
  struct dynlib_block_t *block2 = libdyn_new_block(sim, &compu_func_2, 0, 0, 0, 0);

  libdyn_block_connect(block2, 0, block1, 1); // connect out #0 of block2 to inp #1 of block1



  //
  // Create some blocks
  //


  struct dynlib_block_t *integrator = libdyn_new_block(sim, &compu_func_bilinearint, 1, 0, 0,  0);
  struct dynlib_block_t *integrator2 = libdyn_new_block(sim, &compu_func_bilinearint, 1, 0, 0,  0);
 // struct dynlib_block_t *tp1 = new_TP1_block(sim,&tp1_rpar); 
  struct dynlib_block_t *tp1 = new_TP1_block_(sim,0.8); //libdyn_new_block(sim, &compu_func_TP1, 0, &tp1_rpar, 0);
  double sum_c[] = {1.0, -1.0};
  struct dynlib_block_t *sum = new_sum_block_(sim, &sum_c);

  struct dynlib_block_t *sum2 = new_sum_block_(sim, &sum_c);
  

  /*
   *   0.3 + 0.6 z + 0.9 z^2
   *   ---------------------
   *   0.32 + 0.62 z + 2 z^2
   */
  double qn[] = {0.3, 0.6, 0.9};
  double qd[] = {0.32, 0.62, 2};
  struct dynlib_block_t *zTF = new_zTF_block_(sim, 2, 2, qn, qd);
  
  
  
  libdyn_block_setid(integrator, "int1");
  libdyn_block_setid(integrator2, "int2");
  libdyn_block_setid(tp1, "lowpass");
  libdyn_block_setid(sum, "sum");

  libdyn_block_setid(sum, "sum2");

  libdyn_new_event_notifier(sim, integrator, 1);
  libdyn_new_event_notifier(sim, integrator2, 1);
  libdyn_new_event_notifier(sim, tp1, 1);
  libdyn_new_event_notifier(sim, sum, 1);

  libdyn_new_event_notifier(sim, sum2, 1);
  libdyn_new_event_notifier(sim, zTF, 1);
  //libdyn_new_event_notifier(sim, integrator, 3);

  //
  // Setup clocks
  //


  libdyn_clock_event_generator_addclock(sim,1); //event 0
  libdyn_clock_event_generator_addclock(sim,1); //event 1

  //
  // Connect blocks
  //

  double insig = 1.0;
  libdyn_block_connect_external(sum, 0, &insig, 1); // connect to insig
  
  libdyn_block_connect(tp1, 0, sum, 1);
  libdyn_block_connect(integrator2, 0, tp1, 0);
  libdyn_block_connect(sum, 0, integrator2, 0);
  libdyn_block_connect(integrator2, 0, integrator, 0);

  libdyn_block_connect(integrator2, 0, sum2, 0);
  libdyn_block_connect(integrator, 0, sum2, 1);

  libdyn_block_connect(sum, 0, zTF, 0);

  //libdyn_block_connect(integrator, 0, tp1, 0); 
  //libdyn_block_connect(tp1, 0, integrator2, 0); 

  
  //
  // Print some info about blocks
  //


 // libdyn_block_dumpinfo(block1);
 // libdyn_block_dumpinfo(block2);
  libdyn_block_dumpinfo(integrator);
  libdyn_block_dumpinfo(tp1);
  libdyn_block_dumpinfo(integrator2);
  libdyn_block_dumpinfo(sum);

  libdyn_block_dumpinfo(sum2);
  libdyn_block_dumpinfo(zTF);

  //
  // Build exec list
  //
  libdyn_setup_executionlist(sim);


  //
  // Simulate
  //

  printf("\nStarting simulation\n\n");

  for (i = 0; i < 10; ++i) {

  insig = i;
  //libdyn_event_trigger(sim, 1);   // Manally trigger events
  //libdyn_event_trigger(sim, 3);

  libdyn_clock_event_generator(sim); // autogeneration of events
  libdyn_simulation_step(sim, 0);
  libdyn_simulation_step(sim, 1); // update states

  double *out = libdyn_get_output_ptr(integrator,0); //integrator->outlist[0].data;
  double *out2 = libdyn_get_output_ptr(integrator2,0); //integrator2->outlist[0].data;
  double *sumout = libdyn_get_output_ptr(sum2,0); 

  printf("sim output: int1 = %f, int2=%f, sumout=%f\n", *out,  *out2, *sumout);

  }

  libdyn_del_simulation(sim);
  
}
