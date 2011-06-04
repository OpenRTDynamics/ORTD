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

rt_server_command::rt_server_command(char* name, int id, int (*callback)(rt_server_command*, rt_server *rt_server_src) )
{
//   this->rt_server_i = rt_server_i;
  this->command_name = name;
  this->command_id = id;
  this->callback = callback;
}

void rt_server_command::send_answer(rt_server *rt_server_src , char* str)
{
  printf("answer to cmd %s is %s\n", command_name, str);
 
//   pthread_mutex_lock(rt_server_i->send_mutex);
  
  // signal the sender thread
  
  rt_server_src->iohelper->writeln(str);
  
//   pthread_mutex_unlock(rt_server_i->send_mutex);
}

int rt_server_command::run_callback(rt_server *rt_server_i, char *param)
{
  this->paramter_str = param;
  return (*this->callback)(this, rt_server_i);
}


void rt_server_command::destruct()
{

}


tcp_connection::tcp_connection(tcp_server* tcps, int fd)
{
  this->tcps = tcps;
  this->fd = fd;
  this->bfd = fdopen(fd, "r+"); // use buffered io
  printf("opened buffered io\n");
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
      printf("Tcp connection died\n");
      

      
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
    printf("error writing\n");
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
  exit_if(listen_fd < 0);

  /* vermeide "Error Address already in use" Fehlermeldung */
  ret = setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  exit_if(ret < 0);

  memset((char *) &sock, 0, sizeof(sock));
  sock.sin_family = AF_INET;
  sock.sin_addr.s_addr = htonl(INADDR_ANY);
  sock.sin_port = htons(port);

  ret = bind(listen_fd, (struct sockaddr *) &sock, sizeof(sock));
  exit_if(ret != 0);

  ret = listen(listen_fd, 5);
  exit_if(ret < 0);

  this->listen_fd = listen_fd;
  
  return listen_fd;
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

void* tcp_server::tcp_server_read(void* arg)
/* Thread fuer einen CONNECT Port
 * Lese von der Client Socket Schnittstelle, schreibe an alle anderen Clients
 * in arg: Socket Filedescriptor zum lesen vom Client
 * return:
 */
{
  int rfd;
  char buf[MAXLEN];
  int buflen;
  int wfd;
  
  rfd = (int)arg;
  for(;;) {
    /* lese Meldung */
    buflen = read(rfd, buf, sizeof(buf));
    if (buflen <= 0) {
      /* End of TCP Connection */
      pthread_mutex_lock(&mutex_state);
      FD_CLR(rfd, &the_state);      /* toten Client rfd entfernen */
      pthread_mutex_unlock(&mutex_state);
      close(rfd);
      pthread_exit(NULL);
    }
    
    /* Meldung an alle anderen Clients schreiben */
    pthread_mutex_lock(&mutex_state);
    for (wfd = 3; wfd < MAXFD; ++wfd) {
      if (FD_ISSET(wfd, &the_state) && (rfd != wfd)) {
        tcp_server_write(wfd, buf, buflen);
      }
    }
    pthread_mutex_unlock(&mutex_state);
  }
  return NULL;

}

void tcp_server::loop(int listen_fd)
/* Server Endlosschleife - accept
 * in listen_fd: Socket Filedescriptor zum Verbindungsaufbau vom Client
 */
{
  pthread_t threads[MAXFD];
  
  FD_ZERO(&the_state);
  
//   for (;;) {                    /* Endlosschleife */
//     int rfd;
//     void *arg;
//     
//     /* TCP Server LISTEN Port (Client connect) pruefen */
//     rfd = tcp_server_init2(listen_fd);
//     if (rfd >= 0) {
//       if (rfd >= MAXFD) {
//         close(rfd);
//         continue;
//       }
//       pthread_mutex_lock(&mutex_state);
//       FD_SET(rfd, &the_state);        /* neuen Client fd dazu */
//       pthread_mutex_unlock(&mutex_state);
//       arg = (void *) rfd;
//       pthread_create(&threads[rfd], NULL, tcp_server_read, arg);
//     }
//   }

}

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
     return 0;
}


///////////////////////////////////////////

rt_server::rt_server(rt_server_threads_manager *manager)
{
/*  snd_buf = (char *) malloc(1000);
  rcv_buf = (char *) malloc(1000);*/
  
  this->mymanager = manager;
}

// One thread per client.
void *rt_server_thread(void *data)
{
  rt_server *rt_server_i = (rt_server *) data;
  
  
  printf("Thread started\n");
  
  char buf[256];
  
  for (;;) {
    int ret = rt_server_i->iohelper->readln(sizeof(buf), buf);
    
    if (ret < 0)
      goto out;
    
    if (ret > 0) {
      printf("received %s\n", buf);
    }
    
    ret = rt_server_i->parse_line(buf);
    if (ret < 0)
      rt_server_i->iohelper->writeln("Command not found\n");
    
    // write something
    //rt_server_i->iohelper->writeln(buf);
   
  }
  
  out:
  
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
   	  command_map_t::iterator iter = mymanager->command_map.find(id);
 	
 	  if (iter != mymanager->command_map.end()) {
 	    rt_server_command *command = iter->second;
	    
	    return command;
	  } else {
	    return NULL;
	  }

}

int rt_server::parse_line(char* line)
{
//   printf("parse line = <%s>\n", line);
  rt_server_command *command;

  // search command instance
  command_map_t::iterator iter = mymanager->command_map.begin();
  
  for ( iter = mymanager->command_map.begin() ; iter != mymanager->command_map.end(); iter++ ) {
    command = iter->second;
  
//     printf("testing for command %s\n", command->command_name);
    char *ret = strstr(line, command->command_name);
    
    if (ret == line) { // string an erster stelle gefunden
//       printf("found!\n");
      
      goto foundcmd;
    }
  }
  
  return -1; // command not found
  
foundcmd:
  // command 
  int calb_retval = command->run_callback(this, line);
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

}

int rt_server_threads_manager::init_tcp(int port)
{
  iohelper =  new tcp_server(port);
  iohelper->tcp_server_init();
  
  printf("tcp started\n");
  
}

void rt_server_threads_manager::add_command(char* name, int id, int (*callback)(rt_server_command*, rt_server *rt_server_src) )
{
  rt_server_command *cmd = new rt_server_command(name, id, callback);
  
  command_map.insert(std::make_pair(id, cmd));
}


void rt_server_threads_manager::loop()
{

  for (;;) {
    tcp_connection *tcpc = iohelper->wait_for_connection();
      
    rt_server *rt_server_i = new rt_server(this);
    rt_server_i->set_tcp(tcpc);
    
    
    rt_server_i->init(); // the class will start its receiver thread
  }

}

void rt_server_threads_manager::destruct()
{
  command_map.clear();
  

}
