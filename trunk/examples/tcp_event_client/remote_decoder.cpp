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


#include "remote_decoder.h"

#include <iostream>

#define FAILURE -1
#define NOTHING_HAPPENED 0
#define MORE_SCOPES_TO_READ 1
#define MORE_SCOPES_INFOS_TO_READ 2
#define ALL_SCOPES_WITH_INFOS_READ 3
#define SCOPE_DATA_ARRIVED 4



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
    this->StreamId = StreamId;

    databuffer = (double*) malloc( sizeof(double) * veclen );
}

void rt_server_decoder_scope::start_stream()
{
    decoder->send_raw_command("stream_fetch ");
    decoder->send_raw_command((char*) name.c_str());
    decoder->send_raw_command(" 0\n");
}

void rt_server_decoder_scope::stop_stream()
{
    decoder->send_raw_command("stream_fetch ");
    decoder->send_raw_command((char*) name.c_str());
    decoder->send_raw_command(" -1\n");
}

void rt_server_decoder_scope::enqueue_data(double* data)
{
//     printf("o\n");

    memcpy( (void*) databuffer, (void*) data, sizeof(double) * this->veclen );

//     printf("ok\n");

}

/*bool rt_server_decoder_scope::read_single_vec_blocking(double* wrtite_to)
{
    bool success = this->decoder->await_message();

//   printf("o\n");

    if (success)
        memcpy( (void*) wrtite_to, (void*) databuffer, sizeof(double) * this->veclen );

    return success;

}*/

void rt_server_decoder_scope::read_data(double* wrtite_to)
{

    memcpy( (void*) wrtite_to, (void*) databuffer, sizeof(double) * this->veclen );

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
main decoder class
*/


rt_server_decoder::rt_server_decoder(char* hostname, int port, qt_event_communication* qt_event_com)
{
    this->hostname = (char*) malloc( strlen(hostname)+1 );
    strcpy(this->hostname, hostname);
    this->port = port;

    state = NOTHING_HAPPENED;
    this->qt_event_com = qt_event_com;

}

bool rt_server_decoder::send_raw_command(char* cmd)
{
    qt_event_com->emit_cmd(cmd);
    return true;
}

rt_server_decoder_scope* rt_server_decoder::GetTheScopeThatWasChanged()
{
    rt_server_decoder_scope *sc = get_scope_by_id(changed_scope_id);
    if (sc != NULL)
    {
        return sc;
    }
    else
    {
        return NULL;
    }
}

void rt_server_decoder::init_sending()
{
    // initialise the automatic sending of information
    if (state == NOTHING_HAPPENED)
    {
        send_raw_command("ls\n");
        state = MORE_SCOPES_TO_READ;
    }
}

int rt_server_decoder::feed_external_data(std::string DataFromTcpLine)
{
    bool all_scopes_read, all_scopes_infos_read;
    int parse_info;

    switch (state)
    {
    case MORE_SCOPES_TO_READ:
        all_scopes_read = read_scope(DataFromTcpLine);
        if (all_scopes_read)
        {
            state = MORE_SCOPES_INFOS_TO_READ;
            get_scopes_infos();
            return MORE_SCOPES_INFOS_TO_READ;
        }
        else
        {
            return MORE_SCOPES_TO_READ;
        }
        break;
    case MORE_SCOPES_INFOS_TO_READ:
        all_scopes_infos_read = read_scope_info(DataFromTcpLine);
        if (all_scopes_infos_read)
        {
            state = ALL_SCOPES_WITH_INFOS_READ;
            return ALL_SCOPES_WITH_INFOS_READ;
        }
        else
        {
            return MORE_SCOPES_INFOS_TO_READ;
        }
        break;
    case ALL_SCOPES_WITH_INFOS_READ:
        return parse_line(DataFromTcpLine);
    default:
        printf("Wrong state: %d\n", &state);
    }

    return FAILURE;
}

bool rt_server_decoder::read_scope(std::string input_line)
{
    char name[1024];
    int type;
    int ret;

    ret = sscanf(input_line.c_str(), "%s %d", name, &type);
    if (ret == 2 && type == 2) {

        printf("New scope \"%s\" %d\n", name, type);


        std::string name_ = name;
        scope_names.push_front(name_);

    }

    if (input_line.compare(0, 7, "--EOR--") == 0) {
        // no more scopes -> next state
        if (state == MORE_SCOPES_TO_READ)
        {
            return true; // all scopes are read
        }
    }

    return false; // there are more scopes to read
}

void rt_server_decoder::get_scopes_infos()
{
    // For each scope

    while ( !scope_names.empty() ) {
        std::string iname = scope_names.front();
        scope_names.pop_back();

        send_raw_command("stream_fetch ");
        send_raw_command((char*) iname.c_str());
        send_raw_command(" -2\n");
    }

}

bool rt_server_decoder::read_scope_info(std::string input_line)
{
    char name[1000];
    int veclen, datatype, StreamId;
    int ret;

    ret = sscanf(input_line.c_str(), "Stream information for %s veclen=%d datatype=%d StreamId=%d", name, &veclen, &datatype, &StreamId);
    if (ret == 4 ) {

        printf("sinfo = %s, %d, %d, %d\n", name, veclen, datatype, StreamId);

        rt_server_decoder_scope *iscope = new rt_server_decoder_scope(this, name);

        iscope->setInfo(veclen, datatype, StreamId);

        std::pair <int, rt_server_decoder_scope* > paired = std::make_pair(StreamId, iscope);
        scopes.insert(paired);
    }

    if (input_line.compare(0, 7, "--EOR--") == 0) {
        // no more scopes info -> next state
        if (state == MORE_SCOPES_INFOS_TO_READ)
        {
            return true; // all scopes infos are read
        }
    }

    return false; // there are more scopes infos to read
}

/*bool rt_server_decoder::read_scopes()
{
    std::string input_line;
    int ret;

    std::list<std::string> scope_names;

    while (true) {

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



        send_raw_command("stream_fetch ");
        send_raw_command((char*) iscope->name.c_str());
        send_raw_command(" -2\n");


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


}*/


rt_server_decoder_scope* rt_server_decoder::get_scope_by_id(int id)
{
    scope_map::iterator it;
    it = scopes.find(id);

    if (it == scopes.end()) {
        printf("scopeid not found %d\n", id);
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


/*bool rt_server_decoder::await_message()
{
//     printf("in await_message\n");
    int ret = bio->waitforaline();

    if (ret < 0)
        return false;

    // Read a line
    std::string input_line;
    getline(bio->sstream_in, input_line);
//     std::cout << "I got: " << input_line << "\n";

    
    ////////////
    parse_line(input_line);
    

}*/

int rt_server_decoder::parse_line(std::string input_line)
{
    // copy the line into a stream
    std::stringstream tmp;
    tmp << input_line;
    int parse_info = NOTHING_HAPPENED;

    if (input_line.compare(0, 7, "--EOR--") == 0) {
        // ignore

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

        changed_scope_id = ScopeId;

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
            parse_info = SCOPE_DATA_ARRIVED;
        }
    }

    input_line.clear();

    return parse_info;

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
