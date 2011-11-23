
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
	echo "Module nested done"

lib: nested_block.o
	ar rvs module.a nested_block.o   # Will always be included
	#ar rvs module_Linux.a special_linux_code.o   # Only include for "Linux" Target


clean:
	rm -f *.o module.a module_Linux.a


nested_block.o: nested_block.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -c nested_block.cpp



