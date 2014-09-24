#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

#include "remote_decoder.h"


#include <iostream> 

/*
  USAGE: ./ortd_client setpar stim_v1 0.12
*/

int main(int argc, char* argv[]) {

  rt_server_decoder *rd;
  
  rd = new rt_server_decoder("localhost", 10000);
  

  
  if (argc >= 4) {
    
    
  
    
    int i;
    int Nvalues = argc-3;

    std::stringstream cmd;
    
    cmd << "set_param " << argv[2] << " ";

    
    for (i = 0; i < Nvalues; ++i) {
      cmd << " # " << argv[3+i];
    }
    cmd << "\n";
    
    char tmp[10000];
//     std::string tmp2;
    
    cmd.getline(tmp,10000);
    
    std::cout << "Sending: " << tmp << "\n";
    
    rd->send_raw_command( tmp );
    rd->send_raw_command( "\n" ); // terminate the last line
    rd->await_message();
    rd->await_message();
  }

  delete rd;
  
}
