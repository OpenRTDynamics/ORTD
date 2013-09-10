/*
    Copyright (C) 2010, 2011, 2012, 2013  Christian Klauer

    This file is part of OpenRTDynamics, the Real-Time Dynamics Framework

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

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <strings.h>

extern "C" {
#include "libdyn.h"
#include "libdyn_scicos_macros.h"
#include <math.h>
#include "irpar.h"

}
#include <libdyn_cpp.h>
#include "global_shared_object.h"

#include "directory.h"





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

class udpsocket {
public:
    int sockfd,n;
    struct sockaddr_in servaddr, broadcastAddr;

    udpsocket(char *hostname, int port) {
        // Set up destination
        struct hostent *hostinfo;
        hostinfo = gethostbyname (hostname);
        if (hostinfo == NULL)
        {
            fprintf (stderr, "Unknown host %s.\n", hostname);
            throw -1;
        }

        // FIXME: undecided what to to with the hostname
        
        
        bzero(&broadcastAddr,sizeof(broadcastAddr));  
        broadcastAddr.sin_family = AF_INET;
//         broadcastAddr.sin_addr.s_addr=htonl(INADDR_ANY);
        broadcastAddr.sin_port=htons(port);
        broadcastAddr.sin_addr = *(struct in_addr *) hostinfo->h_addr;  // FIXME: not a broadcastAddr


        // open socket
        sockfd=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);

        int broadcastEnable=1;
        int ret=setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));


	if (port != -1) {
        bzero(&servaddr,sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
        servaddr.sin_port=htons(port);

        if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))==-1) {
	   fprintf (stderr, "could not bind to socket\n");
	   close(sockfd);
           throw 1;
	}
	} else {
	  fprintf (stderr, "Not binding to an UDP-socket\n");
	}


    }
    ~udpsocket()
    {
        close(sockfd);
    }

    void SendPacket(void *buf, int N) {
//         printf("send packet of %d bytes using socket %d:\n", N, sockfd);
        sendto(sockfd,buf,N,0,(struct sockaddr *)&broadcastAddr,sizeof(broadcastAddr));
    }

    void SendToPacket(struct sockaddr_in DestAddr, void *buf, int N) {
//         printf("send packet of %d bytes using socket %d:\n", N, sockfd);
        sendto(sockfd,buf,N,0,(struct sockaddr *)&DestAddr,sizeof(DestAddr));
    }

    int receive(void *buf, int buflen) {
          struct sockaddr_in si_from;
          socklen_t slen = sizeof(si_from);
          if (recvfrom(sockfd, buf, buflen, 0, (struct sockaddr *) &si_from, &slen)==-1) {
	    return -1;
	  }
           
//           printf("Received packet from %s:%d\nData: %s\n\n",  inet_ntoa(si_from.sin_addr), ntohs(si_from.sin_port), buf); 
	  
	  return 0;
    }
    
    



};

//
// A shared object
//
class UDPSocket_SharedObject : public ortd_global_shared_object {
public:
    UDPSocket_SharedObject(const char* identName, dynlib_block_t *block, int Visibility) : ortd_global_shared_object(identName, block->sim, Visibility) {
        data = 1;
        this->block = block;

        // Get the irpar parameters Uipar, Urpar
        libdyn_AutoConfigureBlock_GetUirpar(block, &Uipar, &Urpar);
    }

    ~UDPSocket_SharedObject() {

    }

    int init() {
        printf("Init of UDPSocket_SharedObject\n");

        struct irpar_ivec_t vec;
        if ( irpar_get_ivec(&vec, Uipar, Urpar, 10) < 0 ) {
            return -1;
        }
        udpport = vec.v[0];

//         printf("Using udp port %d\n", udpport);

        //
        // get a string (not so nice by now)
        //
        struct irpar_ivec_t hostname_;
        char *hostname;
        if ( irpar_get_ivec(&hostname_, Uipar, Urpar, 11) < 0 ) { 
	  return -1;	  
	}
        irpar_getstr(&hostname, hostname_.v, 0, hostname_.n);

        printf("destination hostname = %s, port = %d\n", hostname, udpport);

	try {
	  socket = new udpsocket(hostname, udpport);
	} catch (int e) {
	  free(hostname);
	  return -1;
	}

        free(hostname); // do not forget to free the memory allocated by irpar_getstr

	
// 	char hostname[] = "localhost";
	

        return 0; // init was ok
    }

    void send(void *buf, uint N) {
        socket->SendPacket(buf, N);
    }

    void sendTo(struct sockaddr_in DestAddr, void *buf, uint N) {
        socket->SendToPacket(DestAddr, buf, N);
    }

    int *Uipar;
    double *Urpar;
    dynlib_block_t *block;


    /*
        Some data of the shared object
    */
private:

    int data;
    int udpport;

public:
    udpsocket *socket;


};




//
// An example block for accessing the shared object
//
class UDPSocket_Send_Block {
public:
    UDPSocket_Send_Block(struct dynlib_block_t *block) {
        this->block = block;    //  nothing more here. The real initialisation take place in init()
    }
    ~UDPSocket_Send_Block()
    {
        // free your allocated memory, ...
    }

    //
    // define states or other variables
    //

    UDPSocket_SharedObject *IShObj; // instance of the shared object


    uint NCopyBytes;

    //
    // initialise your block
    //

    int init() {
        printf("Init of UDPSocket_Send_Block\n");

        int *Uipar;
        double *Urpar;

        // Get the irpar parameters Uipar, Urpar
        libdyn_AutoConfigureBlock_GetUirpar(block, &Uipar, &Urpar);

        //
        // extract some structured sample parameters
        //

// 	fprintf(stderr, "Trying to get shared obj\n");
        // Obtain the shared object
        IShObj = NULL;
        if ( ortd_GetSharedObj<UDPSocket_SharedObject>(block, &IShObj) < 0 ) {
            return -1;
        }
//         fprintf(stderr, "Trying to get shared obj : %p -- ok\n", IShObj);


        // set the initial states
        resetStates();

        // calc how many bytes shall be copied
        int N = libdyn_get_inportsize(block, 0);
        int datatype = libdyn_get_inportdatatype(block, 0);
        int TypeBytes = libdyn_config_get_datatype_len(datatype);
        NCopyBytes = N * TypeBytes;

        // Return -1 to indicate an error, so the simulation will be destructed
        return 0;
    }


    inline void updateStates()
    {
    }


    inline void calcOutputs()
    {
        void *in1 = (void *) libdyn_get_input_ptr(block,0); // the first input port
//         printf("Sending %d Bytes\n", NCopyBytes);

        // call a function of the shared object
        IShObj->send(in1, NCopyBytes);
    }


    inline void resetStates()
    {
    }


    void printInfo() {
        fprintf(stderr, "I'm a UDP Send block\n");
    }

    // uncommonly used flags
    void PrepareReset() {}
    void HigherLevelResetStates() {}
    void PostInit() {}


    // The Computational function that is called by the simulator
    // and that distributes the execution to the various functions
    // in this C++ - Class, including: init(), io(), resetStates() and the destructor
    static int CompFn(int flag, struct dynlib_block_t *block) {
        return LibdynCompFnTempate<UDPSocket_Send_Block>( flag, block ); // this expands a template for a C-comp fn
    }

    // The data for this block managed by the simulator
    struct dynlib_block_t *block;
};



/*
 * UDP-Receiver
*/

class UDPReceiverBlock {
public:
    UDPReceiverBlock(struct dynlib_block_t *block) {
        this->block = block;    // no nothing more here. The real initialisation take place in init()
    }
    ~UDPReceiverBlock()
    {
        // free your allocated memory, ...
    }

    //
    // define states or other variables
    //

    bool ExitLoop;
    int NCopyBytes;
    UDPSocket_SharedObject *IShObj; // instance of the shared object

    //
    // initialise your block
    //

    int init() {
        int *Uipar;
        double *Urpar;
	
	fprintf(stderr, "UDP-Read init\n");

        // Get the irpar parameters Uipar, Urpar
        libdyn_AutoConfigureBlock_GetUirpar(block, &Uipar, &Urpar);


        // register the callback function to the simulator that shall trigger the simulation while running in a loop
        libdyn_simulation_setSyncCallback(block->sim, &syncCallback_ , this);
        libdyn_simulation_setSyncCallbackDestructor(block->sim, &syncCallbackDestructor_ , this);
	
	ExitLoop = false;


        // Get the UDP-Socket
        IShObj = NULL;
        if ( ortd_GetSharedObj<UDPSocket_SharedObject>(block, &IShObj) < 0 ) {
            return -1;
        }
// 		printf("****************** UDP-Read 1\n");
//         fprintf(stderr, "Trying to get UDP Socket: %s \n", IShObj);


        // set the initial states
        resetStates();

        // calc how many bytes shall be copied
        int N = libdyn_get_outportsize(block, 0);
        int datatype = libdyn_get_outportdatatype(block, 0);
        int TypeBytes = libdyn_config_get_datatype_len(datatype);
        NCopyBytes = N * TypeBytes;

// 	printf("****************** UDP-Read init fin\n");
	
	
        // Return -1 to indicate an error, so the simulation will be destructed
        return 0;
    }


    inline void updateStates()
    {
//         double *output = (double*) libdyn_get_output_ptr(block, 0); // the first output port

    }


    inline void calcOutputs()
    {
//         double *output = (double*) libdyn_get_output_ptr(block, 0); // the first output port

	// Put out the sensor value
     //   printf("output calc\n");

    }


    inline void resetStates()
    {
    }


    
    int SyncCallback( struct dynlib_simulation_t * sim )
    {
        /*
         *		***	MAIN FUNCTION	***
         *
         * This function is called before any of the output or state-update flags
         * are called.
         * If 0 is returned, the simulation will continue to run
         * If 1 is returned, the simulation will pause and has to be re-triggered externally.
         * e.g. by the trigger_computation input of the async nested_block.
        */

        fprintf(stderr, "Threaded simulation synchronised to an UDP-receiver started execution\n");

//     // wait until the callback function  real_syncDestructor_callback is called
//     pthread_mutex_lock(&ExitMutex);
//

	double *output = (double*) libdyn_get_output_ptr(block, 0); // the first output port
	
	// This is the main loop of the new simulation
        while (!ExitLoop) {
//           printf("wait for UDP Data to receive\n");
//             sleep(1); // wait for dummy sensor
	    IShObj->socket->receive( output, NCopyBytes );

            // run one step of the ortd simulator
            libdyn_event_trigger_mask(sim, 1);
            libdyn_simulation_step(sim, 0);
            libdyn_simulation_step(sim, 1);
        }


        return 1; // 1 - this shall not be executed again, directly after returning from this function!
    }

    int SyncDestructorCallback( struct dynlib_simulation_t * sim )
    {
        printf("The Block containing simulation is about to be destructed\n");

	// Trigger termination of the the main loop
	ExitLoop = true;
//     pthread_mutex_unlock(&ExitMutex);

    }


    void printInfo() {
        fprintf(stderr, "I'm a Template block\n");
    }
    
    // uncommonly used flags
    void PrepareReset() {}
    void HigherLevelResetStates() {}
    void PostInit() {}


    // The Computational function that is called by the simulator
    // and that distributes the execution to the various functions
    // in this C++ - Class, including: init(), io(), resetStates() and the destructor
    static int CompFn(int flag, struct dynlib_block_t *block) {
        return LibdynCompFnTempate<UDPReceiverBlock>( flag, block ); // this expands a template for a C-comp fn
    }
    static int syncCallback_(struct dynlib_simulation_t * sim) {
        void * obj = sim->sync_callback.userdat;
        UDPReceiverBlock *p = (UDPReceiverBlock *) obj;
        return p->SyncCallback(sim);
    }
    static int syncCallbackDestructor_(struct dynlib_simulation_t * sim) {
        void * obj = sim->sync_callback.userdatDestructor;
        UDPReceiverBlock *p = (UDPReceiverBlock *) obj;
        return p->SyncDestructorCallback(sim);
    }

    // The data for this block managed by the simulator
    struct dynlib_block_t *block;
};




//
// An example block for accessing the shared object
//
class UDPSocket_SendTo_Block {
public:
    UDPSocket_SendTo_Block(struct dynlib_block_t *block) {
        this->block = block;    //  nothing more here. The real initialisation take place in init()
    }
    ~UDPSocket_SendTo_Block()
    {
        // free your allocated memory, ...
    }

    //
    // define states or other variables
    //

    UDPSocket_SharedObject *IShObj; // instance of the shared object


    uint NCopyBytes;
    int port;
    struct sockaddr_in DestAddr;

    //
    // initialise your block
    //

    int init() {
        printf("Init of UDPSocket_SendTo_Block\n");

        int *Uipar;
        double *Urpar;

        // Get the irpar parameters Uipar, Urpar
        libdyn_AutoConfigureBlock_GetUirpar(block, &Uipar, &Urpar);

        //
        // extract some structured sample parameters
        //

// 	fprintf(stderr, "Trying to get shared obj\n");
        // Obtain the shared object
        IShObj = NULL;
        if ( ortd_GetSharedObj<UDPSocket_SharedObject>(block, &IShObj) < 0 ) {
            return -1;
        }
//         fprintf(stderr, "Trying to get shared obj : %p -- ok\n", IShObj);


        try {
	  irpar_ivec veccpp(Uipar, Urpar, 12); // then use:  veccpp.n; veccpp.v;
	  port = veccpp.v[0];
	  printf("SendTo port = %d\n", port); // print the first element 
	                                           // of the vector that is of size veccpp.n
	} catch(int e) {
	  // parameter not found
	  port=0;
	}

	//
	irpar_string s(Uipar, Urpar, 13);
	const char *hostname  = s.s->c_str() ;
	printf("SendTo hostname = %s\n", hostname);
	
	
        // get IP
        struct hostent *hostinfo;
        hostinfo = gethostbyname (hostname);
        if (hostinfo == NULL)
        {
            fprintf (stderr, "Unknown host %s.\n", hostname);	    
            return -1;
        }
	
        // Set-up destination
        bzero(&DestAddr,sizeof(DestAddr));
        DestAddr.sin_family = AF_INET;
        DestAddr.sin_port=htons(port);
        DestAddr.sin_addr = *(struct in_addr *) hostinfo->h_addr;

	
        // set the initial states
        resetStates();

        // calc how many bytes shall be copied
        int N = libdyn_get_inportsize(block, 0);
        int datatype = libdyn_get_inportdatatype(block, 0);
        int TypeBytes = libdyn_config_get_datatype_len(datatype);
        NCopyBytes = N * TypeBytes;

        // Return -1 to indicate an error, so the simulation will be destructed
        return 0;
    }


    inline void updateStates()
    {
    }


    inline void calcOutputs()
    {
        void *in1 = (void *) libdyn_get_input_ptr(block,0); // the first input port
        uint32_t *Nb = (uint32_t *) libdyn_get_input_ptr(block,1); 
	
	if (*Nb <= NCopyBytes) {
//          printf("Sending %d Bytes\n", *Nb);

        // call a function of the shared object
        IShObj->sendTo(DestAddr, in1, *Nb);
	} else {
	  fprintf(stderr, "UDPSendTo: ERROR: The Number of bytes to send is too big!\n");
	}
    }


    inline void resetStates()
    {
    }


    void printInfo() {
        fprintf(stderr, "I'm a UDP SendTo block\n");
    }

    // uncommonly used flags
    void PrepareReset() {}
    void HigherLevelResetStates() {}
    void PostInit() {}


    // The Computational function that is called by the simulator
    // and that distributes the execution to the various functions
    // in this C++ - Class, including: init(), io(), resetStates() and the destructor
    static int CompFn(int flag, struct dynlib_block_t *block) {
        return LibdynCompFnTempate<UDPSocket_SendTo_Block>( flag, block ); // this expands a template for a C-comp fn
    }

    // The data for this block managed by the simulator
    struct dynlib_block_t *block;
};







class ConcateDataBlock {
public:
    ConcateDataBlock(struct dynlib_block_t *block) {
        this->block = block;    // no nothing more here. The real initialisation take place in init()
    }
    ~ConcateDataBlock()
    {
        // free your allocated memory, ...
        free(SizesList);
    }

    uint *SizesList, Ninports;
    
    //
    // initialise your block
    //

    int init() {
        int *Uipar;
        double *Urpar;

        // Get the irpar parameters Uipar, Urpar
        libdyn_AutoConfigureBlock_GetUirpar(block, &Uipar, &Urpar);

        //
        // extract some structured sample parameters
        //
        int error = 0;

     
	int i, NGesbytes = 0;
	Ninports = libdyn_get_Ninports(block);
	
	SizesList = (uint*) malloc( sizeof(uint) * Ninports );
	
	for (i = 0; i<Ninports; ++i) {
	  int N = libdyn_get_inportsize(block, i); 
	  int datatype = libdyn_get_inportdatatype(block, i); // the datatype
	  int TypeBytes = libdyn_config_get_datatype_len(datatype); // number of bytes allocated for one element of type "datatype"
	  int NBytes = N * TypeBytes;  // Amount of bytes allocated for the input vector
	  
	  SizesList[i] = NBytes;
	  NGesbytes += NBytes;
	}
	
	
	
        //
        // get some informations on the first output port
 	//
	int N = libdyn_get_outportsize(block, 0);  // the size of the first (=0) input vector
	int datatype = libdyn_get_outportdatatype(block, 0); // the datatype
	int TypeBytes = libdyn_config_get_datatype_len(datatype); // number of bytes allocated for one element of type "datatype"
	int NBytesOut = N * TypeBytes;  // Amount of bytes allocated for the input vector
	
	if (NBytesOut != NGesbytes) {
	  fprintf(stderr, "ConcateDataBlock: NBytesOut != NGesbytes\n");
	  free(SizesList);
	  
	  return -1;
	}
	
        // set the initial states
        resetStates();

        // Return -1 to indicate an error, so the simulation will be destructed
        return error;
    }


    inline void updateStates()
    {

    }


    inline void calcOutputs()
    {
        char *output = (char*) libdyn_get_output_ptr(block, 0); // the first output port


	uint i, outPtr=0;
        for (i=0; i<Ninports; ++i) {
	  void *in = (void*) libdyn_get_input_ptr(block, i);

//  	  printf("copy %d bytes\n", SizesList[i] );
	  memcpy( output + outPtr, in, SizesList[i]  );
	  
	  outPtr += SizesList[i];
	}
	
    }


    inline void resetStates()    {    }

    void printInfo() {
        fprintf(stderr, "I'm a ConcateData block\n");
    }

    // uncommonly used flags
    void PrepareReset() {}
    void HigherLevelResetStates() {}
    void PostInit() {}


    // The Computational function that is called by the simulator
    // and that distributes the execution to the various functions
    // in this C++ - Class, including: init(), io(), resetStates() and the destructor
    static int CompFn(int flag, struct dynlib_block_t *block) {
        return LibdynCompFnTempate<ConcateDataBlock>( flag, block ); // this expands a template for a C-comp fn
    }

    // The data for this block managed by the simulator
    struct dynlib_block_t *block;
};



class DisassembleDataBlock {
public:
    DisassembleDataBlock(struct dynlib_block_t *block) {
        this->block = block;    // no nothing more here. The real initialisation take place in init()
    }
    ~DisassembleDataBlock()
    {
        // free your allocated memory, ...
        free(SizesList);
    }

    uint *SizesList, Noutports;
    
    //
    // initialise your block
    //

    int init() {
        int *Uipar;
        double *Urpar;

        // Get the irpar parameters Uipar, Urpar
        libdyn_AutoConfigureBlock_GetUirpar(block, &Uipar, &Urpar);

        //
        // extract some structured sample parameters
        //
        int error = 0;

     
	int i, NGesbytes = 0;
	Noutports = libdyn_get_Noutports(block);
	
	SizesList = (uint*) malloc( sizeof(uint) * Noutports );
	
	for (i = 0; i<Noutports; ++i) {
	  int N = libdyn_get_outportsize(block, i); 
	  int datatype = libdyn_get_outportdatatype(block, i); // the datatype
	  int TypeBytes = libdyn_config_get_datatype_len(datatype); // number of bytes allocated for one element of type "datatype"
	  int NBytes = N * TypeBytes;  // Amount of bytes allocated for the input vector
	  
	  SizesList[i] = NBytes;
	  NGesbytes += NBytes;
	}
	
	
	
        //
        // get some informations on the first input port
 	//
	int N = libdyn_get_inportsize(block, 0);  // the size of the first (=0) input vector
	int datatype = libdyn_get_inportdatatype(block, 0); // the datatype
	int TypeBytes = libdyn_config_get_datatype_len(datatype); // number of bytes allocated for one element of type "datatype"
	int NBytesIn = N * TypeBytes;  // Amount of bytes allocated for the input vector
	
	if (NBytesIn != NGesbytes) {
	  fprintf(stderr, "DisassembleDataBlock: NBytesIn != NGesbytes\n");
	  free(SizesList);
	  
	  return -1;
	}
	
        // set the initial states
        resetStates();

        // Return -1 to indicate an error, so the simulation will be destructed
        return error;
    }


    inline void updateStates()
    {

    }


    inline void calcOutputs()
    {
        char *input = (char*) libdyn_get_input_ptr(block, 0); // the first output port


	uint i, inPtr=0;
        for (i=0; i<Noutports; ++i) {
	  void *out = (void*) libdyn_get_output_ptr(block, i);

//  	  printf("copy %d bytes\n", SizesList[i] );
	  memcpy( out, input + inPtr, SizesList[i]  );
	  
	  inPtr += SizesList[i];
	}
	
    }


    inline void resetStates()    {    }

    void printInfo() {
        fprintf(stderr, "I'm a DisassembleDataBlock block\n");
    }

    // uncommonly used flags
    void PrepareReset() {}
    void HigherLevelResetStates() {}
    void PostInit() {}


    // The Computational function that is called by the simulator
    // and that distributes the execution to the various functions
    // in this C++ - Class, including: init(), io(), resetStates() and the destructor
    static int CompFn(int flag, struct dynlib_block_t *block) {
        return LibdynCompFnTempate<DisassembleDataBlock>( flag, block ); // this expands a template for a C-comp fn
    }

    // The data for this block managed by the simulator
    struct dynlib_block_t *block;
};





extern "C" int libdyn_module_udp_communication_siminit(struct dynlib_simulation_t *sim, int bid_ofs)
{

    // Register my blocks to the given simulation

    int blockid = 39001;

    // Blocks from Template_SharedObjects.cpp. Uncomment if not needed
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid+0, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &SharedObjBlock<UDPSocket_SharedObject>::CompFn);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid+1, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &UDPSocket_Send_Block::CompFn);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid+2, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &UDPReceiverBlock::CompFn);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid+3, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &UDPSocket_SendTo_Block::CompFn);

    
    
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid+10, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &ConcateDataBlock::CompFn);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid+11, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &DisassembleDataBlock::CompFn);
    
    
    
    
    printf("libdyn module udp-communication initialised\n");

}


