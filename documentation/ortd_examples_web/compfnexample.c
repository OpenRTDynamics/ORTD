int ortd_compu_func_modcounter(int flag, struct dynlib_block_t *block)
{
  int Nout = 1; // # input ports
  int Nin = 1; // # output ports

  int *ipar = libdyn_get_ipar_ptr(block);
  double *rpar = libdyn_get_rpar_ptr(block);
  
  int initial_state = ipar[0];
  int mod = ipar[1];
  
  int *state = (void*) libdyn_get_work_ptr(block);

  double *in;
  double *output;
  
  switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
      in= (double *) libdyn_get_input_ptr(block,0);
      output = (double *) libdyn_get_output_ptr(block,0);
      
      *output = state[0];
      
      return 0;
      break;
    case COMPF_FLAG_UPDATESTATES:
      in = (double *) libdyn_get_input_ptr(block,0);
      output = (double *) libdyn_get_output_ptr(block,0);

      if (*in > 0) {
        *state = *state + 1;
	if (*state >= mod)
	  *state = 0;
      }
      
      return 0;
      break;
    case COMPF_FLAG_RESETSTATES: // reset states
      *state = initial_state;
      
      return 0;
      break;
    case COMPF_FLAG_CONFIGURE:  // configure
      libdyn_config_block(block, BLOCKTYPE_DYNAMIC, Nout, Nin, (void *) 0, 0); 
      libdyn_config_block_input(block, 0, 1, DATATYPE_FLOAT); // in, intype, 
      libdyn_config_block_output(block, 0, 1, DATATYPE_FLOAT, 1);
      
      return 0;
      break;
    case COMPF_FLAG_INIT:  // init
      {
        int *state = malloc(sizeof(int));
        libdyn_set_work_ptr(block, (void *) state);
	*state = initial_state;
      }
      return 0;
      break;
    case COMPF_FLAG_DESTUCTOR: // destroy instance
    {
      void *buffer = (void*) libdyn_get_work_ptr(block);
      free(buffer);
    }
      return 0;
      break;
    case COMPF_FLAG_PRINTINFO:
      printf("I'm a modcounter block. initial_state = %d\n", initial_state);
      return 0;
      break;      
  }
}
