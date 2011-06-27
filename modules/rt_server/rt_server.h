//
#ifndef _RT_SERVER_H
#define _RT_SERVER_H 1


#include <pthread.h>
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
    void send_answer(rt_server *rt_server_src , char* str);
    void destruct();

    int command_id;
    char *command_name;
    
    void *userdat; // pointer to user definable data

    
    int run_callback(rt_server *rt_server_src, char *param);
    char * get_parameter_str();
};

class tcp_connection {
  public:
   tcp_connection(tcp_server* tcps, int fd);
   int fd;
   tcp_server *tcps;
   
   int readln(int nb, void *data);
   int writeln(const void* data);
   
   void destruct();
   
  private:
    FILE *bfd;
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
    int tcp_server_write(int fd, char buf[], int buflen);
    void *tcp_server_read(void *arg);
//     void loop(int listen_fd);
    tcp_connection * wait_for_connection();
    
    int port;
    
};

  /**
    * \brief Class for client handling 
    * * one rt_server instance handles one client
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
    
    rt_server_threads_manager *mymanager;

    pthread_t thread_receiver;
    
  public:
    pthread_mutex_t send_mutex;

    tcp_connection *iohelper;

    rt_server(rt_server_threads_manager *manager);
    
    // start a thread and initialise tcp server
    void set_tcp(tcp_connection *tcpc);
    void init();
    
    rt_server_command *get_commandby_id(int id);
    int parse_line(char *line);
    
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
    
    
    void loop();
    
    
  /**
    * \brief Start main thread; Returns immediately
    */
    bool start_main_loop_thread();
    
    void destruct();

    void lock_commandmap();
    // A map of commands 
    // You have to lock commmand_map via lock_commandmap and unlock_commandmap before accessing!
    command_map_t command_map;
    void unlock_commandmap();

  private:

    
    tcp_server *iohelper;
    
    int command_id_counter;
//     tcp_connection *tcpc;
    
    pthread_t mainloop_thread;
    pthread_mutex_t command_map_mutex;

    int error;
    bool mainloop_thread_started;
};

#endif