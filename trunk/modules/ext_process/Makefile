
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
	@echo "Module ext_process_blocks done"

lib: ext_process_blocks.o ortd_fork.o
	ar rvs module.a ext_process_blocks.o ortd_fork.o   # Will always be included
	#ar rvs module_Linux.a special_linux_code.o   # Only include for "Linux" Target


clean:
	rm -f *.o module.a module_Linux.a


## EITHER THIS

#template.o: template.c
#	$(CC) $(CFLAGS) $(INCLUDE) -c template.c 

## OR THIS

ext_process_blocks.o: ext_process_blocks.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -c ext_process_blocks.cpp -o ext_process_blocks.o

ortd_fork.o: ortd_fork.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -c ortd_fork.cpp -o ortd_fork.o





