#include <stdio.h>
#include <stdlib.h>
#include "libdyn_cpp.h"

extern "C" {
#include "irpar.h"
}


int main()
{
  // Define sizes of in- and outports 
  int insizes[] = {1,1};
  int outsizes[] = {1,1};
 
  // make new instance of libdyn
  // 2 inports
  // 2 outports
  libdyn * simbox = new libdyn(2, insizes, 2, outsizes);
  
  
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
    
    char *fname_i = "generic.ipar";
    char *fname_r = "generic.rpar";
  
    irpar_load_from_afile(&ipar_cpy, &rpar_cpy, &Nipar, &Nrpar, fname_i, fname_r);

    int err; // variable for errors
    
    //
    // Get two vectors from scilab through irpar encoding
    // that will be used as input for simulation
    //
    
    struct irpar_rvec_t u1, u2;

    err = irpar_get_rvec(&u1, ipar_cpy, rpar_cpy, 920);
    err = irpar_get_rvec(&u2, ipar_cpy, rpar_cpy, 921);
    
  printf(":: Setting up Box\n----------------------\n");
  
  //
  // Set pointers to simulation input variables
  //
  simbox->cfg_inptr(0, &inputs[0]);
  simbox->cfg_inptr(1, &inputs[1]);

  int schematic_id = 901; // The id under which the schematic is encoded
  err = simbox->irpar_setup(ipar_cpy, rpar_cpy, schematic_id); // compilation of schematic

  if (err == -1) {
      // There may be some problems during compilation. 
      // Errors are reported on stdout    
    printf("Error in libdyn\n");
    exit(1);
  } else {

    
  // Open a file for saving simulation output
  FILE *fd = fopen ( "generic_out.dat", "w" );
  
  int simlen = u1.n; // length of simulated steps is equal to the length of the input vectors
  
  int i, stepc;
  stepc = 0;
  
  for (i = 0; i < simlen*2; ++i) {
    
  int update_states  = i % 2; // loop starts at update_states = 0, then
                              // alternates between 1 and 0
  int low_freq_update = i % (2*3) == 1; // frequency division by 3
  
  
 // printf("upds = %d, low_freq_update=%d\n", update_states, low_freq_update);
  
  //
  // Set input values to the values given by the vectors u1 and u2
  //
  inputs[0] = u1.v[stepc];
  inputs[1] = u2.v[stepc];
  

  
  if (update_states == 1) {    
    simbox->simulation_step(update_states);
  } else {
    // map scicos events to libdyn events
    int eventmask = (1 << 0) +  // one event
		    (low_freq_update << 1);  // another event
    simbox->event_trigger_mask(eventmask);    
    
    simbox->simulation_step(update_states);
  }
   
    
    //
    // read out outputs
    //
  outputs[0] = simbox->get_skalar_out(0);
  outputs[1] = simbox->get_skalar_out(1);

 // printf("a1,2 = [%f, %f]\n", a1, a2);
  
  // write outputs to file
    if (update_states == 0)
      fprintf(fd, "%f %f\n", outputs[0], outputs[1]);
    else
      ++stepc;
    
  }
  
  
  // Destruct
  fclose(fd);
  simbox->destruct();
  }
}
