
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


all: android_sensors_standalone lib 
	@echo "Module android done"

lib: android_block.o android_sensors.o
	ar rvs module.a android_block.o android_sensors.o  # Will always be included
	#ar rvs module_Linux.a special_linux_code.o   # Only include for "Linux" Target


android_sensors_standalone:  android_sensors_standalone.cpp 
	$(CPP) -I../.. -L. $(CFLAGS)  $(LD_LIBRARIES)  android_sensors_standalone.cpp  -o android_sensors_standalone


clean:
	rm -f *.o module.a module_Linux.a android_sensors_standalone


## EITHER THIS

#template.o: template.c
#	$(CC) $(CFLAGS) $(INCLUDE) -c template.c 

## OR THIS

android_block.o: android_block.cpp
	$(CPP) $(CFLAGS) $(INCLUDE) -c android_block.cpp

android_sensors.o: android_sensors.cpp
	$(CPP) $(CFLAGS) $(INCLUDE) -c android_sensors.cpp
