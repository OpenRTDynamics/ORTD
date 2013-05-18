
ifeq ($(main_makefile_invoked),yes)

else
	# If this makefile is just run within this directory

	CC = cc
	CPP = c++
	LD = ld

	#CFLAGS = -I../.. -O2 



	# detect system type
	host-type := $(shell arch)

	ifeq ($(host-type),x86_64)
	# 64 Bit
	CFLAGS = -fPIC -O2 -g -I../..
	INCLUDE = -I../..
	LDFLAGS = -shared
	else
	# 32 Bit
	CFLAGS = -O2 -I../.. -g
	INCLUDE = -I../..
	LDFLAGS = -shared 
	endif

endif # ifeq ($(main_makefile_invoked),yes)


all: lib android_audio_standalone #android_sensors_standalone
	@echo "Module android done"

lib: android_block.o android_sensors.o 
	ar rvs module.a android_block.o android_sensors.o   # Will always be included
	#ar rvs module_Linux.a special_linux_code.o   # Only include for "Linux" Target


#android_sensors_standalone:  android_sensors_standalone.cpp 
#	$(CPP) -I../.. -L. $(CFLAGS)  $(LD_LIBRARIES)  android_sensors_standalone.cpp  -o android_sensors_standalone

android_audio_standalone:  opensl_example.c opensl_io.c
	$(CPP) -I../.. -L. $(CFLAGS)  $(LD_LIBRARIES) -fpermissive opensl_example.c opensl_io.c -o android_audio_standalone


clean:
	rm -f *.o module.a module_Linux.a android_sensors_standalone android_audio_standalone




opensl_io.o:	opensl_io.c
	$(CPP) $(CFLAGS) $(INCLUDE) -c opensl_io.c

android_block.o: android_block.cpp
	$(CPP) $(CFLAGS) $(INCLUDE) -c android_block.cpp

android_sensors.o: android_sensors.cpp
	$(CPP) $(CFLAGS) $(INCLUDE) -c android_sensors.cpp

android_audio.o: android_audio.cpp
	$(CPP) $(CFLAGS) $(INCLUDE) -c android_audio.cpp


