#include <stdio.h>   // printf, stderr, fprintf
#include <string.h>  // strlen, strcpy
#include <unistd.h>  // fork, pipe, dup2, close, execl
#include <stdlib.h>  // exit
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>

#include <string>

class ortd_fork {
   private:
      char *exec_path, *chpwdpath;
      
      int ToChild[2], ToParent[2]; // ToChild[0], ToParent[0] for reading and ToChild[1], ToParent[1] for writing
      FILE *readfd;
      FILE *writefd;
      
      std::string *CmdOption;
      
      int pid;
      bool replace_io;
   public:
      ortd_fork(char* exec_path, char* chpwd, int prio, bool replace_io);   // initialize the scilab path
      ~ortd_fork(); // close scilab etc...
      void addCommandlineOption(char *opt);
       
      bool init(); // starts scilab and generates pipes
      bool terminate(int sig);
      
      bool send_to_stdin(const char *cmd);  // send command to scilab
      bool send_to_stdin_noflush(const char *cmd);  // 
       
      FILE * get_readfd();    // to read from Proc
      FILE * get_writefd();
      
};
