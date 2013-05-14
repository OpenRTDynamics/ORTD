
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
	@echo "Module edf done"

lib: edflib.o edf_blocks.o
	ar rvs module.a edflib.o edf_blocks.o  


clean:
	rm -f *.o module.a module_Linux.a


## EITHER THIS

#edf.o: edf.c
#	$(CC) $(CFLAGS) $(INCLUDE) -c edf.c 

## OR THIS

edf_blocks.o: edf_blocks.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -c edf_blocks.cpp

edflib.o: edflib.c
	$(CC) $(CFLAGS) $(INCLUDE) -c edflib.c 

