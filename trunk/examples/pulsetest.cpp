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


// To compile do: sudo apt-get install libpulse-dev

#include <stdlib.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>

#include <sys/time.h>
#include <sys/types.h>

#include <pthread.h>


#include <pulse/simple.h>
#include <pulse/error.h>
#include <pulse/gccmacro.h>

#define BUFSIZE 100 *300

#include "libdyn_cpp.h"


extern "C" {
#include "irpar.h"

}


#define STDIN 0  // file descriptor for standard input

void changemode ( int dir ) {
    static struct termios oldt, newt;

    if ( dir == 1 ) {
        tcgetattr ( STDIN_FILENO, &oldt );
        newt = oldt;
        newt.c_lflag &= ~ ( ICANON | ECHO );
        tcsetattr ( STDIN_FILENO, TCSANOW, &newt );
    } else
        tcsetattr ( STDIN_FILENO, TCSANOW, &oldt );
}

int kbhit ( void ) {
    struct timeval tv;
    fd_set rdfs;

    tv.tv_sec = 0;
    tv.tv_usec = 0;

    FD_ZERO ( &rdfs );
    FD_SET ( STDIN_FILENO, &rdfs );

    select ( STDIN_FILENO+1, &rdfs, NULL, NULL, &tv );
    return FD_ISSET ( STDIN_FILENO, &rdfs );

}


char get_key() {
    struct timeval tv;
    fd_set readfds;

    tv.tv_sec = 0; //2;
    tv.tv_usec = 0; //500000;

    FD_ZERO ( &readfds );
    FD_SET ( STDIN, &readfds );

    // don't care about writefds and exceptfds:
    select ( STDIN+1, &readfds, NULL, NULL, &tv );

    if ( FD_ISSET ( STDIN, &readfds ) ) {
        //printf("A key was pressed!\n");
        char keys[1024];
        int numRead = read ( STDIN, &keys, 1024 );
        printf ( "%d keys last one is %c %x\n", numRead, keys[numRead-1], keys[numRead-1] );
        return keys[numRead-1];
    } else {
        return 0;   //printf("Timed out.\n");
    }

}

int decode_key ( char key, char *keytable, int keytable_len ) {
    int i;

    for ( i = 0; i < keytable_len; ++i ) {
        if ( keytable[i] == key )
            return i;
    }

    return -1;
}

struct sound_thread_data_t {
    pthread_mutex_t init_mutex;
    pthread_mutex_t buffer1_mutex;
    pthread_mutex_t buffer2_mutex;
    pthread_t sound_thread;
    int16_t buf1[BUFSIZE];
    int16_t buf2[BUFSIZE];
    int activeToWrite_buffer;
    int activeToPlay_buffer;

};

extern "C" {

    void *sound_thread(void *data)
    {
        struct sound_thread_data_t *sound_thread_data = (struct sound_thread_data_t *) data;
        printf("Sound Thread started\n");


        //
        // Open Pulseaudio
        //

        /* The Sample format to use */

        pa_sample_spec audiocfg;
        audiocfg.format = PA_SAMPLE_S16LE;
        audiocfg.rate = 44100;
        audiocfg.channels = 1;

        pa_simple *s = NULL;
        int error;


        /* Create a new playback stream */
        if ( ! ( s = pa_simple_new ( NULL, "libdyntest", PA_STREAM_PLAYBACK, NULL, "playback", &audiocfg, NULL, NULL, &error ) ) ) {
            fprintf ( stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror ( error ) );
            exit ( 0 );
        }




// notify main loop

        pthread_mutex_unlock(&sound_thread_data->init_mutex);


        ssize_t r = BUFSIZE;
        int16_t *bufferToPlay;
	sound_thread_data->activeToPlay_buffer = 2;


        do {

            if (sound_thread_data->activeToPlay_buffer == 2) { // in buf 1 wird gerade geschrieben, spiele buffer 2
//                 printf("waiting for buffer 2\n");
                pthread_mutex_lock(&sound_thread_data->buffer2_mutex);
                bufferToPlay = sound_thread_data->buf2;
//                 printf("playing buffer 2\n");
            } else {
//                 printf("waiting for buffer 1\n");
                pthread_mutex_lock(&sound_thread_data->buffer1_mutex);
                bufferToPlay = sound_thread_data->buf1;
//                 printf("playing buffer 1\n");
            }

            //
            // Play it
            //




//             printf ( "start sample\n" );

            /* ... and play it */
            if ( pa_simple_write ( s, bufferToPlay, ( size_t ) r, &error ) < 0 ) {
                fprintf ( stderr, __FILE__": pa_simple_write() failed: %s\n", pa_strerror ( error ) );
//                 goto finish;
            }


    /* Make sure that every single sample was played */
/*    if (pa_simple_drain(s, &error) < 0) {
        fprintf(stderr, __FILE__": pa_simple_drain() failed: %s\n", pa_strerror(error));
        goto finish;
    }*/
    
//             printf ( "end sample\n" );

	     if (sound_thread_data->activeToPlay_buffer == 2) {
                pthread_mutex_unlock(&sound_thread_data->buffer2_mutex);
		sound_thread_data->activeToPlay_buffer = 1;
	       
	     } else {
                pthread_mutex_unlock(&sound_thread_data->buffer1_mutex);
		sound_thread_data->activeToPlay_buffer = 2;
	       
	     }
	    
        } while (true);


finish:

        if ( s )
            pa_simple_free ( s );



        printf("running\n");
    }


    int main() {
        //
        // Create a thread for playing sound
        //
        struct sound_thread_data_t sound_thread_data;
        sound_thread_data.activeToWrite_buffer = 1; //

        pthread_mutex_init(&sound_thread_data.init_mutex, NULL);
        pthread_mutex_init(&sound_thread_data.buffer1_mutex, NULL);
        pthread_mutex_init(&sound_thread_data.buffer2_mutex, NULL);

        pthread_mutex_lock(&sound_thread_data.init_mutex);
//         pthread_mutex_lock(&sound_thread_data.buffer1_mutex);
        pthread_mutex_lock(&sound_thread_data.buffer2_mutex); // sound_thread will try to play buffer 2 but has to wait initially due to this

        int rc = pthread_create(&sound_thread_data.sound_thread, NULL, sound_thread, (void *) &sound_thread_data);
        if (rc) {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(0);
        }




        changemode ( 1 );

        char keytable[13] = {0x1b, 'q', 'w', 'e', 'r', 't', 'z', 'u', 'i', 'o', 'p', 0xbc, '+' };
        char key;

        int i;



        //for (;;) sleep(1);
        //
        // Open libdyn schematic
        //

        int insizes[] = {1,1,1,1,1,1,1,1,1,1,1,1};
        int outsizes[] = {1};

        libdyn * simbox = new libdyn ( 12, insizes, 1, outsizes );

        struct {
            double keys[12];
        } inputs;


        int *ipar_cpy; // pointer to integer parameter list
        double *rpar_cpy; // pointer to double parameter list
        int Nipar;  // length of ipar list
        int Nrpar;  // length of rpar list

        /*
        *  Load parameters from file
        */

        char *fname_i = "pulsetest.ipar";
        char *fname_r = "pulsetest.rpar";

        irpar_load_from_afile ( &ipar_cpy, &rpar_cpy, &Nipar, &Nrpar, fname_i, fname_r );




        int err;

        printf ( ":: Setting up Box\n----------------------\n" );

        for ( i=0; i<12; ++i )
            simbox->cfg_inptr ( i, &inputs.keys[i] );
        //simbox->cfg_inptr(1, &inputs.in2);


        /*  __libdyn_irpar_cfgports(dyn_adapt, Ninp_ADAPT, Noutp_ADAPT, INsizes_ADAPT, OUTsizes_ADAPT);
          __libdyn_irpar_cfg_inptr(dyn_adapt, 0, &acc_inputs.r);
          __libdyn_irpar_cfg_inptr(dyn_adapt, 1, &acc_inputs.y);*/

        int schematic_id = 901;
        err = simbox->irpar_setup ( ipar_cpy, rpar_cpy, schematic_id );

//   err = __libdyn_irpar_setup(dyn_adapt, ipar_cpy, rpar_cpy, 901);

        if ( err == -1 ) {
            // There may be some problems during compilation.
            // Errors are reported on stdout
            printf ( "Error in libdyn\n" );
            exit ( 1 );
        } else {


            int i, stepc;
            stepc = 0;

            int16_t * bufferToWrite;
            bufferToWrite = sound_thread_data.buf2;
            bool initalWriteToBuffer2 = true;



            for ( ;; ) { // Endless loop



                bool loopevent = true;

                /*  for (i = 0; i < BUFSIZE/2; ++i) {*/
                for ( i = 0; i < BUFSIZE; ++i ) {


                    bool keyevent = ( i%1000 ) == 1; // sample keybord input
                    bool lowevent = ( i%10 ) == 1;
                    bool lowervent = ( i%100 ) == 1;

                    int eventmask = ( 1 << 0 ) +  // one event
                                    ( lowevent << 1 ) +
                                    ( lowervent << 2 ) +
                                    ( keyevent << 3 );

                    loopevent = false; // onely the first time in this for loop


                    //
                    // Get Keys
                    //
                    if (keyevent) {
                        int j;
                        for ( j = 0; j<12; ++j )
                            inputs.keys[j] = 0;

                        key = get_key();
                        int channel = decode_key ( key, keytable, 13 );
                        if ( channel == 0 )
                            goto finish;
                        if ( channel > 0 ) {
                            printf ( "channel %d\n", channel );
                            inputs.keys[channel-1] = 1;
                        }
                    }


                    simbox->event_trigger_mask ( eventmask );

                    simbox->simulation_step ( 0 );

                    double soundstream;
                    //printf("%d %f\n", i, soundstream);
                    soundstream = simbox->get_skalar_out ( 0 );
                    bufferToWrite[i] = soundstream;

                    //   buf[ (i << 1)] = 0; //soundstream; // left ch
                    //   buf[1+ (i << 1)] = soundstream;

                    simbox->simulation_step ( 1 );

                }

// 	    if (sound_thread_data.activeToWrite_buffer == 1) {
// 	      bufferToWrite = sound_thread_data.buf1;
// 	      pthread_mutex_lock(&sound_thread_data.buffer1_mutex);
// 	    } else {
// 	      bufferToWrite = sound_thread_data.buf2;
// 	      pthread_mutex_lock(&sound_thread_data.buffer2_mutex);
// 	    }

                if (initalWriteToBuffer2) {
		  
		  printf("inital write to buf 2 complete\n");
		  
                        pthread_mutex_unlock(&sound_thread_data.buffer2_mutex);
		bufferToWrite = sound_thread_data.buf1; // Now really write to buffer 1
		initalWriteToBuffer2 = false;
		
                } else {
                    if (sound_thread_data.activeToWrite_buffer == 1) {
		        printf("Write to buffer 1 complete\n");
		      
                        pthread_mutex_unlock(&sound_thread_data.buffer1_mutex);
                        pthread_mutex_lock(&sound_thread_data.buffer2_mutex);

                        sound_thread_data.activeToWrite_buffer = 2;
			bufferToWrite = sound_thread_data.buf2;

                    } else {
		        printf("Write to buffer 2 complete\n");
                        pthread_mutex_unlock(&sound_thread_data.buffer2_mutex);
                        pthread_mutex_lock(&sound_thread_data.buffer1_mutex);

                        sound_thread_data.activeToWrite_buffer = 1;
			bufferToWrite = sound_thread_data.buf1;
                    }
                }

// play

            } // endless loop

finish :
            changemode ( 0 );




            simbox->destruct();
        }
    }

} // extern C
