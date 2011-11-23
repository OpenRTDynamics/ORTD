
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


all: lib
	echo "Module serial done"

lib: serial_block.o ortd_serial_linux.o
	ar rvs module.a serial_block.o ortd_serial_linux.o  # Will always be included
	#ar rvs module_Linux.a special_linux_code.o   # Only include for "Linux" Target


clean:
	rm -f *.o module.a module_Linux.a




serial_block.o: serial_block.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -c serial_block.cpp

ortd_serial_linux.o: ortd_serial_linux.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -c ortd_serial_linux.cpp

