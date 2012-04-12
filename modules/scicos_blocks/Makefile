
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

lib: scicos_block_interface.o ScicosWrapper.o scicos_compfn_list.o
	ar rvs module.a scicos_block_interface.o ScicosWrapper.o scicos_compfn_list.o # Will always be included
	#ar rvs module_Linux.a special_linux_code.o   # Only include for "Linux" Target


clean:
	rm -f *.o module.a module_Linux.a


## EITHER THIS

#template.o: template.c
#	$(CC) $(CFLAGS) $(INCLUDE) -c template.c 

## OR THIS

scicos_block_interface.o: scicos_block_interface.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -Iscicos_include -c scicos_block_interface.cpp

ScicosWrapper.o: ScicosWrapper.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -Iscicos_include -c ScicosWrapper.cpp

scicos_compfn_list.o: scicos_compfn_list.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -c scicos_compfn_list.cpp

#compfns/SuperBlock.o: compfns/SuperBlock.c
#	$(CC) $(CFLAGS) $(INCLUDE) -I .. -c compfns/SuperBlock.c -o compfns/SuperBlock.o

