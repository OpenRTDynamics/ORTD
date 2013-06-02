
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
	echo "Module template done"

lib: udp_blocks.o
	ar rvs module.a udp_blocks.o  # Will always be included


clean:
	rm -f *.o module.a module_Linux.a


## EITHER THIS

#template.o: template.c
#	$(CC) $(CFLAGS) $(INCLUDE) -c template.c 

## OR THIS

udp_blocks.o: udp_blocks.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -c udp_blocks.cpp

#PracticalSocket.o: PracticalSocket.cpp
#	$(CC) $(CFLAGS) $(INCLUDE) -c PracticalSocket.cpp


