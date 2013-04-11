/*
    Copyright (C) 2010, 2011, 2012, 2013  Christian Klauer

    This file is part of OpenRTDynamics, the Real Time Dynamics Framework

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

//// Compile only on Android /////
  #ifdef __ORTD_TARGET_ANDROID
//////////////////////////////////


#include <malloc.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <time.h>
#include <sys/time.h>
#include <sched.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>
#include <pthread.h>
//#include <sys/io.h>


extern "C" {
#include "libdyn.h"
#include "libdyn_scicos_macros.h"
}




// android headers
#include <jni.h>
#include <errno.h>
#include <android/sensor.h>
#include <android/log.h>
#include <android/looper.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "native-activity", __VA_ARGS__))




#define LOOPER_ID 1





class compu_func_SyncAndroidSensors_class {
public:
    compu_func_SyncAndroidSensors_class(struct dynlib_block_t *block);
    void destruct();
    void io(int update_states);
    int init();

    // C to C++ callback function
    static int sync_callback(struct dynlib_simulation_t * sim) {
        void * obj = sim->sync_callback.userdat;
        compu_func_SyncAndroidSensors_class *p = (compu_func_SyncAndroidSensors_class *) obj;
        return p->real_sync_callback(sim);
    }


private:
    struct dynlib_block_t *block;

    // the c++ callback rountine
    int real_sync_callback(struct dynlib_simulation_t * sim );


    // variables
    ASensorManager* sensorManager;
    ALooper* looper;
    ASensorRef accelerometerSensor;
    ASensorEventQueue* queue;
    
    double ax, ay, az;
};



compu_func_SyncAndroidSensors_class::compu_func_SyncAndroidSensors_class(dynlib_block_t* block)
{
    this->block = block;
}

int compu_func_SyncAndroidSensors_class::init()
{
    double *rpar = libdyn_get_rpar_ptr(block);
    int *ipar = libdyn_get_ipar_ptr(block);



    
    
    
    
    // register the callback function to the simulator that shall trigger the simulation while running in a loop
    libdyn_simulation_setSyncCallback(block->sim, &compu_func_SyncAndroidSensors_class::sync_callback , this);



    return 0;
}


int compu_func_SyncAndroidSensors_class::real_sync_callback( struct dynlib_simulation_t * sim )
{
    /*
     * This function is called before any of the output or state-update flags
     * are called.
     * If 0 is returned, the simulation will continue to run
     * If 1 is returned, the simulation will pause and has to be re-triggered externally.
     * e.g. by the trigger_computation input of the async nested_block.


    */

    double *rpar = libdyn_get_rpar_ptr(block);
    int *ipar = libdyn_get_ipar_ptr(block);


    int SAMP_PER_SEC = ipar[0];
    
    // set-up sensors
    sensorManager = ASensorManager_getInstance();
    looper = ALooper_forThread();
    if(looper == NULL)
        looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);  

    accelerometerSensor = ASensorManager_getDefaultSensor(sensorManager,ASENSOR_TYPE_ACCELEROMETER);
    printf("accelerometerSensor: %s, vendor: %s\n", ASensor_getName(accelerometerSensor), ASensor_getVendor(accelerometerSensor));

    queue = ASensorManager_createEventQueue(sensorManager, looper, LOOPER_ID, NULL, NULL);

    ASensorEventQueue_enableSensor(queue, accelerometerSensor);
    ASensorEventQueue_setEventRate(queue, accelerometerSensor, (1000L/SAMP_PER_SEC)*1000);

    
    
//   printf("waiting for data\n");

int ident;//identifier
    int events;
    do { // Main loop is now here

        //
        // Do something e.g. wait for a new value from a sensors
        //
        while ((ident=ALooper_pollAll(-1, NULL, &events, NULL) >= 0)) {
// 	    printf("Data available\n");
            // If a sensor has data, process it now.
            if (ident == LOOPER_ID) {
                ASensorEvent event;
                while (ASensorEventQueue_getEvents(queue, &event, 1) > 0) {
//                     printf("accelerometer X = %f y = %f z=%f \n", event.acceleration.x, event.acceleration.y, event.acceleration.z);

		    ax = event.acceleration.x;
		    ay = event.acceleration.y;
		    az = event.acceleration.z;

                    // run the simulation
                    // run one simulation step

                    // Use C-functions to simulation one timestep
                    libdyn_event_trigger_mask(sim, 1);
                    libdyn_simulation_step(sim, 0);
                    libdyn_simulation_step(sim, 1);

                }
            }
        }





        // at some point in time do
        // return 1;
        // to stop execution of the simulation
        double *input = (double*) libdyn_get_input_ptr(block, 0);
	
	if (*input > 0.5) {

	  // destroy
	  ASensorEventQueue_disableSensor(queue, accelerometerSensor);
	  ASensorManager_destroyEventQueue(sensorManager, queue);
	  // FIXME do not know how to destroy a looper. Is it necessarry?
	  
	  
	  return 1; // abort loop, if input is 1
	  
	}


    } while (true);



    return 0; // return zero means that this callback is called again
}


void compu_func_SyncAndroidSensors_class::io(int update_states)
{
    if (update_states==0) {
        double *output = (double*) libdyn_get_output_ptr(block, 0);

        output[0] = ax;
	output[1] = ay;
	output[2] = az;
    }
}

void compu_func_SyncAndroidSensors_class::destruct()
{

}



extern "C" {

// This is the main C-Callback function, which forwards requests to the C++-Class above
int compu_func_AndroidSensor(int flag, struct dynlib_block_t *block)
{

//     printf("comp_func template: flag==%d\n", flag);

    double *in;
    double *rpar = libdyn_get_rpar_ptr(block);
    int *ipar = libdyn_get_ipar_ptr(block);

    int Nin = 1;
    int Nout = 1;


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        in = (double *) libdyn_get_input_ptr(block,0);
        compu_func_SyncAndroidSensors_class *worker = (compu_func_SyncAndroidSensors_class *) libdyn_get_work_ptr(block);

        worker->io(0);

    }
    return 0;
    break;
    case COMPF_FLAG_UPDATESTATES:
    {
        in = (double *) libdyn_get_input_ptr(block,0);
        compu_func_SyncAndroidSensors_class *worker = (compu_func_SyncAndroidSensors_class *) libdyn_get_work_ptr(block);

        worker->io(1);

    }
    return 0;
    break;
    case COMPF_FLAG_CONFIGURE:  // configure. NOTE: do not reserve memory or open devices. Do this while init instead!
    {
        int i;
        libdyn_config_block(block, BLOCKTYPE_DYNAMIC, Nout, Nin, (void *) 0, 0);


        libdyn_config_block_input(block, 0, 1, DATATYPE_FLOAT);
        libdyn_config_block_output(block, 0, 10, DATATYPE_FLOAT, 0);


    }
    return 0;
    break;
    case COMPF_FLAG_INIT:  // init
    {
        compu_func_SyncAndroidSensors_class *worker = new compu_func_SyncAndroidSensors_class(block);
        libdyn_set_work_ptr(block, (void*) worker);

        int ret = worker->init();
        if (ret < 0)
            return -1;
    }
    return 0;
    break;
    case COMPF_FLAG_DESTUCTOR: // destroy instance
    {
        compu_func_SyncAndroidSensors_class *worker = (compu_func_SyncAndroidSensors_class *) libdyn_get_work_ptr(block);

        worker->destruct();
        delete worker;

    }
    return 0;
    break;
    case COMPF_FLAG_PRINTINFO:
        printf("I'm a SyncAndroidSensors block\n");
        return 0;
        break;

    }
}

} // extern C








//// Compile only on Android /////
  #endif
//////////////////////////////////
