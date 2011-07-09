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

rt_server_command::rt_server_command(char* name, int id, int (*callback)(rt_server_command*, rt_server *rt_server_src) )
{
//   this->rt_server_i = rt_server_i;
  this->command_name = name;
  this->command_id = id;
  this->callback = callback;
}

void rt_server_command::send_answer(rt_server *rt_server_src , char* str)
{
//   printf("answer to cmd %s is %s\n", command_name, str);
 
//   pthread_mutex_lock(rt_server_i->send_mutex);
  
  // signal the sender thread
  
  rt_server_src->iohelper->writeln(str);
  
//   pthread_mutex_unlock(rt_server_i->send_mutex);
}

int rt_server_command::run_callback(rt_server *rt_server_src, char *param)
{
  this->paramter_str = param; // The command line for the callback within rt_server_command 

//   this->send_answer(rt_server_src, "-- BEGIN --\n");
  int calb_ret = (*this->callback)(this, rt_server_src);
  this->send_answer(rt_server_src, "--EOR--\n");
  
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
  this->tcps = tcps;
  this->fd = fd;
  this->bfd = fdopen(fd, "r+"); // use buffered io
//   printf("opened buffered io\n");
}

void tcp_connection::destruct()
{
  fflush(this->bfd);
  fclose(this->bfd);
}


int tcp_connection::readln(int nb, void* data)
{
  int buflen;
  char *rvp;  
    /* lese Meldung */
    
    
/*    buflen = read(fd, data, nb);
    if (buflen <= 0) {*/
      
    rvp = fgets((char*) data, nb, bfd);
    if (NULL == rvp) {
      
                    // Gegenseite hat Verbindung beendet oder Fehler
    
 
      
      /* End of TCP Connection */
//       printf("Tcp connection died\n");
      

      
      pthread_mutex_lock(&this->tcps->mutex_state);
      FD_CLR(fd, &this->tcps->the_state);      /* toten Client rfd entfernen */
      pthread_mutex_unlock(&this->tcps->mutex_state);
      close(fd);
      
      return -1;
    }
    
    return 1;
}

int tcp_connection::writeln(const void* data) // FIXME remove nb
{
  int ret;

/*  ret = write(fd, data, nb);
  if (ret != nb) 
    return -1;*/

  ret = fputs((char*) data, bfd);  

  if (ret < 0) {
//     printf("error writing\n");
    return -1;
  }
  
  return 1;
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
  int yes = 1;

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
  return_if(fd < 0, ERROR);

  return fd;

}

tcp_server::~tcp_server()
{
  // FIXME fill in vlose sockets etc
  close(listen_fd);
}


int tcp_server::tcp_server_write(int fd, char buf[], int buflen)
/* Schreibe auf die Client Socket Schnittstelle
 * in fd: Socket Filedescriptor zum Schreiben zum Client
 * in buf: Meldung zum Schreiben
 * in buflen: Meldungslaenge
 * return: OKAY wenn Schreiben vollstaendig, ERROR sonst
 */
{
  int ret;

  ret = write(fd, buf, buflen);
  return_if(ret != buflen, ERROR);
  return OKAY;
}





// calls accept on socket in order to do a blocking wait for a new connection
// interunption is done via signals
tcp_connection *tcp_server::wait_for_connection()
{
     int rfd;
     void *arg;

     /* TCP Server LISTEN Port (Client connect) pruefen */
    rfd = tcp_server_init2(listen_fd);
    if (rfd >= 0) {
/*      if (rfd >= MAXFD) {
        close(rfd);
        goto out;
      }*/
      pthread_mutex_lock(&mutex_state);
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
/*  snd_buf = (char *) malloc(1000);
  rcv_buf = (char *) malloc(1000);*/
  
  this->mymanager = manager;
}


// Dummy signal handling function
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
    // wait for input
    int ret = rt_server_i->iohelper->readln(sizeof(buf), buf);
    
    // check for read error or signal
    if (ret < 0)
      goto out;
    
//      parse the received line
    ret = rt_server_i->parse_line(buf);
    
    // if parsing faild return an error message to the client
    if (ret < 0)
      rt_server_i->iohelper->writeln("Command not found\n");
  }
  
  out: // break loop
  
  printf("Client disconnected\n");

  // FIXME correct ???
  delete rt_server_i;
  
  return NULL;
}

void rt_server::set_tcp(tcp_connection *tcpc)
{
  
  
  iohelper = tcpc ;
}

void rt_server::init()
{
    pthread_mutex_init(&send_mutex, NULL);
    
    int rc = pthread_create(&thread_receiver, NULL, rt_server_thread, (void *) this);

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
  
  int calb_retval = command->run_callback(this, line_parameter_part);
  return calb_retval;
  //command->
}



void rt_server::destruct()
{
  free(snd_buf);
  free(rcv_buf);
  
  pthread_mutex_destroy(&send_mutex);
}




rt_server_threads_manager::rt_server_threads_manager()
{
  command_id_counter = 0;
  pthread_mutex_init(&command_map_mutex, NULL);
  mainloop_thread_started = false;
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

  for (;;) { // FIXME how to abort?
    tcp_connection *tcpc = iohelper->wait_for_connection();
    
    printf("Wait for connect returned\n");
    
    if (tcpc == NULL) {
      printf("rt_server: Exiting thread\n"); 
      pthread_exit(NULL);
    }
    
    rt_server *rt_server_i = new rt_server(this);
    rt_server_i->set_tcp(tcpc);
    
    
    rt_server_i->init(); // the class will start its receiver thread
  }

}

bool rt_server_threads_manager::start_main_loop_thread()
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
    return true;
}

void rt_server_threads_manager::destruct()
{
  // DONE FIXME: destruct all rt_server_command instances DONE
//    TODO abort loop() running in thread


  if (mainloop_thread_started) {
    printf("rt_server: Trying to kill tcp accept thread\n");
    pthread_kill( mainloop_thread, SIGUSR1 );
    printf("rt_server: joining thread...\n");
    pthread_join( mainloop_thread, NULL );
    
    printf("rt_server: joined thread\n");
  }

/*  if (mainloop_thread_started) {
  printf("Killing mainloop_thread\n");
  pthread_cancel(mainloop_thread);
  }*/
  
  
  // FIXME Close sockets
//   iohelper->destruct();
  delete iohelper;  
  
  lock_commandmap();

  
  command_map_t::iterator iter = command_map.begin();
  
  for ( iter = command_map.begin() ; iter != command_map.end(); iter++ ) {
    rt_server_command *command = iter->second;
    command->destruct();
    delete command;
  }


  command_map.clear();
  
  unlock_commandmap();
  
  pthread_mutex_destroy(&command_map_mutex);
}

void rt_server_threads_manager::lock_commandmap()
{

}

void rt_server_threads_manager::unlock_commandmap()
{

}
