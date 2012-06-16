#ifndef REMOTE_DECODER_H
#define REMOTE_DECODER_H

#include <pthread.h>
#include <stdio.h>
#include <map>
#include <sstream>
#include "qt_event_communication.h"

class qt_event_communication;
class rt_server_decoder;


class rt_server_decoder_scope {
  public:
    rt_server_decoder_scope (rt_server_decoder* decoder, std::string name);
    ~rt_server_decoder_scope ();
    
    void enqueue_data(double *data);

// BEGIN changed by Arne
    // bool read_single_vec_blocking(double *wrtite_to);
    void setInfo(int veclen, int datatype, int StreamId);
    void start_stream();
    void stop_stream();
    void read_data(double* wrtite_to);
// ENDchanged by Arne
    
    std::string name;
    
  private:
    
//     std::queue<std::string> dataqueue;
    
/*    ortd_tcpclient *io;
    ortd_buffered_io *bio;*/
    rt_server_decoder* decoder;
    int veclen; int datatype; int StreamId;
    
    void *databuffer;
};

class rt_server_decoder
{
  public:
    
 // BEGIN changed by Arne
    rt_server_decoder(char *hostname, int port, qt_event_communication* qt_event_com);

    bool send_raw_command(char *cmd); // returns ALWAYS true

    rt_server_decoder_scope * get_scope_by_id(int id);

    // bool read_scopes();

    bool read_scope(std::string input_line); // true if all scopes are read

    bool read_scope_info(std::string input_line); // true if all scopes infos are read

    void get_scopes_infos();

    void init_sending(); // if the tcp client is connected this function is called

    // Wait for a message and parse the received line
    //bool await_message(); // TODO Diese funktion raus und durch folgende ersetzten:

    // If the Qt event loop receives a line from the network this function should be called to process that data.
    // returns FAILURE if a wrong state occured
    // returns MORE_SCOPES_TO_READ if there are more scopes to be read
    // returns MORE_SCOPES_INFOS_TO_READ if all scopes are read, but there are more scopes infos to be read
    // returns ALL_SCOPES_WITH_INFOS_READ if all scopes and their infos are read
    // returns SCOPE_DATA_ARRIVED if a scope was updated, NOTHING_HAPPENED if not
    int feed_external_data(std::string DataFromTcpLine);
    // user calls this function if feed_external_data returns SCOPE_DATA_ARRIVED, so he is informed about which scoped data has changed
    rt_server_decoder_scope *GetTheScopeThatWasChanged();

    // the line was read somewhere else. It is only parsed
    int parse_line(std::string input_line); // returns SCOPE_DATA_ARRIVED if a scope was updated, NOTHING_HAPPENED if not

 // END changed by Arne

    ~rt_server_decoder();

//     void init_get_scope();
    rt_server_decoder_scope * get_next_scope(); // Multiple calls return a list of all scopes


    rt_server_decoder_scope * get_scope_by_name(std::string name);

    
    char *hostname;
    int port;


private:
    
    typedef std::map<int, rt_server_decoder_scope *> scope_map;
    scope_map scopes;
 // BEGIN changed by Arne
    int state;
    std::list<std::string> scope_names;
    int scope_infos_to_get;
    int changed_scope_id; // is -1 if no scope was changed
    qt_event_communication* qt_event_com;
 // END changed by Arne
    
};

#endif // REMOTE_DECODER_H
