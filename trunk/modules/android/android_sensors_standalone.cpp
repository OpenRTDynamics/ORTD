#include <jni.h>
#include <errno.h>

// #include <EGL/egl.h>
// #include <GLES/gl.h>

#include <android/sensor.h>
#include <android/log.h>
#include <android/looper.h>
//#include <android_native_app_glue.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "native-activity", __VA_ARGS__))


#include <malloc.h>

#include <stdio.h>


static int get_sensor_events(int fd, int events, void* data) { 
  ASensorEvent event; 
  
  printf("sensval\n");
  
 // ASensorEventQueue* sensorEventQueue; 
  
//   while (ASensorEventQueue_getEvents(sensorEventQueue, &event, 1) > 0) 
// { 
//     if(event.type == ASENSOR_TYPE_ACCELEROMETER) { 
//             if(SensorSamplingRate == 0) 
//                 { 
//                          LOGI("Time1 of SensorValue %lld", event.timestamp); 
//                 } 
//             if(SensorSamplingRate == 1000) 
//                 { 
// 
//                      LOGI("Time2 of SensorValue %lld", event.timestamp); 
//                 } 
// 
//             SensorSamplingRate++; 
//     } 
// 
//   }
//   
  //should return 1 to continue receiving callbacks, or 0 to 
  return 1; 
} 




void android_init_sensor( ) { 
        ASensorEvent event; 
        int events, ident; 
        ASensorManager* sensorManager; 
        const ASensor* accSensor; 
        void* sensor_data = malloc(1000); 

	printf("1\n");

	
        ALooper* looper = ALooper_forThread(); 

        if(looper == NULL) 
        { 
                looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS); 
        } 
        
	printf("2\n");

        sensorManager = ASensorManager_getInstance(); 
	printf("3\n");

        accSensor = ASensorManager_getDefaultSensor(sensorManager, ASENSOR_TYPE_ACCELEROMETER); 
	printf("4\n");

        ASensorEventQueue* sensorEventQueue = ASensorManager_createEventQueue(sensorManager, looper, 3, get_sensor_events, sensor_data); 
	printf("5\n");

        ASensorEventQueue_enableSensor(sensorEventQueue, accSensor); 

        int a = ASensor_getMinDelay(accSensor); 

        ASensorEventQueue_setEventRate(sensorEventQueue, accSensor, a); 

        LOGI("sensorValue() - START"); 
} 



#define LOOPER_ID 1
#define SAMP_PER_SEC 100 //i've changed to 120, even 10, but nothing happen

void start() {
    ASensorManager* sensorManager = ASensorManager_getInstance();

    ALooper* looper = ALooper_forThread();
    if(looper == NULL)
        looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);

    ASensorRef accelerometerSensor = ASensorManager_getDefaultSensor(sensorManager,ASENSOR_TYPE_ACCELEROMETER);
    printf("accelerometerSensor: %s, vendor: %s", ASensor_getName(accelerometerSensor), ASensor_getVendor(accelerometerSensor));

    ASensorEventQueue* queue = ASensorManager_createEventQueue(sensorManager, looper, LOOPER_ID, NULL, NULL);

    ASensorEventQueue_enableSensor(queue, accelerometerSensor);
    ASensorEventQueue_setEventRate(queue, accelerometerSensor, (1000L/SAMP_PER_SEC)*1000); 

    int ident;//identifier 
    int events;
    while (1) {
        while ((ident=ALooper_pollAll(-1, NULL, &events, NULL) >= 0)) {
            // If a sensor has data, process it now.
            if (ident == LOOPER_ID) {
                ASensorEvent event;
                while (ASensorEventQueue_getEvents(queue, &event, 1) > 0) {
                    printf("accelerometer X = %f y = %f z=%f \n", event.acceleration.x, event.acceleration.y, event.acceleration.z);
                }
            }
        }
    }

}




int main() {
  //android_init_sensor();
  start();
}
