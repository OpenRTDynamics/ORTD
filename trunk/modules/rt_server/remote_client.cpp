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

main(int argc, char* argv[]) {

  rt_server_decoder *rd;
  
  rd = new rt_server_decoder("localhost", 10000);
  
// //   rd->read_scopes();
// //  rt_server_decoder_scope *sc = rd->get_scope_by_id(23);
//   std::string name("osc_output");
//   rt_server_decoder_scope *sc = rd->get_scope_by_name(name);
//   if (sc == NULL) {
//     printf("scope %s not found\n", name.c_str() );
//     
//     delete rd;    
// //     std::cout << "lasjl\n";
//     exit(0);
//   }
//   
//   sc->start_stream();
// 
//   double data[1000];
//   
//   bool sucess = true;
//   
//   while (sucess) {
//     sucess = sc->read_single_vec_blocking(data);
//     printf("v = %f, %f\n", data[0], data[5] );
//   }
//   
  
  if (argc >= 4) {
    
    
  
    
    int i;
    int Nvalues = argc-3;

    std::stringstream cmd;
    
    cmd << "set_param " << argv[2] << " ";

    
//     sprintf(cmd, "set_param %s # %s\n", argv[2], argv[3] );
    for (i = 0; i < Nvalues; ++i) {
//       sprintf(cmd, "set_param %s # %s\n", argv[2], argv[3] );
      cmd << " # " << argv[3+i];
    }
    cmd << "\n";
    
    char tmp[10000];
//     std::string tmp2;
    
    cmd.getline(tmp,10000);
    
    std::cout << "Sending: " << tmp << "\n";
    
    rd->send_raw_command( tmp );
    rd->await_message();
    rd->await_message();
  }

  delete rd;
  
}
