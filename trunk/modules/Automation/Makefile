
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
	@echo "Module Automation done"

lib: Automation_blocks.o
	 ar rvs module.a Automation_blocks.o   # Will always be included


clean:
	rm -f *.o module.a


## EITHER THIS

#template.o: template.c
#	$(CC) $(CFLAGS) $(INCLUDE) -c template.c 

## OR THIS

Automation_blocks.o: Automation_blocks.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -c Automation_blocks.cpp


