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

extern "C" {
#include "libdyn.h"
#include "libdyn_scicos_macros.h"
#include "irpar.h"

}

#include "ortd_serial_linux.h"
#include <unistd.h>
#include <bits/pthreadtypes.h>
#include <pthread.h>
#include <string.h>

#include "signal.h"

class compu_func_serial_interface_class;

// special parser function adjust to your device

int SerialCommandFormer_arduino_1(FILE *stream, double *input, compu_func_serial_interface_class *serialblock) {
    int pwm1 = input[0]; 
    int pwm2 = input[1];
  
    fprintf(stream, "w d 8 1\r\n" 
		    "w d 9 0\r\n"
		    "w d 10 1\r\n"
		    "w d 11 0\r\n"
		    "w d 12 1\r\n"
		    
		    "w a 5 %d\r\n"
		    "w a 6 %d\r\n"
		    
		    " r all 4\r\n", pwm1, pwm2);

    double *v = input;
    printf("sending %f\n", v[0]);
}

int SerialParser_arduino_1(  char *line, int N, double * v, compu_func_serial_interface_class *serialblock) {
//   sscanf(line, "a %lf %lf %lf %lf %lf %lf", &v[0], &v[1], &v[2], &v[3], &v[4], &v[5]  );

    if (N < 15) {
        printf("error\n");
        return -1;
    }

//   printf("arduino said: %s\n", line);

    sscanf(line, "analog:  %lf %lf %lf %lf %lf %lf digital: %lf %lf %lf %lf %lf temp: %lf %lf %lf %lf",

           &v[0], &v[1], &v[2], &v[3], &v[4], &v[5],  // analog
           &v[6], &v[7], &v[8], &v[9], &v[10],	// digital
           &v[11], &v[12], &v[13], &v[14]	// temp
          );


//    printf("%f %f %f\n", v[0], v[6], v[11]);

//   v[0] = 123;
}






class serial_asynchronus_bufferedIO {
public:
    serial_asynchronus_bufferedIO(FILE *fd, void *read_buffer, int bufferElements, compu_func_serial_interface_class *serialblock);
    ~serial_asynchronus_bufferedIO();

    int thread_read_fn();

    // parser items
    va_list varead;
    void *read_buffer;
    int bufferElements;

    void lock_buf();
    void unlock_buf();
private:
//     Buffered io for writing
    FILE *fd;
    
//     Buffered io for reading
    FILE *fdread;
    
    int raw_fdread;

    pthread_t read_thread;
    pthread_mutex_t read_mutex;
    
    compu_func_serial_interface_class *serialblock;
};


void *serial_asynchronus_bufferedIO_thread_gotsig(int sig, siginfo_t *info, void *ucontext)
{
//         global_gotsig++;
    return NULL;
}

void *serial_asynchronus_bufferedIO_thread(void *data)
{
    serial_asynchronus_bufferedIO *io = (serial_asynchronus_bufferedIO *) data;

    int hdlsig = (int)SIGUSR1;

    struct sigaction sa;
    sa.sa_handler = NULL;
    sa.sa_sigaction = (void (*)(int, siginfo_t*, void*)) serial_asynchronus_bufferedIO_thread_gotsig;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);

    if (sigaction(hdlsig, &sa, NULL) < 0) {
        perror("sigaction");
        return (void*) -1;
    }


//     printf("thread ste\n");

    /*  sigset_t set;
          sigemptyset(&set);
        sigaddset(&set, SIGHUP);
    //      block out these signals
        sigprocmask(SIG_BLOCK, &set, NULL);*/

    int ret;

    do {
        ret = io->thread_read_fn();
    } while (ret == 1);

    printf("serial: exit thread\n");

    pthread_exit(NULL);

}

int serial_asynchronus_bufferedIO::thread_read_fn()
{
    char buf[512];

//   printf("read io fd = %p\n", this->fd);

    char *ret = fgets(buf, sizeof(buf), this->fdread);
//   printf("read io2\n");

    if (ret != NULL) {
//     printf("got: %s\n", ret);

        // lock
        this->lock_buf();
        SerialParser_arduino_1(ret, bufferElements, (double*) read_buffer, serialblock);
        this->unlock_buf();
        // unlock

        return 1;
    }


    return -1;
}

serial_asynchronus_bufferedIO::serial_asynchronus_bufferedIO(FILE *fd, void *read_buffer, int bufferElements, compu_func_serial_interface_class *serialblock)
{
    this->bufferElements= bufferElements;
    this->read_buffer = read_buffer;
    this->serialblock = serialblock;

    pthread_mutex_init(&read_mutex, NULL);


    this->fd = fd;

    // because it is not possible to have a pending read on the fd
    // and at the same time to write to fd within another thread
    // create a new bufferd io for reading
    int rawfd = fileno(fd);

    raw_fdread = dup(rawfd);   // FIXME this and the one belov are not lcosed!
    fdread = fdopen(raw_fdread, "r+");

    
    



    int rc = pthread_create(&read_thread, NULL, serial_asynchronus_bufferedIO_thread, (void *) this);
    if (rc)
    {
        printf("serial: ERROR; return code from pthread_create() is %d\n", rc);
        //return -1;
    }

}

void serial_asynchronus_bufferedIO::lock_buf()
{
    pthread_mutex_lock(&read_mutex);
}

void serial_asynchronus_bufferedIO::unlock_buf()
{
    pthread_mutex_unlock(&read_mutex);
}



serial_asynchronus_bufferedIO::~serial_asynchronus_bufferedIO()
{

    // FIXME: Terminate the reader thread
    printf("serial: Trying to close reader stream\n");

    pthread_kill( read_thread, SIGUSR1 );
    pthread_join( read_thread, NULL );
    
    printf("serial: joined thread\n");

    fclose( fdread );
//     printf("closed reader stream\n");

    pthread_mutex_destroy( &this->read_mutex );
//     printf("freed mutex\n");


}
















extern "C" {
    int libdyn_module_serial_interface_siminit(struct dynlib_simulation_t *sim, int bid_ofs);
}

class compu_func_serial_interface_class {
public:
    compu_func_serial_interface_class(struct dynlib_block_t *block);
    void destruct();
    void io(int update_states);
    int init();
private:
    struct dynlib_block_t *block;

    ortd_SerialPort *serial_port;
    serial_asynchronus_bufferedIO *iohelper;

    char *devname;
    char *send_cmd;
    char *rcv_cmd;

    double *rcv_buf;

    // parameter
    double inSizes;
    double outSizes;
};

compu_func_serial_interface_class::compu_func_serial_interface_class(dynlib_block_t* block)
{
    this->block = block;
}

int compu_func_serial_interface_class::init()
{
    double *rpar = libdyn_get_rpar_ptr(block);
    int *ipar = libdyn_get_ipar_ptr(block);

    inSizes = ipar[1];
    outSizes = ipar[2];

    // get parameter strings
    int BaudRate = ipar[3];

    int len_devname = ipar[7];
    int len_sendcmd = ipar[8];
    int len_rcvcmd = ipar[9];


    int *coded_devname = &ipar[10];
    int *coded_send_cmd = &ipar[10 + len_devname];
    int *coded_rcv_cmd = &ipar[10 + len_devname + len_sendcmd];
    printf("set-up \n");


    irpar_getstr(&devname, coded_devname, 0, len_devname);
    irpar_getstr(&send_cmd, coded_send_cmd, 0, len_sendcmd);
    irpar_getstr(&rcv_cmd, coded_rcv_cmd, 0, len_rcvcmd);
    printf("set-up \n");

    printf("Opening serial port %s\n", devname);
    printf("send command is: %s\n", send_cmd);
    printf("rcv command is:  %s\n", rcv_cmd);

    /*
      ALlocate receive buffer

    */
    rcv_buf = (double*) malloc(sizeof(double)*outSizes);
    memset(rcv_buf, 0, sizeof(double)*outSizes);


    //
    // Open port
    //
    int ret;
    serial_port = new ortd_SerialPort();

    ret = serial_port->serial_openport(devname);
    if (ret < 0) {
        printf("razor imu: Could not open %s\n", devname);
        delete serial_port;
        return -1;
    }

    serial_port->serial_setupport(115200, 0, 0);

    serial_port->serial_B_useBufferedIO();

//      start asyc iohelper
    double *output = (double*) libdyn_get_output_ptr(block, 0);
    iohelper = new serial_asynchronus_bufferedIO( serial_port->BufferedFD , rcv_buf, outSizes, this);

//     sleep(4);

    return 0;
}


void compu_func_serial_interface_class::io(int update_states)
{
    int ret;

    if (update_states == 1) {
        double *input = (double*) libdyn_get_input_ptr(block, 0);

	SerialCommandFormer_arduino_1(serial_port->BufferedFD, input, this);
        serial_port->serial_B_BufferFLush();
    }

    if (update_states==0) {
        double *output = (double*) libdyn_get_output_ptr(block, 0);
//         sleep(1);

	// Copy the read buffer to the blocks output
        iohelper->lock_buf();
        memcpy(output, rcv_buf, sizeof(double)*outSizes  );
        iohelper->unlock_buf();
    }
}

void compu_func_serial_interface_class::destruct()
{
    delete iohelper;

    serial_port->serial_B_close();
//     serial_port->serial_closeport();
    delete serial_port;

    free(rcv_buf);

    free(devname);
    free(send_cmd);
    free(rcv_cmd);
}


int compu_func_serial_interface(int flag, struct dynlib_block_t *block)
{

//      printf("comp_func serial_interface: flag==%d\n", flag);

    double *in;
    double *rpar = libdyn_get_rpar_ptr(block);
    int *ipar = libdyn_get_ipar_ptr(block);

    int inSize = ipar[1];
    int outSize = ipar[2];


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        in = (double *) libdyn_get_input_ptr(block,0);
        compu_func_serial_interface_class *worker = (compu_func_serial_interface_class *) libdyn_get_work_ptr(block);

        worker->io(0);

    }
    return 0;
    break;
    case COMPF_FLAG_UPDATESTATES:
    {
        in = (double *) libdyn_get_input_ptr(block,0);
        compu_func_serial_interface_class *worker = (compu_func_serial_interface_class *) libdyn_get_work_ptr(block);

        worker->io(1);

    }
    return 0;
    break;
    case COMPF_FLAG_CONFIGURE:  // configure. NOTE: do not reserve memory or open devices. Do this while init instead!
    {
        int i;
        libdyn_config_block(block, BLOCKTYPE_DYNAMIC, 1, 1, (void *) 0, 0);

        libdyn_config_block_input(block, i, inSize, DATATYPE_FLOAT);
        libdyn_config_block_output(block, i, outSize, DATATYPE_FLOAT, 0);
    }
    return 0;
    break;
    case COMPF_FLAG_INIT:  // init
    {
        compu_func_serial_interface_class *worker = new compu_func_serial_interface_class(block);
        libdyn_set_work_ptr(block, (void*) worker);

        int ret = worker->init();
        if (ret < 0)
            return -1;
    }
    return 0;
    break;
    case COMPF_FLAG_DESTUCTOR: // destroy instance
    {
        compu_func_serial_interface_class *worker = (compu_func_serial_interface_class *) libdyn_get_work_ptr(block);

        worker->destruct();
	delete worker;

    }
    return 0;
    break;
    case COMPF_FLAG_PRINTINFO:
        printf("I'm a serial_interface block\n");
        return 0;
        break;

    }
}

//#include "block_lookup.h"

int libdyn_module_serial_interface_siminit(struct dynlib_simulation_t *sim, int bid_ofs)
{

    // Register my blocks to the given simulation

    int blockid = 16001;
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &compu_func_serial_interface);

    printf("libdyn module serial_interface initialised\n");

}


//} // extern "C"
