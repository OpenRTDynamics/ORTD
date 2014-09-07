#include <stdio.h>   // printf, stderr, fprintf
#include <string.h>  // strlen, strcpy
#include <unistd.h>  // fork, pipe, dup2, close, execl
#include <stdlib.h>  // exit
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>

extern "C" {
#include "libdyn.h"
}

class run_scilab {
   private:
      char *scilab_path;
      int ToChild[2], ToParent[2]; // ToChild[0], ToParent[0] for reading and ToChild[1], ToParent[1] for writing
      FILE *readfd;
      FILE *writefd;
   public:
      run_scilab(const char *scilab_path);   // initialize the scilab path
      ~run_scilab(); // close scilab etc...
      
      bool init(); // starts scilab and generates pipes
      
      bool send_to_scilab(const char *cmd);  // send command to scilab
      bool send_to_scilab_noFlush(const char *cmd);  // send command to scilab
      
      bool send_to_scilab(char *buf, int size);
      
      
      FILE * get_readfd();    // read from scilab
      
};


class scilab_calculation {
   public:
      // send init_cmd an scilab
      scilab_calculation(dynlib_block_t *block, const char *scilab_path, char *init_cmd, char *destr_cmd, char *calc_cmd);//, int Nin, int Nout, int *insizes, int *outsizes, double **inptr, double **outptr);
      
      
      // send destr_cmd an scilab
      ~scilab_calculation(); // close
      bool init();
      
      /*int Nin, Nout;
      int *insizes, *outsizes;
      double **outptr, **inptr;*/
      

      bool calculate(int invec_no, int outvec_no, int insize, int outsize); // calc_cmd in scilab ausführen
      bool send_vector_to_scilab(int vector_nr, double *data, int veclen);
      bool send_buffer(char *buf, int size);
      bool read_vector_from_scilab(int vector_nr, double *data, int veclen);
      bool print_vector(int vector_nr);
      bool exit_scilab_and_read_remaining_data();
      
   private:
      run_scilab *scilab;
      char *init_cmd;
      char *destr_cmd;
      char *calc_cmd;
      
      dynlib_block_t *block;
};
