/*
    Copyright (C) 2010, 2011  Christian Klauer

    This file is part of OpenRTDynamics, the Real Time Dynamic Toolbox

    OpenRTDynamics is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenRTDynamics is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with OpenRTDynamics.  If not, see <http://www.gnu.org/licenses/>.
*/

#define DEBUG 0

#include "rt_server.h"
#include "malloc.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// #include <pthread.h>
// #include <bits/sigthread.h>
// #include <bits/pthreadtypes.h>
// #include <signal.h>

#include <unistd.h>
#include <bits/pthreadtypes.h>
#include <pthread.h>
#include <string.h>
#include "signal.h"
#include <stdio.h>
#include <iostream>
#include <sstream>


rt_server_command::rt_server_command(char* name, int id, int (*callback)(rt_server_command*, rt_server *rt_server_src) )
{
//   this->rt_server_i = rt_server_i;
    this->command_name = name;
    this->command_id = id;
    this->callback = callback;
}

int rt_server_command::send_answer(rt_server *rt_server_src , char* str)
{
// FIXME: remove this function

    // signal the sender thread

    int ret = rt_server_src->iohelper->writeln(str);

    return ret;
}

int rt_server_command::run_callback(rt_server *rt_server_src, char *param)
{
    this->paramter_str = param; // The command line for the callback within rt_server_command

//   this->send_answer(rt_server_src, "-- BEGIN --\n");
    int calb_ret = (*this->callback)(this, rt_server_src);
//    this->send_answer(rt_server_src, "--EOR--\n");

    if (DEBUG==1)      printf("rt_server: callback returned\n");


    // send end of direct command output
    rt_server_src->iohelper->writeln("--EOR--\n");

    if (DEBUG==1)      printf("rt_server: --EOR-- was send\n");


    // flush the send buffers, so the client gets the data immediately
    rt_server_src->iohelper->send_flush_buffer();

    if (DEBUG==1)      printf("rt_server: buffers were flushed\n");


    return calb_ret;
}

char* rt_server_command::get_parameter_str()
{
    return paramter_str;
}


void rt_server_command::destruct()
{

}





tcp_connection::tcp_connection(tcp_server* tcps, int fd)
{
    this->error_number = 0;
    this->error_state = false;

    this->usage_counter = 0;

    this->tcps = tcps;
    this->fd = fd;

    bufferedio = new ortd_buffered_io(fd);



    request_for_destruct = false;
    usage_counter = 0;
    pthread_mutex_init(&useage_counter_mutex, NULL);
    pthread_mutex_init(&used_mutex, NULL);


}

void tcp_connection::destruct_wait_until_unused()
{
    // This will block until this instance is ready for destruction
    pthread_mutex_lock(&used_mutex); // initially locked
}


void tcp_connection::register_usage()
{
    pthread_mutex_lock(&useage_counter_mutex);

    usage_counter++;
    if (usage_counter == 1) {
//       printf("tcp_connection instance locked\n");
        pthread_mutex_lock(&used_mutex); // this should always pass
    }


    pthread_mutex_unlock(&useage_counter_mutex);
}

void tcp_connection::unregister_usage()
{
    pthread_mutex_lock(&useage_counter_mutex);

    usage_counter--;

    if (usage_counter == 0) { //  && request_for_destruct) {
//         printf("tcp_connection instance unlocked\n");
        pthread_mutex_unlock(&used_mutex); // destruct can continue
    }

    pthread_mutex_unlock(&useage_counter_mutex);
}

bool tcp_connection::check_error()
{
    return error_state;
}


void tcp_connection::destruct()
{
    printf("waiting to be ready for destruction...\n");
    destruct_wait_until_unused(); // blocks until this class is not used any more
    printf("tcp_connection now unused\n");

//     // close buffered io
//     fclose(bfd);
//     fclose(bfdread);

    // remove mutexes
    pthread_mutex_destroy(&useage_counter_mutex);
    pthread_mutex_destroy(&used_mutex);

    delete bufferedio;
}


int tcp_connection::readln(int nb, void* data)
{
    int buflen;
    char *rvp;

    if (error_state == true)
        return -1;

    if (DEBUG==1)      printf("rt_server: waiting for message from the client\n");


    /* lese Meldung */
    int ret = bufferedio->waitforaline();
    if (ret < 0) {
        // Gegenseite hat Verbindung beendet oder Fehler
        printf("Tcp connection died\n");
        error_state = true;
	error_number = errno;

        return -1;
    }

    std::string input_line;
    getline(bufferedio->sstream_in, input_line);

//     std::cout << "iohelper got " << input_line;

    if (strlen(input_line.c_str()) >= nb)
        return -1;

    strcpy((char*) data, (char*) input_line.c_str() );



    if (DEBUG==1)      printf("rt_server: read returned\n");

    return 1;
}

int tcp_connection::writeln(const void* data)
{
    int ret;


    if (error_state == true)
        return -1;


    if (DEBUG==1) printf("rt_server: write to tcp..\n");

    ret = bufferedio->writeln((char*) data);
    if (ret < 0) {
        if (DEBUG==1) printf("error writing\n");
        error_state = true;
        return -1;
    }

    if (DEBUG==1) printf("done\n");

    return 1;
}

int tcp_connection::writelnff(const void* data)
{

    this->writeln(data);
}



FILE* tcp_connection::get_io_fd() // FIXME OBSOLETE
{
//     return bfd;
    return NULL;
}


int tcp_connection::send_flush_buffer()
{
//     if (ferror(bfd) < 0 )
//         printf("er\n");
//
//     if (DEBUG==1) printf("rt_server: flushing buffers\n");
//
//     fflush(bfd);
//
//     if (DEBUG==1) printf("rt_server: flush done\n");


}


/* Die Makros exit_if() und return_if() realisieren das Error Handling
 * der Applikation. Wenn die exit_if() Bedingung wahr ist, wird
 * das Programm mit Fehlerhinweis Datei: Zeile: Funktion: errno beendet.
 * Wenn die return_if() Bedingung wahr ist, wird die aktuelle Funktion
 * beendet. Dabei wird der als Parameter 2 angegebene Returnwert benutzt.
 */

#define exit_if(expr) \
if(expr) { \
  syslog(LOG_WARNING, "exit_if() %s: %d: %s: Error %s\n", \
  __FILE__, __LINE__, __PRETTY_FUNCTION__, strerror(errno)); \
  exit(1); \
}

#define return_if(expr, retvalue) \
if(expr) { \
  syslog(LOG_WARNING, "return_if() %s: %d: %s: Error %s\n\n", \
  __FILE__, __LINE__, __PRETTY_FUNCTION__, strerror(errno)); \
  return(retvalue); \
}

#define MAXLEN 1024
#define MAXFD 20

#define OKAY 0
#define ERROR (-1)


tcp_server::tcp_server(int port)
{
    this->port = port;
}

int tcp_server::tcp_server_init()
{
    int listen_fd;
    int ret;
    struct sockaddr_in sock;
    int yes = 1; // FIXME Constant???

    listen_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0)
        goto error;
//   exit_if(listen_fd < 0);

    /* vermeide "Error Address already in use" Fehlermeldung */
    ret = setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    if (ret < 0)
        goto error;
//   exit_if(ret < 0);

    memset((char *) &sock, 0, sizeof(sock));
    sock.sin_family = AF_INET;
    sock.sin_addr.s_addr = htonl(INADDR_ANY);
    sock.sin_port = htons(port);

    ret = bind(listen_fd, (struct sockaddr *) &sock, sizeof(sock));
    if (ret < 0)
        goto error;
//   exit_if(ret != 0);

    ret = listen(listen_fd, 5);
    if (ret < 0)
        goto error;
//   exit_if(ret < 0);

    this->listen_fd = listen_fd;

    return listen_fd;


error:
    fprintf(stderr, "Error opening socket\n");

    return -1;
}

int tcp_server::tcp_server_init2(int listen_fd)
/* communication (connection) oeffnen - fuer jeden neuen client ausfuehren
 * in listen_fd: Socket Filedescriptor zum Verbindungsaufbau vom Client
 * return: wenn okay Socket Filedescriptor zum lesen vom Client, ERROR sonst
 */
{
    int fd;
    struct sockaddr_in sock;
    socklen_t socklen;

    socklen = sizeof(sock);
    fd = accept(listen_fd, (struct sockaddr *) &sock, &socklen);
//     return_if(fd < 0, ERROR);

    return fd;

}

tcp_server::~tcp_server()
{
    // FIXME fill in vlose sockets etc
    close(listen_fd);
}







// calls accept on socket in order to do a blocking wait for a new connection
// interunption is done via signals
tcp_connection *tcp_server::wait_for_connection()
{
    int rfd;
    void *arg;

    /* TCP Server LISTEN Port (Client connect) pruefen */

    // Accept connection
    rfd = tcp_server_init2(listen_fd); // accept
    if (rfd >= 0) {
        /*      if (rfd >= MAXFD) {
                close(rfd);
                goto out;
              }*/
        pthread_mutex_lock(&mutex_state); // FIXME : Not needed?
        FD_SET(rfd, &the_state);        /* neuen Client fd dazu */
        pthread_mutex_unlock(&mutex_state);

        tcp_connection *tcpc = new tcp_connection(this, rfd);
        //tcpc->fd = rfd;

        return tcpc;
    }

out:
    return NULL;   // Got signal
}


///////////////////////////////////////////

rt_server::rt_server(rt_server_threads_manager *manager)
{
    this->mymanager = manager;

    hangup_state = false;
}


// Dummy signal handling function called when rt_server should hangup the connection
void *rt_server_gotsig(int sig, siginfo_t *info, void *ucontext)
{
    return NULL;
}


// One thread per client.
void *rt_server_thread(void *data)
{
    rt_server *rt_server_i = (rt_server *) data;

//
//   Install signal handler
//
    int hdlsig = (int)SIGUSR1;

    struct sigaction sa;
    sa.sa_handler = NULL;
    sa.sa_sigaction = (void (*)(int, siginfo_t*, void*)) rt_server_gotsig;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);

    if (sigaction(hdlsig, &sa, NULL) < 0) {
        perror("sigaction");
        pthread_exit(NULL);
    }

//
// Start main readout loop
//

    char buf[1024]; // The line buffer

    for (;;) {
        rt_server_i->iohelper->register_usage(); // Tell that the iohelper is in use

        // wait for input
        int ret = rt_server_i->iohelper->readln(sizeof(buf), buf);

        if (DEBUG==1) printf("rt_server: got command\n");


        // check for read error or signal
        if (ret < 0)
            goto ioerror;



//      parse the received line and run the apropriate callback
        ret = rt_server_i->parse_line(buf);

        if (DEBUG==1) printf("rt_server: line parsed\n");

        // if parsing faild return an error message to the client
        if (ret == -1) {
            if ( rt_server_i->iohelper->writelnff("Command not found\n") < 0) {
                // Error while writing to the client.
                goto ioerror;
            }
        }

        if (ret < 0 && rt_server_i->iohelper->check_error() ) {
            if (DEBUG==1) printf("rt_server: main loop detected ioerror\n");

            goto ioerror;
        }


        rt_server_i->iohelper->unregister_usage();// Tell that the iohelper is not used any more

    }

ioerror: // break loop

    /*
     *  Client disconnected, clean up
     */



    printf("Client disconnected (or an hangup request)\n");
    //sleep(2);
    
    bool thread_signaled = rt_server_i->iohelper->check_for_signal_while_io();

    rt_server_i->iohelper->unregister_usage();// Tell that the iohelper is not used any more

    
    
    
    
    
    
    
      //rt_server_i->mymanager->hangup_request(rt_server_i); 

      
    


        //     Remove from client list
        rt_server_i->mymanager->del_from_client_list(rt_server_i);


        // FIXME This SHOULD to be done by the rt_server_thread_manager instance
        // This ONLY works because this function is not a method of rt_server_i
        // but an independent C function. Therefore, it is possible to destruct
        // rt_server_o

        printf("destructing rt_server instance\n");
        rt_server_i->destruct();
        delete rt_server_i;


    return NULL; // by now this thread will not be joint by someone
}

void rt_server::set_tcp(tcp_connection *tcpc)
{


    iohelper = tcpc ;
}

void rt_server::init()
{
//     pthread_mutex_init(&send_mutex, NULL);
    pthread_mutex_init(&hangup_state_mutex, NULL);

    int rc = pthread_create(&thread_receiver, NULL, rt_server_thread, (void *) this);

    //pthread_detach(thread_receiver); // FIXME !!!!

    if (rc)
    {
        printf("rt_server: ERROR; return code from pthread_create() is %d\n", rc);
        //return -1;
    }
}

rt_server_command * rt_server::get_commandby_id(int id)
{

    mymanager->lock_commandmap();

    command_map_t::iterator iter = mymanager->command_map.find(id);

    if (iter != mymanager->command_map.end()) {
        mymanager->unlock_commandmap();

        rt_server_command *command = iter->second;

        return command;
    } else {
        mymanager->unlock_commandmap();

        return NULL;
    }

}

int rt_server::parse_line(char* line)
{
//   printf("parse line = <%s>\n", line);
    if (DEBUG==1) printf("rt_server: parsing line\n");


    rt_server_command *command;
    char *line_parameter_part;

    // search command instance

    mymanager->lock_commandmap();

    command_map_t::iterator iter = mymanager->command_map.begin();

    for ( iter = mymanager->command_map.begin() ; iter != mymanager->command_map.end(); iter++ ) {
        command = iter->second;

//     printf("testing for command %s\n", command->command_name);
        char *ret = strstr(line, command->command_name);

        if (ret == line) { // string an erster stelle gefunden
//       printf("found!\n");
            line_parameter_part = line + strlen(command->command_name);

            goto foundcmd;
        }
    }

    mymanager->unlock_commandmap();
    return -1; // command not found

foundcmd:
    // command
    mymanager->unlock_commandmap();

    //
    // Run the  callback for the command_id
    //

    if (DEBUG==1) printf("rt_server: running callback\n");


    int calb_retval = command->run_callback(this, line_parameter_part);

    if (DEBUG==1) printf("rt_server: callback returned\n");

    return calb_retval;
    //command->
}

void rt_server::hangup()
{
    pthread_mutex_lock(&hangup_state_mutex);
    hangup_state = true;
    pthread_mutex_unlock(&hangup_state_mutex);

    printf("rt_server: Trying to close connection\n");

    // signal the thread, so it returns from a potetial io operation
    pthread_kill( thread_receiver, SIGUSR1 );
}




void rt_server::destruct()
{
    iohelper->destruct();
    delete iohelper;

    pthread_mutex_destroy(&hangup_state_mutex);
}




rt_server_threads_manager::rt_server_threads_manager()
{
    command_id_counter = 0;
    pthread_mutex_init(&command_map_mutex, NULL);
    mainloop_thread_started = false;

    pthread_mutex_init(&client_list_mutex, NULL);
    pthread_mutex_init(&client_list_empty_mutex, NULL);
//    pthread_mutex_init(&hangup_request_state_mutex, NULL);
    
    // The thread that destructs rt_server instances
    destroy_thread_terminate = false;
    pthread_mutex_init(&destroy_thread_mutex, NULL);
    pthread_cond_init(&destroy_thread_condition, NULL);


}

int rt_server_threads_manager::init_tcp(int port)
{
    iohelper =  new tcp_server(port);
    int ret = iohelper->tcp_server_init();

    this->error = ret;

    return ret;
//   printf("tcp started\n");

}

// when the callback is called userdat will be available within cmd->userdat
void rt_server_threads_manager::add_command(char* name, int (*callback)(rt_server_command*, rt_server *rt_server_src), void *userdat)
{
    lock_commandmap();

    rt_server_command *cmd = new rt_server_command(name, command_id_counter, callback);
    cmd->userdat = userdat;

    command_map.insert(std::make_pair(command_id_counter, cmd));
    command_id_counter++;

    unlock_commandmap();
}



void *rt_server_thread_mainloop(void *data)
{
    rt_server_threads_manager *rtsthmgr = (rt_server_threads_manager *) data;

//   printf("Thread started\n");

    rtsthmgr->loop();

    return NULL;
}

// Dummy signal handling function
void *rt_server_thread_gotsig(int sig, siginfo_t *info, void *ucontext)
{
    return NULL;
}


void rt_server_threads_manager::loop()
{
//
//   Install signal handler
//
    int hdlsig = (int)SIGUSR1;

    struct sigaction sa;
    sa.sa_handler = NULL;
    sa.sa_sigaction = (void (*)(int, siginfo_t*, void*)) rt_server_thread_gotsig;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);

    if (sigaction(hdlsig, &sa, NULL) < 0) {
        perror("sigaction");
//         return (void*) -1;
        pthread_exit(NULL);
    }


//
//    Start loop, which accepts incoming connectionns
//

    for (;;) { // accept connections
        tcp_connection *tcpc = iohelper->wait_for_connection();

        printf("Wait for connect returned\n");

        if (tcpc == NULL) {
            // in case of a signal to this thread

//              rt_server *tokill = mainloop_rendevous_check();
//              if (tokill != NULL) {
//                 printf("Request to kill an rt_server instance\n");
// 
// 		//     Remove from client list
//                 tokill->mymanager->del_from_client_list(tokill);
// 
//                 tokill->destruct();
//                 delete tokill;

//             } else {
                // means termination of the accept thread
                printf("rt_server: closing all connections\n");

                hangup_all_clients();

                printf("rt_server: Exiting accept thread\n");
                pthread_exit(NULL);
//             }

            // finish the transaction, initiated by the signaling thread
//             mainloop_rendevous_finish();
        } else {

            // start new client
            rt_server *rt_server_i = new rt_server(this);
            rt_server_i->set_tcp(tcpc);
            rt_server_i->init(); // the class will start its receiver thread

            add_to_client_list(rt_server_i); // Store the new connection into a list
        }
    }

}

void rt_server_threads_manager::start_destroy_thread()
{
  // Loop that destroy rt_server instances
  
  printf("rt_Sever destroy thread started\n");
  
  // just exit because this thread is not used at the moment
  return;
  
  // This is never executed
  pthread_mutex_lock(&destroy_thread_mutex);
  for (;;) {


        while (destroy_thread_signal == NULL && !destroy_thread_terminate) {
            pthread_cond_wait(&destroy_thread_condition, &destroy_thread_mutex);
        }
        
        if (destroy_thread_terminate) {
	  pthread_exit(NULL);
	}
        
        rt_server *tokill = destroy_thread_signal;
        destroy_thread_signal = NULL;

        printf("destroy_thread: request for a termination\n");

	    //pthread_join( tokill->thread_receiver, NULL ); 
	
		//     Remove from client list
                tokill->mymanager->del_from_client_list(tokill);

                tokill->destruct();
                delete tokill;

        printf("destroy_thread: rt_server was terminated\n");
  
  }
  
  pthread_mutex_unlock(&destroy_thread_mutex);
  sleep(10);
}


bool rt_server_threads_manager::start_threads()
{
//     pthread_mutex_init(&send_mutex, NULL);
//     printf("starting rt_server therad\n");

    // if there was a previous error
    if (this->error < 0)
        return false;

    int rc = pthread_create(&mainloop_thread, NULL, rt_server_thread_mainloop, (void *) this);
    if (rc)
    {
        printf("rt_server: ERROR; return code from pthread_create() is %d\n", rc);
        this->error = -1;
        return false;
    }

    mainloop_thread_started = true;
    
    // The destroy thread
    destroy_thread_signal = NULL;
    
    rc = pthread_create(&destroy_thread, NULL, start_destroy_threadC, (void *) this);
    if (rc)
    {
        printf("rt_server: ERROR; return code from pthread_create() is %d\n", rc);
        this->error = -1;
        return false;
    }
    
    return true;
}

void rt_server_threads_manager::lock_client_list()
{
    pthread_mutex_lock(&client_list_mutex);
}

void rt_server_threads_manager::unlock_client_list()
{
    pthread_mutex_unlock(&client_list_mutex);
}

void rt_server_threads_manager::hangup_request(rt_server* rt_server_i)
{
    // ring the accept loop, and initialise the destruction of rt_server_i, including the joining of the rt_server thread
    printf("hangup request\n");
//     mainlook_rendevous_start(rt_server_i);

    destroy_thread_request(rt_server_i);
    
    // from here this is an atomar transaction that is finished by get_hangup_request_state()
    // in the main accept loop



}

void rt_server_threads_manager::hangup_all_clients()
{
    // Kill all clients within client_list

    lock_client_list();


    client_map_t::iterator iter; // = client_list.begin();

    for (iter = client_list.begin(); iter != client_list.end(); iter++) {
        rt_server *client = iter->second;

        printf("Terminating rt_server thread... \n");

        unlock_client_list();
        client->hangup();
        lock_client_list();

        printf("done\n");
    }

    unlock_client_list();

    // FIXME warten bis alle client threads beendet wurden. NOTE möglicherweise wird schon in hangup() gewartet
    printf("rt_server_threads_manager: Waiting for all threads to terminate...\n");
    wait_for_client_list_to_become_empty();
    printf("rt_server_threads_manager: All threads were terminated\n");
}

void rt_server_threads_manager::add_to_client_list(rt_server* rt_server_i)
{
    pthread_mutex_lock(&client_list_mutex);
//   client_list.insert(client_list.end(), rt_server_i);
    client_list.insert(std::make_pair(rt_server_i, rt_server_i) );
    pthread_mutex_trylock(&client_list_empty_mutex);
    pthread_mutex_unlock(&client_list_mutex);
}

void rt_server_threads_manager::del_from_client_list(rt_server* rt_server_i)
{
    pthread_mutex_lock(&client_list_mutex);
//   client_list.erase();
    client_map_t::iterator it = client_list.find(rt_server_i);
    client_list.erase(it);
    if (client_list.empty()) {
      pthread_mutex_unlock(&client_list_empty_mutex);    
    }
    pthread_mutex_unlock(&client_list_mutex);


}


void rt_server_threads_manager::destruct()
{
    // DONE FIXME: destruct all rt_server_command instances DONE


    if (mainloop_thread_started) {
        printf("rt_server: Trying to kill tcp accept thread\n");
        pthread_kill( mainloop_thread, SIGUSR1 );
//        this->mainlook_rendevous_start(NULL); // signal the main loop thread for its termination


        printf("rt_server: joining thread...\n");


        pthread_join( mainloop_thread, NULL );

        printf("rt_server: joined thread\n");
    }


    //
    // Kill all client handlers
    //

    // Cleap clint list
    client_list.clear();

    // FIXME Close sockets; Was ist das
    printf("delete something\n");
//   iohelper->destruct();
    delete iohelper;

    //
    // Destruct all commands
    //

    lock_commandmap();
    {
        command_map_t::iterator iter = command_map.begin();

        for ( iter = command_map.begin() ; iter != command_map.end(); iter++ ) {
            rt_server_command *command = iter->second;
            command->destruct();
            delete command;
        }

        command_map.clear();
    }
    unlock_commandmap();

    pthread_mutex_destroy(&command_map_mutex);
    pthread_mutex_destroy(&client_list_mutex);
    pthread_mutex_destroy(&client_list_empty_mutex);
//     pthread_mutex_destroy(&hangup_request_state_mutex);
    
    // The thread that destructs rt_server instances
    pthread_mutex_destroy(&destroy_thread_mutex);
    pthread_cond_destroy(&destroy_thread_condition);
    
}

void rt_server_threads_manager::lock_commandmap()
{
    pthread_mutex_lock(&command_map_mutex);
}

void rt_server_threads_manager::unlock_commandmap()
{
    pthread_mutex_unlock(&command_map_mutex);
}
