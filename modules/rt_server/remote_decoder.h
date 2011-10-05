//

#include <pthread.h>
#include <stdio.h>
#include <map>

class ortd_tcpclient;
class rt_server_decoder;

class rt_server_decoder_scope {
  public:
    rt_server_decoder_scope (rt_server_decoder* decoder);
    ~rt_server_decoder_scope ();
    
    bool read_single_vec(double *wrtite_to, bool blocking);
    
    int veclen;
    char *name;
    
  private:
    class rt_server_decoder_iohelper * myiohelper;
    rt_server_decoder* decoder;
};

class ortd_tcpclient {
  private:
    int sd;
    FILE *buffer_fd;
    
  public:
    ortd_tcpclient();
    ~ortd_tcpclient();
    
    bool tcpconnect(char *hostname, int port);
    
};

class rt_server_decoder {
  public:
    rt_server_decoder(char *hostname, int port);
    ~rt_server_decoder();

//     void init_get_scope();
    rt_server_decoder_scope * get_next_scope(); // Multiple calls return a list of all scopes

    
    char *hostname;
    int port;
    
  private:
    rt_server_decoder_iohelper *myiohelper;
    
};