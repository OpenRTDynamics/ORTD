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


#include <malloc.h>
#include <iostream>

extern "C" {
#include "libdyn.h"
#include "libdyn_scicos_macros.h"
#include <math.h>
#include "irpar.h"

}
#include <libdyn_cpp.h>
#include "global_shared_object.h"

#include "directory.h"

#include "PracticalSocket.h"




#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <string>
#include <stdio.h>
#include <iostream>
#include <map>
#include <unistd.h>
#include <fcntl.h>

// class udpsocket {
// private:
//     udpsocket(int port) {
//         int sockfd,n;
//         struct sockaddr_in servaddr,cliaddr;
//         char sendline[1000];
//         char recvline[1000];
// 
// 
//         sockfd=socket(AF_INET,SOCK_DGRAM,0);
// 
//         bzero(&servaddr,sizeof(servaddr));
//         servaddr.sin_family = AF_INET;
//         servaddr.sin_addr.s_addr=inet_addr(SOCK_GROUP);
//         servaddr.sin_port=htons(32000);
// 
//         while (fgets(sendline, 10000,stdin) != NULL)
//         {
//             sendto(sockfd,sendline,strlen(sendline),0,
//                    (struct sockaddr *)&servaddr,sizeof(servaddr));
//             n=recvfrom(sockfd,recvline,10000,0,NULL,NULL);
//             recvline[n]=0;
//             fputs(recvline,stdout);
//         }
//     }
// 
// 
// };



// Abgeleitete Klasse

class udp_socket_shobj : public ortd_global_shared_object {
  public:
    udp_socket_shobj(const char* identName, libdyn_master* master, int udpport, struct dynlib_simulation_t *sim) : ortd_global_shared_object(identName, master) {
        this->udpport = udpport;

	

        printf("init udp_socket\n");

        try {
            socket = new UDPSocket(udpport);
        } catch (SocketException &e)  {
            cerr << e.what() << std::endl;
//             exit(1);
        }


        libdyn_simulation_setSyncCallback(sim, &udp_socket_shobj::sync_callback, this);



    }

    int read_dval(double *dvec, int maxlen) {
        char buf[1024];

        printf("UDPSocket Doing I/O\n");

//         int rd = socket->recv(buf, sizeof(buf));
// 
//         buf[rd] = '\0';
// 
//         printf("recved: %s\n", buf);

        return 0;

    }
    
    int wait_for_Event() {
        char buf[1024];

        printf("waiting for something\n");

        int rd = socket->recv(buf, sizeof(buf));

        buf[rd] = '\0';

        printf("recved: %s\n", buf);

        return 1;
      
    }

    ~udp_socket_shobj() {
        delete socket;
    }


//   register_usage() ;
private:
    int udpport;

    UDPSocket *socket;

    
  /*
      For the lindyn sync_callback infrastruture
    */
public:
  
    static int sync_callback(struct dynlib_simulation_t * sim) {
        void * obj = sim->sync_callback.userdat;
        udp_socket_shobj *p = (udp_socket_shobj *) obj;
        return p->real_sync_callback(sim);
    }


private:

    int real_sync_callback(struct dynlib_simulation_t * sim ) {
      int ret;
      
      fprintf(stderr, "UDPSocket: Event loop started\n");
      
      do {
        ret = wait_for_Event();
	
	if (ret == 0)
	  break;
	
         // Use C-functions to simulation one timestep
         libdyn_event_trigger_mask(sim, 1);
         libdyn_simulation_step(sim, 0);
         libdyn_simulation_step(sim, 1);

	
	
      } while(true);
      
    }
    
};













class udp_main_SendRecv_block_class {
public:
    udp_main_SendRecv_block_class(struct dynlib_block_t *block);
    void destruct();
    void io_recv(int update_states);
    void io_send();
    void reset();
    int init();
private:
    struct dynlib_block_t *block;

    char * directory_entry_fname;

    int datatype;
    int size;

    udp_socket_shobj  *udpsocket;        
};

udp_main_SendRecv_block_class::udp_main_SendRecv_block_class(dynlib_block_t* block)
{
    this->block = block;
}

int udp_main_SendRecv_block_class::init()
{
    double *rpar = libdyn_get_rpar_ptr(block);
    int *ipar = libdyn_get_ipar_ptr(block);

    double Nin = ipar[0];
    double Nout = ipar[1];

//     this->ifname = "ident1_exch.ipar";
//     this->rfname = "ident1_exch.rpar";


    int udpport = ipar[0];

    this->datatype = ipar[2];
    this->size = ipar[1];

    int len_s1 = ipar[3];

    int ofs = 4;

    int *s1 = &ipar[ofs+0];

    irpar_getstr(&this->directory_entry_fname, s1, 0, len_s1);

    fprintf(stderr, "udp_main_receiver_block_class: fname is %s\n", directory_entry_fname);


    libdyn_master *master = (libdyn_master *) block->sim->master;
    udpsocket = new udp_socket_shobj( directory_entry_fname, master, udpport, block->sim );


    /*    udp_main_receiver_init meminit;
        pmem = meminit.udp_main_receiver_register(master, this->directory_entry_fname);*/


    return 0;
}


void udp_main_SendRecv_block_class::io_recv(int update_states)
{
    if (update_states==0) {
        double vec[10];

        udpsocket->read_dval(vec, 10);

    }
}

void udp_main_SendRecv_block_class::io_send()
{
//   udpsocket->
}

void udp_main_SendRecv_block_class::reset()
{

}


void udp_main_SendRecv_block_class::destruct()
{
    libdyn_master *master = (libdyn_master *) block->sim->master;


    udpsocket->unregister_usage();
    free(directory_entry_fname);
}


int udp_main_receiver_block(int flag, struct dynlib_block_t *block)
{

//     printf("comp_func nested_exchange_fromfile: flag==%d\n", flag);

    double *in;
    double *rpar = libdyn_get_rpar_ptr(block);
    int *ipar = libdyn_get_ipar_ptr(block);

    int Nin = 0;
    int Nout = 1;

    int outsize = ipar[1];


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        in = (double *) libdyn_get_input_ptr(block,0);
        udp_main_SendRecv_block_class *worker = (udp_main_SendRecv_block_class *) libdyn_get_work_ptr(block);

        worker->io_recv(0);



    }
    return 0;
    break;
    case COMPF_FLAG_UPDATESTATES:
    {
        in = (double *) libdyn_get_input_ptr(block,0);
        udp_main_SendRecv_block_class *worker = (udp_main_SendRecv_block_class *) libdyn_get_work_ptr(block);

        worker->io_recv(1);

    }
    return 0;
    break;
    case COMPF_FLAG_CONFIGURE:  // configure. NOTE: do not reserve memory or open devices. Do this while init instead!
    {


//      int datatype = ipar[1];
        int datatype = DATATYPE_FLOAT; // ipar[2]
        int size = ipar[2];

        libdyn_config_block(block, BLOCKTYPE_DYNAMIC, Nout, Nin, (void *) 0, 0);

//         libdyn_config_block_input(block, 0, 1, DATATYPE_FLOAT);
//         libdyn_config_block_input(block, 1, 1, DATATYPE_FLOAT);
        libdyn_config_block_output(block, 0, outsize, datatype, 1);


    }
    return 0;
    break;
    case COMPF_FLAG_PREINIT:  // init
    {
        udp_main_SendRecv_block_class *worker = new udp_main_SendRecv_block_class(block);
        libdyn_set_work_ptr(block, (void*) worker);

        int ret = worker->init();
        if (ret < 0)
            return -1;
    }
    return 0;
    break;
    case COMPF_FLAG_RESETSTATES:
    {
        in = (double *) libdyn_get_input_ptr(block,0);
        udp_main_SendRecv_block_class *worker = (udp_main_SendRecv_block_class *) libdyn_get_work_ptr(block);

        worker->reset();
    }
    return 0;
    break;
    case COMPF_FLAG_PREINITUNDO: // destroy instance
    {
        udp_main_SendRecv_block_class *worker = (udp_main_SendRecv_block_class *) libdyn_get_work_ptr(block);

        worker->destruct();
        delete worker;

    }
    return 0;
    break;
    case COMPF_FLAG_PRINTINFO:
        printf("I'm a udp_main_receiver initialising block\n");
        return 0;
        break;

    }
}

int udp_main_sender_block(int flag, struct dynlib_block_t *block)
{

//     printf("comp_func nested_exchange_fromfile: flag==%d\n", flag);

    double *in;
    double *rpar = libdyn_get_rpar_ptr(block);
    int *ipar = libdyn_get_ipar_ptr(block);

    int Nin = 1;
    int Nout = 0;

    int outsize = ipar[1];


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        in = (double *) libdyn_get_input_ptr(block,0);
        udp_main_SendRecv_block_class *worker = (udp_main_SendRecv_block_class *) libdyn_get_work_ptr(block);

//         worker->io_send();
    }
    return 0;
    break;
    case COMPF_FLAG_UPDATESTATES:
    {
        in = (double *) libdyn_get_input_ptr(block,0);
        udp_main_SendRecv_block_class *worker = (udp_main_SendRecv_block_class *) libdyn_get_work_ptr(block);

         worker->io_send();

    }
    return 0;
    break;
    case COMPF_FLAG_CONFIGURE:  // configure. NOTE: do not reserve memory or open devices. Do this while init instead!
    {


//      int datatype = ipar[1];
        int datatype = DATATYPE_FLOAT; // ipar[2]
        int size = ipar[2];

        libdyn_config_block(block, BLOCKTYPE_DYNAMIC, Nout, Nin, (void *) 0, 0);

         libdyn_config_block_input(block, 0, 1, DATATYPE_FLOAT);
//         libdyn_config_block_input(block, 1, 1, DATATYPE_FLOAT);
//         libdyn_config_block_output(block, 0, outsize, datatype, 1);


    }
    return 0;
    break;
    case COMPF_FLAG_PREINIT:  // init
    {
        udp_main_SendRecv_block_class *worker = new udp_main_SendRecv_block_class(block);
        libdyn_set_work_ptr(block, (void*) worker);

        int ret = worker->init();
        if (ret < 0)
            return -1;
    }
    return 0;
    break;
    case COMPF_FLAG_RESETSTATES:
    {
        in = (double *) libdyn_get_input_ptr(block,0);
        udp_main_SendRecv_block_class *worker = (udp_main_SendRecv_block_class *) libdyn_get_work_ptr(block);

        worker->reset();
    }
    return 0;
    break;
    case COMPF_FLAG_PREINITUNDO: // destroy instance
    {
        udp_main_SendRecv_block_class *worker = (udp_main_SendRecv_block_class *) libdyn_get_work_ptr(block);

        worker->destruct();
        delete worker;

    }
    return 0;
    break;
    case COMPF_FLAG_PRINTINFO:
        printf("I'm a udp_main_sender initialising block\n");
        return 0;
        break;

    }
}









// Export to C so the libdyn simulator finds this function
extern "C" int libdyn_module_udp_communication_siminit(struct dynlib_simulation_t *sim, int bid_ofs);

int libdyn_module_udp_communication_siminit(struct dynlib_simulation_t *sim, int bid_ofs)
{

    // Register my blocks to the given simulation

    int blockid = 39001;
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &udp_main_receiver_block);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid+1, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &udp_main_sender_block);

    printf("libdyn module udp-communication initialised\n");

}


//} // extern "C"
