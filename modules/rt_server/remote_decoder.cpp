// Decoder that reads out the fifo file
//

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







rt_server_decoder_scope::rt_server_decoder_scope(rt_server_decoder* decoder)
{
  this->decoder = decoder;

  this->myiohelper = new rt_server_decoder_iohelper();
  this->myiohelper->tcpconnect (decoder->hostname, decoder->port);
    
}

rt_server_decoder_scope::~rt_server_decoder_scope()
{

}


/*
  Iohelper
*/


rt_server_decoder_iohelper::rt_server_decoder_iohelper()
{
  buffer_fd = NULL;
  
  
}

rt_server_decoder_iohelper::~rt_server_decoder_iohelper()
{
  if (buffer_fd != NULL)
    fclose(buffer_fd);    
}

bool rt_server_decoder_iohelper::tcpconnect(char* hostname, int port)
{
    struct sockaddr_in serveraddr;

    int rc;
    struct hostent *hostp;
    
    char *server = hostname;

    if ((this->sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Client-socket() error");
        return false;
    }
    else
        printf("Client-socket() OK\n");
    
    

    memset(&serveraddr, 0x00, sizeof(struct sockaddr_in));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);

    if ((serveraddr.sin_addr.s_addr = inet_addr(server)) == (unsigned long)INADDR_NONE)
    {

        /* When passing the host name of the server as a */
        /* parameter to this program, use the gethostbyname() */
        /* function to retrieve the address of the host server. */
        /***************************************************/
        /* get host address */
        hostp = gethostbyname(server);
        if (hostp == (struct hostent *)NULL)
        {
            printf("HOST NOT FOUND --> ");
            /* h_errno is usually defined */
            /* in netdb.h */
            printf("h_errno = %d\n",h_errno);
            close(sd);
            return false;
        }
        memcpy(&serveraddr.sin_addr, hostp->h_addr, sizeof(serveraddr.sin_addr));
    }

    /* After the socket descriptor is received, the */
    /* connect() function is used to establish a */
    /* connection to the server. */
    /***********************************************/
    /* connect() to server. */
    if ((rc = connect(sd, (struct sockaddr *)&serveraddr, sizeof(serveraddr))) < 0)
    {
        perror("Client-connect() error");
        close(sd);
        return false;
    }
    else
        printf("Connection established...\n");
    
    
    buffer_fd = fdopen(this->sd, "r+");

    return true;
}



/*
main decoder class
*/


rt_server_decoder::rt_server_decoder(char* hostname, int port)
{
  this->hostname = (char*) malloc( strlen(hostname)+1 );
  strcpy(this->hostname, hostname);
  
  this->port = port;
  
  this->myiohelper = new rt_server_decoder_iohelper();
  this->myiohelper->tcpconnect (this->hostname, this->port);
}

rt_server_decoder::~rt_server_decoder()
{
  free(this->hostname);
}

rt_server_decoder_scope* rt_server_decoder::get_next_scope()
{

}
