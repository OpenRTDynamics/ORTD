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

#include "parameter_manager.h"

#include <malloc.h>

parameter::parameter()
{
  data = NULL;
  data_loadbuffer = NULL;
}

parameter::parameter(parameter_manager *p_mgr, char *name, int type, int const_size)
{
  data = NULL;
  
  datatype = type;
  nElements = const_size;
  
  switch(type) {
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
  p_mgr->directory->add_entry(name, ORTD_DIRECTORY_ENTRYTYPE_PARAMETER, p_mgr, (void*) this);
}

parameter::~parameter()
{

}



void parameter::lock_buffer()
{
  // FIXME
}

void parameter::unlock_buffer()
{
  // FIXME
}


void* parameter::get_data_ptr()
{
  return data;
}

void parameter::atomic_buffer_copy_b2d()
{
  // LOCK
  memcpy(data, data_loadbuffer, byte_size);
  // UNLOCK
}

void parameter::atomic_buffer_copy_d2b()
{
  // LOCK
  memcpy(data_loadbuffer, data, byte_size);
  // UNLOCK
}

void parameter::atomic_buffer_copy(void* dest)
{
  // LOCK
  memcpy(dest, data, byte_size);
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
	      printf("%f\n", v);
	    }
		
	  }  
	};
	
      } while ( valcounter < nElements );


      atomic_buffer_copy_b2d();
      
}



void parameter::destruct()
{
  if (data != NULL)
    free(data);

  if (data_loadbuffer != NULL)
    free(data_loadbuffer);
}









int ortd_callback_setpar__(rt_server_command *cmd, rt_server *rt_server_src)
{
  parameter_manager *pmgr = (parameter_manager *) cmd->userdat;
  pmgr->callback_set( cmd, rt_server_src );
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
      
      cmd->send_answer(rt_server_src, "Parameter was set\n");
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

  A Stream

*/


ortd_stream::ortd_stream(ortd_stream_manager * str_mgr, char *name, int datatype, int const_len, int numBufferElements )
{
  // const_len : number of vector elements
  
  nElements = const_len;
  numBytes = const_len * libdyn_config_get_datatype_len(datatype);
  rb = new ortd_ringbuffer(numBytes, numBufferElements, 0);
  this->datatype = datatype;
  oneElementBuf = malloc( numBytes );
  
//   Add directory entry
  str_mgr->directory->add_entry(name, ORTD_DIRECTORY_ENTRYTYPE_STREAM, str_mgr, (void*) this);
}

ortd_stream::~ortd_stream()
{
  // FIXME; remove file (also for parameter class)
}

void ortd_stream::destruct()
{
  delete rb;
  free(oneElementBuf);
}

int ortd_stream::parse_and_return(rt_server_command* cmd, rt_server* rt_server_src, char* line, int ndatasets)
{
  int ret;
  int fetch_counter = 0;
  int max_fetch = ndatasets;
  
  while ( fetch_counter < max_fetch && (ret = rb->read_nonblock(oneElementBuf)) >= 0 ) { // fetch new vector element
    fetch_counter++;
//     printf(". ret=%d\n", ret);
    
    char str[256];
    
    // decide on datatype
    if (datatype == DATATYPE_FLOAT) {
      double *vec = (double *) oneElementBuf;
      
      // print all elements of the vector
      int i;
      for (i = 0; i < this->nElements; ++i) {
// 	printf(str, "%f, ", vec[i]);
        sprintf(str, "%f ", vec[i]);
        cmd->send_answer(rt_server_src, str);
      }
      
      cmd->send_answer(rt_server_src, "\n");

    }
  }
  
}

// writes one vecotr to the stream
void ortd_stream::write_to_stream(void* data)
{
  rb->write(data, 1);
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
      stream->parse_and_return(cmd, rt_server_src, parstr, nElements);
      
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

  // register commands
  this->rts_thmng->add_command("stream_fetch", &ortd_callback_fetchstream__, this );
}

ortd_stream* ortd_stream_manager::new_stream(char* name, int datatype, int const_len, int numBufferElements)
{
  ortd_stream * stream = new ortd_stream(this, name, datatype, const_len, numBufferElements);
  
  return stream;
}

void ortd_stream_manager::delete_stream(ortd_stream* stream)
{
  // FIXME TODO
}


void ortd_stream_manager::destruct()
{

}

