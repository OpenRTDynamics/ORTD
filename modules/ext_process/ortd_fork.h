#include <stdio.h>   // printf, stderr, fprintf
#include <string.h>  // strlen, strcpy
#include <unistd.h>  // fork, pipe, dup2, close, execl
#include <stdlib.h>  // exit
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>


class ortd_fork {
   private:
      char *exec_path;
      int ToChild[2], ToParent[2]; // ToChild[0], ToParent[0] for reading and ToChild[1], ToParent[1] for writing
      FILE *readfd;
      FILE *writefd;
      
      int pid;
      bool replace_io;
   public:
      ortd_fork(const char* exec_path, bool replace_io);   // initialize the scilab path
      ~ortd_fork(); // close scilab etc...
      
      bool init(); // starts scilab and generates pipes
      bool terminate(int sig);
      
      bool send_to_stdin(const char *cmd);  // send command to scilab
      
      FILE * get_readfd();    // read from scilab
      
};
