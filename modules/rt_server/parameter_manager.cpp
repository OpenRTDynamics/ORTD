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

#include "parameter_manager.h"

#include <malloc.h>

parameter::parameter()
{
    data = NULL;
    data_loadbuffer = NULL;
}

parameter::parameter(parameter_manager *p_mgr, const char *name, int type, int const_size)
{
    data = NULL;

    datatype = type;
    nElements = const_size;

    this->p_mgr = p_mgr;
    this->name = name;

    // init buffer mutex
    pthread_mutex_init(&buffer_mutex, NULL);


    switch (type) {
    case DATATYPE_FLOAT:
    {
        byte_size = sizeof(double) * const_size;
        data = (void*) malloc( byte_size );
        data_loadbuffer = (void*) malloc( byte_size );
        memset(data, 0x0, byte_size );
        memset(data_loadbuffer, 0x0, byte_size );
    }

    }

    // for now insert into root
    p_mgr->directory->add_entry((char*) name, ORTD_DIRECTORY_ENTRYTYPE_PARAMETER, p_mgr, (void*) this);
}

void parameter::destruct()
{
    // remove directory.h entry
    p_mgr->directory->delete_entry((char*) name);

    if (data != NULL)
        free(data);

    if (data_loadbuffer != NULL)
        free(data_loadbuffer);
}

parameter::~parameter()
{

}



void parameter::lock_buffer()
{
    pthread_mutex_lock(&buffer_mutex);
}

void parameter::unlock_buffer()
{
    pthread_mutex_unlock(&buffer_mutex);
}


void* parameter::get_data_ptr()
{
    return data;
}

void parameter::atomic_buffer_copy_b2d()
{
    // LOCK
    lock_buffer();

    memcpy(data, data_loadbuffer, byte_size);

    // UNLOCK
    unlock_buffer();
}

void parameter::atomic_buffer_copy_d2b()
{
    // LOCK
    lock_buffer();

    memcpy(data_loadbuffer, data, byte_size);

    // UNLOCK
    unlock_buffer();
}

void parameter::atomic_buffer_copy(void* dest)
{
    // LOCK
    lock_buffer();

    memcpy(dest, data, byte_size);

    // UNLOCK
    unlock_buffer();
}

void parameter::atomic_buffer_copy_e2d(void* source)
{
    memcpy(data, source, byte_size);
}



// Parse the vector wothin line and store to data_loadbuffer. Then atomic copy it to data
int parameter::parse_and_set(char* line)
{
    char *iter = line;
    int ret;
    int valcounter = 0;

    // get all values
    do {
        double v;

        iter = strchr(iter, '#');
        if (iter == NULL)
            break;

        iter++; // ' ' überspringen

        //       printf(":: %s\n", iter);

        // read value

        switch ( datatype ) {
        case DATATYPE_FLOAT:
        {

            double *param = (double *) this->data_loadbuffer;

            ret = sscanf(iter, "%lf", &v );
            if (ret == 1) { // found a new value

                param[valcounter] = v; // store the new values

                ++valcounter;
                //printf("%f\n", v);
            }

        }
        };

    } while ( valcounter < nElements );

    printf("pm: pause...\n");
//     sleep(1);
    printf("pm: pause finished\n");
    
    atomic_buffer_copy_b2d();

}











int ortd_callback_setpar__(rt_server_command *cmd, rt_server *rt_server_src)
{
    parameter_manager *pmgr = (parameter_manager *) cmd->userdat;
    pmgr->callback_set( cmd, rt_server_src );
    
    	    if (DEBUG==1) printf("rt_server, pm: return from ortd_callback_setpar__\n");

}

int ortd_callback_getpar__(rt_server_command *cmd, rt_server *rt_server_src)
{
    parameter_manager *pmgr = (parameter_manager *) cmd->userdat;
    pmgr->callback_get( cmd, rt_server_src );
}



void parameter_manager::callback_get(rt_server_command* cmd, rt_server* rt_server_src)
{
    char *parstr = cmd->get_parameter_str();

//   printf("GOT: %s\n", parstr);

//   set_param halloasfs ; 1.2 ; 1.3
//   set_param   /param/value1 #  1.2 #1.3 #234.55 #23#23.4
// set_param parameter1 #  1.2 #1.3 #234.55 #23#23.4
//  get_param parameter1

    char *pardir;
    int ret = sscanf(parstr, "%as ", &pardir);
    if (ret == 1) {
//     printf("pardir = \"%s\"\n", pardir);

        directory_entry::direntry* dentr = directory->access(pardir, this);
        free(pardir);

        if (dentr == NULL) {
//       printf("parameter not found\n");
            goto error_pnf;
        }

        parameter *param = (parameter *) dentr->userptr;

        if (param != NULL) {
            param->atomic_buffer_copy_d2b();

            int i;
            double *par = (double*) param->data_loadbuffer;

            // send a list of values
//       cmd->send_answer(rt_server_src, "dataset = [ \n");

            for (i = 0; i < param->nElements; ++i) {
                char returnstr[50];

                sprintf(returnstr, "%f\n", par[i]);
                cmd->send_answer(rt_server_src, returnstr);
            }

//       cmd->send_answer(rt_server_src, "];\n");


        }



    }


//   cmd->send_answer(rt_server_src, "**\n");
    return;

error_pnf:
    cmd->send_answer(rt_server_src, "Error: parameter not found\n");
    return;
}



void parameter_manager::callback_set(rt_server_command* cmd, rt_server* rt_server_src)
{
    char *parstr = cmd->get_parameter_str();

//   printf("GOT: %s\n", parstr);

//   set_param halloasfs ; 1.2 ; 1.3
//   set_param   /param/value1 #  1.2 #1.3 #234.55 #23#23.4
// set_param parameter1 #  1.2 #1.3 #234.55 #23#23.4

    char *pardir;
    int ret = sscanf(parstr, "%as ", &pardir);
    if (ret == 1) {
//     printf("pardir = \"%s\"\n", pardir);

        directory_entry::direntry* dentr = directory->access(pardir, this);
        free(pardir);

        if (dentr == NULL) {
//       printf("Error: parameter not found\n");
            goto error_pnf;
        }

        parameter *param = (parameter *) dentr->userptr;

        if (param != NULL) {
            param->parse_and_set(parstr);

	    if (DEBUG==1) printf("rt_server, pm: sinding ok\n");

	    
            cmd->send_answer(rt_server_src, "Parameter was set\n");

	    if (DEBUG==1) printf("rt_server, pm: ok was sent\n");

            return;
        }

    }

    cmd->send_answer(rt_server_src, "Error: parser error\n");
    return;


error_pnf:
    cmd->send_answer(rt_server_src, "Error: parameter not found\n");
    return;
}


parameter_manager::parameter_manager(rt_server_threads_manager* rts_thmng, directory_tree* root_directory)
{
    this->rts_thmng = rts_thmng;
    this->directory = root_directory;

    // register commands
    this->rts_thmng->add_command("set_param", &ortd_callback_setpar__, this );
    this->rts_thmng->add_command("get_param", &ortd_callback_getpar__, this );

}

parameter * parameter_manager::new_parameter( char *name, int type, int size )
{
    parameter * p = new parameter(this, name, type, size);

    return p;
}

void parameter_manager::delete_parameter(parameter* par)
{
//    FIXME TODO
}


void parameter_manager::destruct()
{
//    FIXME: TODO
}


/*

  Wrapper for log.c's ringbuffer

*/




ortd_ringbuffer::ortd_ringbuffer(int element_size, int num_elements, int autoflushInterval)
{
    rb = log_ringbuffer_new(element_size, num_elements, autoflushInterval);
}

ortd_ringbuffer::~ortd_ringbuffer()
{
    log_ringbuffer_del(rb);
}

int ortd_ringbuffer::read_block(void *data)
{
    return log_ringbuffer_read(rb, data, 0);
}

int ortd_ringbuffer::read_nonblock(void *data)
{
    return log_ringbuffer_read(rb, data, 1);
}

void ortd_ringbuffer::write(void* data, int numElements)
{
    log_ringbuffer_write(rb, data, numElements);
}

void ortd_ringbuffer::flush()
{
    log_ringbuffer_flush(rb);
}







/*

  A Stream multiplexer

*/


ortd_stream_multiplexer::ortd_stream_multiplexer(ortd_stream* stream, void* dataptr, int datatype, int vlen, int autoflushInterval, double autoflushtimeout)
{
    this->stream = stream;
    this->dataptr = dataptr;
    this->datatype = datatype;
    this->nElements = vlen;

    this->autoflushtimeout = autoflushtimeout;
    this->autoflushInterval = autoflushInterval;
    autoflushInterval_counter = 0;

//      init list (comes later)
    rt_server_i = NULL;
    pthread_mutex_init(&client_list_mutex, NULL);

//  initialise log.h sink infrastructure
    int bufsize = 100;
    int numElementsToWrite = 1;
    int element_size  = sizeof(double) * nElements;

//    FIXME decide on the datatype
    sink = log_sink_new(element_size, bufsize, (void*) &ortd_stream_multiplexer::callback_c,
                        (void*) this, numElementsToWrite);
    if (sink == 0) {
//      return 0;
    }

}

ortd_stream_multiplexer::~ortd_stream_multiplexer()
{
    log_sink_del(sink);

    pthread_mutex_destroy(&client_list_mutex);
}

int ortd_stream_multiplexer::callback_cpp(void* data, int flag)
{
    int ret;


    switch (flag) {
    case 1: {

        printf("stream transmission: open fifo \n");

    }
    return 0;
    break;
    case 2: {  // process data
        void *vec = (char *) data;
//   printf("calback\n");
//  	  printf("process\n");
        multiplex(data);

    }
    return 0;
    break;
    case 4: {  // flush
    }
    return 0;
    break;
    case 3:  // close
    {
        printf("stream transmission: close fifo\n");
    }
    return 0;
    break;
    }

}


int ortd_stream_multiplexer::callback_c(void* data, void* calbdata, int flag)
{
    ortd_stream_multiplexer * mux = (ortd_stream_multiplexer *) calbdata;

    //   just run the c++ version of the calback function
    mux->callback_cpp(data, flag);
}




void ortd_stream_multiplexer::add_client(rt_server* rt_server_i)
{

    printf("resistering new client\n");

    rt_server_i->iohelper->register_usage();

    // muxtex
    lock_client_list();
    this->rt_server_i = rt_server_i;


    unlock_client_list();
    // increase usage counter
//   rt_server_i->register_usage();
}

void ortd_stream_multiplexer::remove_client(rt_server* rt_server_i)
{
//   rt_server_i->unregister_usage();

    lock_client_list();
    this->rt_server_i = NULL;
    unlock_client_list();

    rt_server_i->iohelper->unregister_usage();

}

void ortd_stream_multiplexer::lock_client_list()
{
//   pthread_mutex_lock(&client_list_mutex);
}

void ortd_stream_multiplexer::unlock_client_list()
{
//   pthread_mutex_unlock(&client_list_mutex);
}


int ortd_stream_multiplexer::feed_data(void *data)
{
    log_ringbuffer_write(sink->rb, data, 1);
    
// TODO:      call only each "autoflushInterval" samples
    log_sink_flush(sink);
}

int ortd_stream_multiplexer::multiplex(void *data)
{
    // go through list

    lock_client_list();

    if (this->rt_server_i == NULL) {
//        printf("no rt_server was set-up\n");

        return 0;
    }

//      printf("sending data\n");

    rt_server *rt_server_client;

    rt_server_client = this->rt_server_i;

    char str[256];
    int ret;

    // decide on datatype
    if (datatype == DATATYPE_FLOAT) {
        double *vec = (double *) data;

//          printf("send\n");

	sprintf(str, "stream %d %d ", 1, this->stream->StreamId );
        ret = rt_server_client->iohelper->writeln(str);
        if (ret < 0) goto ioerror;
	
        // print all elements of the vector
        int i;
        for (i = 0; i < this->nElements; ++i) {

// 	   	printf(str, "%f, ", vec[i]);
            sprintf(str, "%f ", vec[i]);

//                printf("sending %s\n", str);


            ret = rt_server_client->iohelper->writeln(str);
            if (ret < 0) goto ioerror;

        }

        ret = rt_server_client->iohelper->writeln("\n");
        if (ret < 0) goto ioerror;

    }

    //
    // autoflush of buffer to the client if needed
    //


    autoflushInterval_counter++;

    if (autoflushInterval_counter >= autoflushInterval) {
        autoflushInterval_counter = 0;
        // flush data_loadbuffer

        rt_server_client->iohelper->send_flush_buffer();
    }



    unlock_client_list();
    return 0;

ioerror:

    printf("ioerror\n");

//      Client not available any more. close connection
    this->remove_client(rt_server_client);

    unlock_client_list();
    return -1;

}



/*

  A Stream

*/


ortd_stream::ortd_stream(ortd_stream_manager* str_mgr, const char* name, int datatype, int const_len, int numBufferElements, int autoflushInterval, int StreamId )
{
    // const_len : number of vector elements

    nElements = const_len;
    numBytes = const_len * libdyn_config_get_datatype_len(datatype);
    rb = new ortd_ringbuffer(numBytes, numBufferElements, 0);
    this->datatype = datatype;
    oneElementBuf = malloc( numBytes );

    this->autoflushInterval = autoflushInterval;
    this->name = name;
    this->str_mgr = str_mgr;
    this->StreamId = StreamId;

//     Create the stream multiplexer
    multiplexer = new ortd_stream_multiplexer(this, NULL, this->datatype, nElements, autoflushInterval, 0.0);



    // reader mutex init
    pthread_mutex_init(&mutex_readstream, NULL);


//   Add directory entry
    str_mgr->directory->add_entry((char*) name, ORTD_DIRECTORY_ENTRYTYPE_STREAM, str_mgr, (void*) this);
}

ortd_stream::~ortd_stream()
{
    delete multiplexer;
}

void ortd_stream::destruct()
{
    // remove file entry
    str_mgr->directory->delete_entry((char*) name);

    delete rb;
    free(oneElementBuf);
}

int ortd_stream::parse_and_return(rt_server_command* cmd, rt_server* rt_server_src, char* line, int ndatasets)
{
    int ret;
    int fetch_counter = 0;
    int max_fetch = ndatasets;
    //int autoflushInterval = 10;
    int autoflushInterval_counter = 0;

//  lock stream
    pthread_mutex_lock(&mutex_readstream);


//   while ( fetch_counter < max_fetch && (ret = rb->read_nonblock(oneElementBuf)) >= 0 ) { // fetch new vector element

    for (;;) {

        if (ndatasets != 0) { // read only a limited number of samples

            if (rb->read_nonblock(oneElementBuf) < 0)
                break;

            if (fetch_counter >= max_fetch)
                break;

        } else { // continously read samples
            printf("blockingread\n");
            if (rb->read_block(oneElementBuf) < 0) {
                printf("break\n");

                break;
            }
            printf("continue\n");

        }

        fetch_counter++;
//     printf(". ret=%d\n", ret);

        char str[256];

        // decide on datatype
        if (datatype == DATATYPE_FLOAT) {
            double *vec = (double *) oneElementBuf;

            printf("send\n");

            // print all elements of the vector
            int i;
            for (i = 0; i < this->nElements; ++i) {
// 	printf(str, "%f, ", vec[i]);
                sprintf(str, "%f ", vec[i]);
//         ret = cmd->send_answer(rt_server_src, str);
                ret = rt_server_src->iohelper->writeln(str);
                if (ret < 0) goto ioerror;

                printf("send %s\n", str);
            }

//       ret = cmd->send_answer(rt_server_src, "\n");
            ret = rt_server_src->iohelper->writeln("\n");
            if (ret < 0) goto ioerror;

        }

        //
        // autoflush of buffer to the client if needed
        //

// stream_fetch osc_output 12

        if (ndatasets == 0) {
            autoflushInterval_counter++;

            if (autoflushInterval_counter >= autoflushInterval) {
                autoflushInterval_counter = 0;
                // flush data_loadbuffer

                rt_server_src->iohelper->send_flush_buffer();
//  	       cmd->send_flush_buffer();
            }
        }

    }


    pthread_mutex_unlock(&mutex_readstream);
    return 0;

ioerror:

    printf("ioerror\n");
    pthread_mutex_unlock(&mutex_readstream);
    return -1;
}


int ortd_stream::send_info(rt_server_command* cmd, rt_server* rt_server_src, char* line, int nElements)
{
//  lock stream
//     pthread_mutex_lock(&mutex_readstream);

//     FILE *fd = rt_server_src->iohelper->get_io_fd();
//      fprintf(fd, "Stream information for %s\n", this->name);

  // FIXME: some locks missing?
//     rt_server_src->iohelper->register_usage();
  
    char tmp[1024];
    sprintf(tmp, "Stream information for streamname=\"%s\", veclen=%d, datatype=%d\n", this->name, this->nElements, this->datatype);

    rt_server_src->iohelper->writelnff(tmp);


//   rt_server_src->iohelper->writeln("scope information\n");


//     pthread_mutex_unlock(&mutex_readstream);
    return 0;

ioerror:

    printf("ioerror\n");
//     pthread_mutex_unlock(&mutex_readstream);
    return -1;

}


// writes one vecotr to the stream
void ortd_stream::write_to_stream(void* data)
{
    multiplexer->feed_data(data);

    rb->write(data, 1);
}

void ortd_stream::add_client(rt_server* rt_server_i)
{
    multiplexer->add_client(rt_server_i);
}

void ortd_stream::remove_client(rt_server* rt_server_i)
{
    multiplexer->remove_client(rt_server_i);
}






/*

 Stream Manager

*/


// C-callback function
int ortd_callback_fetchstream__(rt_server_command *cmd, rt_server *rt_server_src)
{
    ortd_stream_manager *smgr = (ortd_stream_manager *) cmd->userdat;
    smgr->callback_get( cmd, rt_server_src );
}

void ortd_stream_manager::callback_get(rt_server_command* cmd, rt_server* rt_server_src)
{
    char *parstr = cmd->get_parameter_str();

    // get the name of the stream
    char *pardir;
    int nElements; // number of elements to fetch
    int ret = sscanf(parstr, "%as %d ", &pardir, &nElements);
    if (ret == 2) {
//     printf("name of the stream = \"%s\" fetch = %d \n", pardir, nElements);
        directory_entry::direntry* dentr = directory->access(pardir, this);
        free(pardir);

        if (dentr == NULL || dentr->type != ORTD_DIRECTORY_ENTRYTYPE_STREAM) {
//       printf("Error: stream not found\n");
            goto error_pnf;
        }

        ortd_stream *stream = (ortd_stream *) dentr->userptr;

        if (stream != NULL) {

// 	  send stream data
            if (nElements > 0)
                stream->parse_and_return(cmd, rt_server_src, parstr, nElements);

            if (nElements == 0) // register the client for receiving the stream automatically
                stream->add_client(rt_server_src);

            if (nElements == -1) // deregister the client for receiving the stream automatically
                stream->remove_client(rt_server_src);

// 	  send some information about the stream
            if (nElements == -2)
                stream->send_info(cmd, rt_server_src, parstr, nElements);

            return;
        }

    }

    cmd->send_answer(rt_server_src, "Error: parser error\n");
    return;


error_pnf:
    cmd->send_answer(rt_server_src, "Error: stream not found\n");
    return;
}

ortd_stream_manager::ortd_stream_manager(rt_server_threads_manager* rts_thmng, directory_tree* directory)
{
    this->rts_thmng = rts_thmng;
    this->directory = directory; // the directory tree subsystem
    this->StreamId_counter = 22;

    // register commands
    this->rts_thmng->add_command("stream_fetch", &ortd_callback_fetchstream__, this );
}

ortd_stream* ortd_stream_manager::new_stream(char* name, int datatype, int const_len, int numBufferElements, int autoflushInterval)
{
    this->StreamId_counter++;
    
    ortd_stream * stream = new ortd_stream(this, name, datatype, const_len, numBufferElements, autoflushInterval, this->StreamId_counter);

    return stream;
}

void ortd_stream_manager::delete_stream(ortd_stream* stream)
{
    // FIXME TODO
}


void ortd_stream_manager::destruct()
{

}


