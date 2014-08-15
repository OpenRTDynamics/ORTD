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

/*


http://stackoverflow.com/questions/13679568/using-android-gyroscope-instead-of-accelerometer-i-find-lots-of-bits-and-pieces

http://developer.android.com/reference/android/hardware/SensorManager.html

http://code.google.com/p/android/issues/detail?id=41965


But the important thing is that newer versions of Android contain these two new virtual sensors:

TYPE_GRAVITY is the accelerometer input with the effect of motion filtered out 
TYPE_LINEAR_ACCELERATION is the accelerometer with the gravity component filtered out.

These two virtual sensors are synthesized through a combination of accelerometer input and gyro input.
Another notable sensor is TYPE_ROTATION_VECTOR which is a Quaternion synthesized from accelerometer, 
magnetometer, and gyro. It represents the full 3-d orientation of the device with the effects of linear 
acceleration filtered out.

 * Sensor types from master/hardware/libhardware/include/hardware/sensors.h
 * Not all are exposed through NDK (r8d)

 #define SENSOR_TYPE_ACCELEROMETER       1
#define SENSOR_TYPE_MAGNETIC_FIELD      2
#define SENSOR_TYPE_ORIENTATION         3
#define SENSOR_TYPE_GYROSCOPE           4
#define SENSOR_TYPE_LIGHT               5
#define SENSOR_TYPE_PRESSURE            6
#define SENSOR_TYPE_TEMPERATURE         7   // deprecated
#define SENSOR_TYPE_PROXIMITY           8
#define SENSOR_TYPE_GRAVITY             9
#define SENSOR_TYPE_LINEAR_ACCELERATION 10
#define SENSOR_TYPE_ROTATION_VECTOR     11
#define SENSOR_TYPE_RELATIVE_HUMIDITY   12
#define SENSOR_TYPE_AMBIENT_TEMPERATURE 13




*/


#include <malloc.h>
#include <stdio.h>

extern "C" {
#include "libdyn.h"
#include "libdyn_scicos_macros.h"
}
#include "libdyn_cpp.h"




// only compile this code if the platform is android
#ifdef __ORTD_TARGET_ANDROID


// android headers
#include <jni.h>
#include <errno.h>
#include <android/sensor.h>
#include <android/log.h>
#include <android/looper.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "native-activity", __VA_ARGS__))




#define LOOPER_ID 1







class SynchronisingAndroidSensorsBlock {
public:
    SynchronisingAndroidSensorsBlock(struct dynlib_block_t *block) {
        this->block = block;    // no nothing more here. The real initialisation take place in init()

//         printf("construct()\n");
    }
    ~SynchronisingAndroidSensorsBlock()
    {
        // free your allocated memory, ...
    }

    //
    // define states or other variables
    //

    int rateAcc, rateGyro, rateMagn, rateGPS;
    int AccId, GyroId, MagnId;
    bool ExitLoop;

    ASensorManager* sensorManager;
    ALooper* looper;
    ASensorRef accelerometerSensor, GyroSensor, MagnSensor;
    ASensorEventQueue* queue;

    ASensorEvent event;
//     double ax, ay, az;

    //
    // initialise your block
    //

    int init() {
//       printf("init()\n");

        int *Uipar;
        double *Urpar;

        // Get the irpar parameters Uipar, Urpar
        libdyn_AutoConfigureBlock_GetUirpar(block, &Uipar, &Urpar);




        try {
            irpar_ivec veccpp(Uipar, Urpar, 10); // then use:  veccpp.n; veccpp.v;
// 	  printf("veccpp[0] = %d\n", veccpp.v[0]); // print the first element
            // of the vector that is of size veccpp.n
            rateAcc = veccpp.v[0];
            rateGyro = veccpp.v[1];
            rateMagn = veccpp.v[2];
            rateGPS = veccpp.v[3];
        } catch(int e) {
            // parameter not found
            return -1;
        }


        fprintf(stderr, "AndroidSensors: Config: %d %d %d %d\n", rateAcc, rateGyro, rateMagn, rateGPS);

        // set the initial states
        resetStates();

        // register the callback function to the simulator that shall trigger the simulation while running in a loop
        libdyn_simulation_setSyncCallback(block->sim, &syncCallback_ , this);
        libdyn_simulation_setSyncCallbackDestructor(block->sim, &syncCallbackDestructor_ , this);

        //
        ExitLoop = false;

        //
        // Set initial output
 	//
	int N = libdyn_get_outportsize(block, 0);  // the size of the first (=0) input vector
// 	int datatype = libdyn_get_outportdatatype(block, 0); // the datatype
// 	int TypeBytes = libdyn_config_get_datatype_len(datatype); // number of bytes allocated for one element of type "datatype"
// 	int NBytes = N * TypeBytes;  // Amount of bytes allocated for the input vector

	int i;
	for (i=0; i<N; ++i) {
	  double *output = (double*) libdyn_get_output_ptr(block, 0); // the first output port
	  output[i] = 0; 
	}
	
	

        // Return -1 to indicate an error, so the simulation will be destructed
        return 0;
    }


    inline void updateStates()
    {
//         double *in1 = (double *) libdyn_get_input_ptr(block,0); // the first input port
//         double *in2 = (double *) libdyn_get_input_ptr(block,1); // the 2nd input port
//         double *output = (double*) libdyn_get_output_ptr(block, 0); // the first output port
    }


    inline void calcOutputs()
    {
//         double *in1 = (double *) libdyn_get_input_ptr(block,0); // the first input port
//         double *in2 = (double *) libdyn_get_input_ptr(block,1); // the 2nd input port
        double *output = (double*) libdyn_get_output_ptr(block, 0); // the first output port
        int32_t *SensorID = (int32_t *) libdyn_get_output_ptr(block, 1); // the first output port

        *SensorID = event.type;

        // Put out the sensor value
        if (event.type == ASENSOR_TYPE_ACCELEROMETER) {
            output[0] = event.acceleration.x;
            output[1] = event.acceleration.y;
            output[2] = event.acceleration.z;
        } else if (event.type == ASENSOR_TYPE_GYROSCOPE) {
            output[3] = event.vector.x;
            output[4] = event.vector.y;
            output[5] = event.vector.z;
        } else if (event.type == ASENSOR_TYPE_MAGNETIC_FIELD) {
            output[6] = event.magnetic.x;
            output[7] = event.magnetic.y;
            output[8] = event.magnetic.z;
	}

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

//         printf("Threaded simulation started execution\n");


        // set-up sensors
        sensorManager = ASensorManager_getInstance();
        looper = ALooper_forThread();
        if(looper == NULL)
            looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);


        queue = ASensorManager_createEventQueue(sensorManager, looper, LOOPER_ID, NULL, NULL);

	int SensorCounter = 0;
	
        // ACC
        if (rateAcc > 0) {
            accelerometerSensor = ASensorManager_getDefaultSensor(sensorManager,ASENSOR_TYPE_ACCELEROMETER);
            if (accelerometerSensor != NULL) {
                fprintf(stderr, "accelerometerSensor: %s, vendor: %s\n", ASensor_getName(accelerometerSensor), ASensor_getVendor(accelerometerSensor));

                ASensorEventQueue_enableSensor(queue, accelerometerSensor);
                ASensorEventQueue_setEventRate(queue, accelerometerSensor, (1000L/rateAcc)*1000);
		
		AccId = SensorCounter;
		SensorCounter++;
		
            } else {
                fprintf(stderr, "accelerometerSensor not found\n");
            }
        }

        // Gyro
        if (rateGyro > 0) {
            GyroSensor = ASensorManager_getDefaultSensor(sensorManager,ASENSOR_TYPE_GYROSCOPE);
            if (GyroSensor != NULL) {
                fprintf(stderr, "GyroSensor: %s, vendor: %s\n", ASensor_getName(GyroSensor), ASensor_getVendor(GyroSensor));

                ASensorEventQueue_enableSensor(queue, GyroSensor);
                ASensorEventQueue_setEventRate(queue, GyroSensor, (1000L/rateAcc)*1000);
		
		GyroId = SensorCounter;
		SensorCounter++;
            } else {
                fprintf(stderr, "Gyro Sensor not found\n");
            }
        }

        // Magn
        if (rateMagn > 0) {
            MagnSensor = ASensorManager_getDefaultSensor(sensorManager, ASENSOR_TYPE_MAGNETIC_FIELD);
            if (MagnSensor != NULL) {
                fprintf(stderr, "MagnSensor: %s, vendor: %s\n", ASensor_getName(MagnSensor), ASensor_getVendor(MagnSensor));

                ASensorEventQueue_enableSensor(queue, MagnSensor);
                ASensorEventQueue_setEventRate(queue, MagnSensor, (1000L/rateAcc)*1000);
		
		MagnId = SensorCounter;
		SensorCounter++;
            } else {
                fprintf(stderr, "Magn Sensor not found\n");
            }
        }


        int ident;//identifier
        int events;


        // This is the main loop of the new simulation
        {
            //
            // Do something e.g. wait for a new value from a sensors
            //
            while (  (ident=ALooper_pollAll(-1, NULL, &events, NULL) >= 0)  && !ExitLoop ) {
                // If a sensor has data, process it now.
                if (ident == LOOPER_ID) {

                    while (ASensorEventQueue_getEvents(queue, &event, 1) > 0) {
//                     printf("accelerometer X = %f y = %f z=%f \n", event.acceleration.x, event.acceleration.y, event.acceleration.z);

//                          ax = event.acceleration.x;
//                          ay = event.acceleration.y;
//                          az = event.acceleration.z;

                        // run one step of the ortd simulator
                        // Use C-functions to simulation one timestep
                        libdyn_event_trigger_mask(sim, 1);
                        libdyn_simulation_step(sim, 0);
                        libdyn_simulation_step(sim, 1);
                    }
                }

            }
        }

        fprintf(stderr, "Android Sensors main loop exited\n");

        return 1; // 1 - this shall not be executed again, directly after returning from this function!
    }

    int SyncDestructorCallback( struct dynlib_simulation_t * sim )
    {
        // Trigger termination of the the main loop
        ExitLoop = true;
    }


    void printInfo() {
        fprintf(stderr, "I'm a AndroidSensors block\n");
    }

    // uncommonly used flags
    void PrepareReset() {}
    void HigherLevelResetStates() {}
    void PostInit() {}

    // The Computational function that is called by the simulator
    // and that distributes the execution to the various functions
    // in this C++ - Class, including: init(), io(), resetStates() and the destructor
    static int CompFn(int flag, struct dynlib_block_t *block) {
//       printf("CompFn called %d\n", flag);
        return LibdynCompFnTempate<SynchronisingAndroidSensorsBlock>( flag, block ); // this expands a template for a C-comp fn
    }
    static int syncCallback_(struct dynlib_simulation_t * sim) {
        void * obj = sim->sync_callback.userdat;
        SynchronisingAndroidSensorsBlock *p = (SynchronisingAndroidSensorsBlock *) obj;
        return p->SyncCallback(sim);
    }
    static int syncCallbackDestructor_(struct dynlib_simulation_t * sim) {
        void * obj = sim->sync_callback.userdatDestructor;
        SynchronisingAndroidSensorsBlock *p = (SynchronisingAndroidSensorsBlock *) obj;
        return p->SyncDestructorCallback(sim);
    }

    // The data for this block managed by the simulator
    struct dynlib_block_t *block;
};











#endif // Android



// Export to C so the libdyn simulator finds this function
extern "C" {
    // ADJUST HERE: must match to the function name in the end of this file
    int libdyn_module_android_siminit(struct dynlib_simulation_t *sim, int bid_ofs);

//// Compile only on Android /////
#ifdef __ORTD_TARGET_ANDROID
//////////////////////////////////


    int compu_func_AndroidSensor(int flag, struct dynlib_block_t *block);

//// Compile only on Android /////
#endif
//////////////////////////////////

}

// CHANGE HERE: Adjust this function name to match the name of your module
int libdyn_module_android_siminit(struct dynlib_simulation_t *sim, int bid_ofs)
{
//// Compile only on Android /////
#ifdef __ORTD_TARGET_ANDROID
//////////////////////////////////

    // Register my blocks to the given simulation

    int blockid = 15500;  // CHANGE HERE: choose a unique id for each block
//     libdyn_compfnlist_add(sim->private_comp_func_list, blockid+0, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &compu_func_AndroidSensor);
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid+0, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &SynchronisingAndroidSensorsBlock::CompFn  );


//     libdyn_compfnlist_add(sim->private_comp_func_list, blockid+1, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &compu_func_AndroidAudio);

#ifdef DEBUG
    printf("libdyn module android initialised\n");
#endif  


//// Compile only on Android /////
#endif
//////////////////////////////////


}


