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
#include <list>

#include "ortd_buffered_io.h"
#include "remote_decoder.h"

#include <iostream>





rt_server_decoder_scope::rt_server_decoder_scope(rt_server_decoder* decoder, std::string name)
{
    this->decoder = decoder;

    this->name = name;

    std::cout << "new scope named " << this->name << "\n";

    /*  this->myiohelper = new ortd_tcpclient();
      this->myiohelper->tcpconnect (decoder->hostname, decoder->port);*/
    databuffer = NULL;

}

void rt_server_decoder_scope::setInfo(int veclen, int datatype, int StreamId)
{
    this->veclen = veclen;
    this->datatype = datatype;
    this->StreamId;

    databuffer = (double*) malloc( sizeof(double) * veclen );
}

void rt_server_decoder_scope::start_stream()
{
    decoder->bio->writeln("stream_fetch ");
    decoder->bio->writeln((char*) name.c_str());
    decoder->bio->writeln(" 0\n");
}

void rt_server_decoder_scope::stop_stream()
{
    decoder->bio->writeln("stream_fetch ");
    decoder->bio->writeln((char*) name.c_str());
    decoder->bio->writeln(" -1\n");
}

void rt_server_decoder_scope::enqueue_data(double* data)
{
//     printf("o\n");

    memcpy( (void*) databuffer, (void*) data, sizeof(double) * this->veclen );

//     printf("ok\n");

}

bool rt_server_decoder_scope::read_single_vec_blocking(double* wrtite_to)
{
    bool success = this->decoder->await_message();

//   printf("o\n");

    if (success)
        memcpy( (void*) wrtite_to, (void*) databuffer, sizeof(double) * this->veclen );

    return success;

}



rt_server_decoder_scope::~rt_server_decoder_scope()
{
    std::cout << "delete scope\n";

    if (databuffer != NULL) {
        free(databuffer);
    }
    
    std::cout << "ok\n";

}


/*
  Iohelper
*/


ortd_tcpclient::ortd_tcpclient()
{
    buffer_fd = NULL;


}

ortd_tcpclient::~ortd_tcpclient()
{
    if (buffer_fd != NULL)
        fclose(buffer_fd);
}

bool ortd_tcpclient::tcpconnect(char* hostname, int port)
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


//     buffer_fd = fdopen(this->sd, "r+");

    this->fd = this->sd;

    return true;
}

int ortd_tcpclient::get_fd()
{
    return fd;
}


void ortd_tcpclient::closeconnection()
{
    close(this->fd);
}






/*
main decoder class
*/


rt_server_decoder::rt_server_decoder(char* hostname, int port)
{
    this->hostname = (char*) malloc( strlen(hostname)+1 );
    strcpy(this->hostname, hostname);
    this->port = port;

    // open connection to the realtime process
    io = new ortd_tcpclient();
    io->tcpconnect(this->hostname, this->port);

    bio = new ortd_buffered_io( io->get_fd() );

    read_scopes();
}

bool rt_server_decoder::send_raw_command(char* cmd)
{
  bio->writeln(cmd);
}


bool rt_server_decoder::read_scopes()
{
    std::string input_line;
    int ret;

    std::list<std::string> scope_names;

    // initialse the automatic sending of information
    bio->writeln("ls\n");

    while (true) {
        ret = bio->waitforaline();

        if (ret < 0)
            break;

        // Read a line
        getline(bio->sstream_in, input_line);
//             std::cout << "I got: " << input_line << "\n";

        char name[1024];
        int type;

// 	    printf("parse: %s\n", input_line.c_str());
        ret = sscanf(input_line.c_str(), "%s %d", name, &type);
        if (ret == 2 && type == 2) {
            //

            printf("New scope \"%s\" %d\n", name, type);


            std::string name_ = name;
            scope_names.push_front(name_);




//             std::pair <std::string, rt_server_decoder_scope* > paired = std::make_pair(name_, scope);
//             scopes.insert(paired);

        }

        // copy the line into a stream
        std::stringstream tmp;
        tmp << input_line;

        if (input_line.compare(0, 7, "--EOR--") == 0) {
            // ingnore
            break;
        }
    }


    // For each scope

//     scope_names::iterator iter;
    while ( !scope_names.empty() ) {
        std::string iname = scope_names.front();
        scope_names.pop_back();

        rt_server_decoder_scope *iscope = new rt_server_decoder_scope(this, iname);



        bio->writeln("stream_fetch ");
        bio->writeln((char*) iscope->name.c_str());
        bio->writeln(" -2\n");


        if (bio->waitforaline() < 0)
            break;

        getline(bio->sstream_in, input_line);

        char name[1000];
        int veclen, datatype, StreamId;

//         ret = sscanf(input_line.c_str(), "Stream information for streamname=\"%s, veclen=%d, datatype=%d, StreamId=%d", name, &veclen, &datatype, &StreamId);
        ret = sscanf(input_line.c_str(), "Stream information for %s veclen=%d datatype=%d StreamId=%d", name, &veclen, &datatype, &StreamId);
        if (ret == 4 ) {

            printf("sinfo = %s, %d, %d, %d\n", name, veclen, datatype, StreamId);

            iscope->setInfo(veclen, datatype, StreamId);

            std::pair <int, rt_server_decoder_scope* > paired = std::make_pair(StreamId, iscope);
            scopes.insert(paired);


        }
    }


}


rt_server_decoder_scope* rt_server_decoder::get_scope_by_id(int id)
{
    scope_map::iterator it;
    it = scopes.find(id);

    if (it == scopes.end()) {
        printf("scopeid not found\n");
        return NULL;
    }

    return it->second;


}

rt_server_decoder_scope* rt_server_decoder::get_scope_by_name(std::string name)
{
    scope_map::iterator iter;

    for ( iter = scopes.begin() ; iter != scopes.end(); iter++ ) {
        printf("testing <%s> and <%s>\n", iter->second->name.c_str(), name.c_str() );
	if ( iter->second->name.compare( name ) == 0 ) {
//         if (strcmp( 
	  printf("found scope %s\n", name.c_str());
	  return iter->second; 
	}
    }

	  printf("scope %s was not found\n", name.c_str());
    
    return NULL;
}


bool rt_server_decoder::await_message()
{
//     printf("in await_message\n");
    int ret = bio->waitforaline();

    if (ret < 0)
        return false;

    // Read a line
    std::string input_line;
    getline(bio->sstream_in, input_line);
//     std::cout << "I got: " << input_line << "\n";

    // copy the line into a stream
    std::stringstream tmp;
    tmp << input_line;

    if (input_line.compare(0, 7, "--EOR--") == 0) {
        // ingnore

    }
    else if (input_line.compare(0, 6, "stream") == 0)     {


        // read away "stream "
        for (int i=0; i<6; ++i) {
            char c;
            tmp >> c;

            //std::cout << c;
        }

        // read stream packet information
        int NDatasets, ScopeId;
        double streamData[100];  // FIXME

        tmp >> NDatasets;
        tmp >> ScopeId;

        //    std::cout << "ScopeId: " << ScopeId << ", NDatasets: " << NDatasets << '\n';

        // read out the numerical content
        int Nread=0;
        while ( !tmp.eof() ) {
            tmp >> streamData[Nread];
            ++Nread;
            if (Nread > sizeof(streamData))
                break;
        }
        Nread--;


//         std::cout << "read " << Nread << " values: ";

//         for (int i = 0; i < Nread; ++i)
//             std::cout << streamData[i] << " ";

//         std::cout << "\n";

        rt_server_decoder_scope *sc = get_scope_by_id(ScopeId);
        if (sc != NULL) {
//             printf("enq data\n");

            sc->enqueue_data( streamData );
        }
    }

    input_line.clear();

    return true;

}


rt_server_decoder::~rt_server_decoder()
{
    scope_map::iterator iter;

    for ( iter = scopes.begin() ; iter != scopes.end(); iter++ ) {
        delete iter->second;
    }

  std::cout << "all scopes deleted\n";

    free(this->hostname);
    
    std::cout << "...\n";
}

rt_server_decoder_scope* rt_server_decoder::get_next_scope()
{

}
