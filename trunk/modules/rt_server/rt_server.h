//
#ifndef _RT_SERVER_H
#define _RT_SERVER_H 1

#include "ortd_buffered_io.h"


#include <stdio.h>
#include <map>

#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <list>

#include <pthread.h>
#include "signal.h"



class rt_server;
class tcp_server;
class rt_server_threads_manager;
class rt_server_command;

typedef std::map<int, rt_server_command *> command_map_t;

/**
  * \brief One special command, which is registrated by the user
  *        and may be called by the clients
  */

class rt_server_command {
private:
    rt_server *rt_server_i;


    // function that will be called if the command is received by the server
    int (*callback)(rt_server_command*, rt_server *rt_server_src);

    char *paramter_str;

    rt_server_command *command_list_next;

    pthread_t thread_sender; // one sender thread for each command

    pthread_mutex_t send_condition_mutex;
    pthread_cond_t send_condition;

public:
    //rt_server_command(rt_server  * rt_server_i, char* name, int id, int* (rt_server_command*), int*    callback);

//     rt_server_command(rt_server* rt_server_i, char* name, int id, function int* (rt_server_command*), int* callback );
    rt_server_command(char* name, int id, int (*callback)(rt_server_command*, rt_server *rt_server_src) );
    int send_answer(rt_server *rt_server_src , char* str);
    void destruct();

    int command_id; // FIXME NOT USED  REMOVE
    char *command_name;

    void *userdat; // pointer to user definable data


    int run_callback(rt_server *rt_server_src, char *param);
    char * get_parameter_str();
};

class tcp_connection {
public:
    tcp_connection(tcp_server* tcps, int fd);
    int fd;  // raw fd for writing
    int raw_fdread; // raw fd for reading
    tcp_server *tcps;

    int readln(int nb, void *data);
    int writeln(const void* data);  // FIXME: Zwischen Verbindung abgebrochen und Puffer voll unterscheiden
    int writelnff(const void* data);  // FIXME: Zwischen Verbindung abgebrochen und Puffer voll unterscheiden

    FILE *get_io_fd();
    int send_flush_buffer();

    void destruct();

    void register_usage();
    void unregister_usage();

    // check for ioerror. true if there is one
    bool check_error();
    
    bool check_for_signal_while_io() { return error_number == EINTR; }

private:
//     FILE *bfd; // buffered io for writing
//     FILE *bfdread; // buffered io for reading

    int error_number; // in case of an error while read() errno is stored here

    ortd_buffered_io *bufferedio;

    // Count how many users of this class instance are pending
    // the instance can be only destroyed if this is zero
    int usage_counter;
    bool request_for_destruct;
    pthread_mutex_t used_mutex, useage_counter_mutex;
    void destruct_wait_until_unused(); // blocks until this class is not used any more

    // true if there was an ioerror
    bool error_state;   // FIXME PROTECTION
};

class tcp_server {
public:
    tcp_server(int port);
    ~tcp_server();

    volatile fd_set the_state;
    pthread_mutex_t mutex_state;

    int listen_fd;

    int tcp_server_init();
    int tcp_server_init2(int listen_fd);
//     int tcp_server_write(int fd, char buf[], int buflen);


    void *tcp_server_read(void *arg);

    tcp_connection * wait_for_connection();



    int port;

};

/**
  * \brief Class for client handling
  * * one rt_server instance handles one client in a separate thread
  */

class rt_server {
private:

    pthread_mutex_t buffer_mutex;
    pthread_cond_t thread_condition;

    /*
        rt_server_command *command_list_head;
        rt_server_command *command_list_tail;
        */

    char *rcv_buf;
    char *snd_buf;


    pthread_t thread_receiver;



public:
//     pthread_mutex_t send_mutex;

    rt_server_threads_manager *mymanager;
    tcp_connection *iohelper;

    rt_server(rt_server_threads_manager *manager);

    // start a thread and initialise tcp server
    void set_tcp(tcp_connection *tcpc);
    void init();

    rt_server_command *get_commandby_id(int id);

    // Parse the received line and run callback
    // returns -1 on parsing error
    // returns -2 on io error
    int parse_line(char *line);

    // close connection
    void hangup();

    bool get_hangup_state() {
        pthread_mutex_lock(&hangup_state_mutex);
        bool tmp = hangup_state;
        pthread_mutex_unlock(&hangup_state_mutex);

        return tmp;
    };


private:
    // if true the thread terminates on the next return from io
    // is set to true by hangup()
    // default is false;
    pthread_mutex_t hangup_state_mutex;
    bool hangup_state;

public:

    void destruct();
};


/*
 \brief Verwaltet liste mit commandos und nimmt Verbindungen entgegen
        startet neuen rt_server bei neuem Client


*/

/**
  * \brief Manages many clients by maintaining a list of rt_server instances
  *        starts its own management thread for accepting incoming network connections,
  *        which creates new rt_server instances
  */

class rt_server_threads_manager {
public:
    rt_server_threads_manager();

    /**
     * \brief Open TCP-Port
     */
    int init_tcp(int port);

    /**
      * \brief Registrates a new commmand, which can be called by clients
      * * register a command "name" with id and its callback function
      */
    void add_command(char* name, int (*callback)(rt_server_command*, rt_server *rt_server_src), void *userdat );
    bool del_command(char *name); // TODO

    void loop();


    /**
      * \brief Start main thread; Returns immediately
      */
    bool start_threads();

    // called by an instance of rt_server to request its deletion
    void hangup_request(rt_server *rt_server_i);
public:

    void destruct();

    void lock_commandmap();
    // A map of commands
    // You have to lock commmand_map via lock_commandmap and unlock_commandmap before accessing!
    command_map_t command_map;
    void unlock_commandmap();

private:


    tcp_server *iohelper;

//     tcp_connection *tcpc;

    // The main accept loop thread
    pthread_t mainloop_thread;
    bool mainloop_thread_started;
    
    // The thread that destructs rt_server instances
    pthread_t destroy_thread;
    pthread_mutex_t destroy_thread_mutex;
    pthread_cond_t destroy_thread_condition;
    rt_server * destroy_thread_signal;
    bool destroy_thread_terminate;
    static void * start_destroy_threadC(void *obj) {  ((rt_server_threads_manager *) obj)->start_destroy_thread(); }
    void start_destroy_thread();
    void destroy_thread_request(rt_server *tokill) { 
      printf("destroy_thread_request\n");
      
      pthread_mutex_lock(&destroy_thread_mutex);
      
      printf("destroy_thread_request lock passed\n");      
      
      destroy_thread_signal = tokill;
      pthread_cond_signal(&destroy_thread_condition);
      pthread_mutex_unlock(&destroy_thread_mutex);      
    }
    
    // Command list
    int command_id_counter; // FIXME NOT USED REMOVE
    pthread_mutex_t command_map_mutex;

    
    int error;

    /// client management
    void hangup_all_clients();
    
    pthread_mutex_t client_list_mutex;
    pthread_mutex_t client_list_empty_mutex; // locked by default; when unlocked no clients exist any more
    typedef std::map<rt_server *, rt_server *> client_map_t;
    client_map_t client_list;

    void lock_client_list();
    void unlock_client_list();
    void add_to_client_list(rt_server * rt_server_i);
    void wait_for_client_list_to_become_empty() {     pthread_mutex_lock(&client_list_empty_mutex); }
public:
    void del_from_client_list(rt_server * rt_server_i);

};

#endif
