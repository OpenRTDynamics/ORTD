

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


#include "ortd_buffered_io.h"


/*

ortd_buffered_io::ortd_buffered_io(int fd)
{
  this->fd = fd;
}

int ortd_buffered_io::waitforaline()
{
  int socket = this->fd;
  
//  this->sstream_in << "a line: \n";
  
   for(char c; recv(socket, &c, 1, 0) > 0; ) 
    { 
      this->sstream_in << c;
      //std::cout << c;
      
        if(c == '\n') 
        { 
//           printf("got a line...\n");
            return 0; 
        } 
    }
    
    return -1;
    
}

// void ortd_buffered_io::writeln(char* line)
// {
//   send(this->fd, line, strlen(line), 0 );
// }

int ortd_buffered_io::writeln(char* line)
//void SendAll(int socket, const char* const buf, const int size) 
{ 
  int socket = this->fd;
  char *buf = line;
  int size = strlen(line);
  
    int bytesSent = 0; // Anzahl Bytes die wir bereits vom Buffer gesendet haben 
    do 
    { 
        int result = send(socket, buf + bytesSent, size - bytesSent, 0); 
        if(result < 0) // Wenn send einen Wert < 0 zurück gibt deutet dies auf einen Fehler hin. 
        { 
  //          throw CreateSocketError(); 
          return -1;
        } 
        bytesSent += result; 
    } while(bytesSent < size); 
    
    return 0;
}

ortd_buffered_io::~ortd_buffered_io()
{
//  sstream_in.
}


*/


ortd_buffered_io::ortd_buffered_io(int fd)
{
    this->fd = fd;
}

int ortd_buffered_io::waitforaline()
{
    int socket = this->fd;

//  this->sstream_in << "a line: \n";

    for (char c; recv(socket, &c, 1, 0) > 0; )
    {
        this->sstream_in << c;
        //std::cout << c;

        if (c == '\n')
        {
            // printf("got a line...\n");
            return 0;
        }
    }

    return -1;

}

// void ortd_buffered_io::writeln(char* line)
// {
//   send(this->fd, line, strlen(line), 0 );
// }

int ortd_buffered_io::writeln(char* line)
//void SendAll(int socket, const char* const buf, const int size)
{
    int socket = this->fd;
    char *buf = line;
    int size = strlen(line);

    int bytesSent = 0; // Anzahl Bytes die wir bereits vom Buffer gesendet haben
    do
    {
        int result = send(socket, buf + bytesSent, size - bytesSent, 0);
        if (result < 0) // Wenn send einen Wert < 0 zurück gibt deutet dies auf einen Fehler hin.
        {
            //          throw CreateSocketError();
            return -1;
        }
        bytesSent += result;
    } while (bytesSent < size);

    return 0;
}

ortd_buffered_io::~ortd_buffered_io()
{
//  sstream_in.
}


