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





main() {

  rt_server_decoder *rd;
  
  rd = new rt_server_decoder("localhost", 10000);
  

//  rt_server_decoder_scope *sc = rd->get_scope_by_id(23);
  std::string name("osc_output");
  rt_server_decoder_scope *sc = rd->get_scope_by_name(name);
  if (sc == NULL) {
    printf("scope %s not found\n", name.c_str() );
    
    delete rd;    
//     std::cout << "lasjl\n";
    exit(0);
  }
  
  sc->start_stream();

  double data[1000];
  
  bool sucess = true;
  
  while (sucess) {
    sucess = sc->read_single_vec_blocking(data);
    printf("v = %f, %f\n", data[0], data[5] );
  }
  

  
  delete rd;
  
}
