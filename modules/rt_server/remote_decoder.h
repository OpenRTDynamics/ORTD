//

#include <pthread.h>
#include <stdio.h>
#include <map>
#include <sstream>
#include "ortd_buffered_io.h"

class ortd_tcpclient;
class rt_server_decoder;



class ortd_tcpclient {
  private:
    int sd;
    int fd;
    
    FILE *buffer_fd;
    
  public:
    
    ortd_tcpclient();
    ~ortd_tcpclient();
    
    bool tcpconnect(char *hostname, int port);
    
    int get_fd();
    
    void closeconnection();
    
};


class rt_server_decoder_scope {
  public:
    rt_server_decoder_scope (rt_server_decoder* decoder, std::string name);
    ~rt_server_decoder_scope ();
    
    void setInfo(int veclen, int datatype, int StreamId);
    void start_stream();
    void stop_stream();
    
    bool read_single_vec_blocking(double *wrtite_to);
    void enqueue_data(double *data);
    
    std::string name;
    
  private:
    
//     std::queue<std::string> dataqueue;
    
/*    ortd_tcpclient *io;
    ortd_buffered_io *bio;*/
    rt_server_decoder* decoder;
    int veclen; int datatype; int StreamId;
    
    void *databuffer;
};

class rt_server_decoder {
  public:
    rt_server_decoder(char *hostname, int port);
    ~rt_server_decoder();

//     void init_get_scope();
    rt_server_decoder_scope * get_next_scope(); // Multiple calls return a list of all scopes

    rt_server_decoder_scope * get_scope_by_id(int id);
    rt_server_decoder_scope * get_scope_by_name(std::string name);
    
    bool send_raw_command(char *cmd);
    
    bool read_scopes();
    bool await_message();
    
    char *hostname;
    int port;

    ortd_tcpclient *io;
    ortd_buffered_io *bio;
    
  private:
    
    typedef std::map<int, rt_server_decoder_scope *> scope_map;
    scope_map scopes;

    
};