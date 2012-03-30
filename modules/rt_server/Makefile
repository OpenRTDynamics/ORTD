
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




all: lib ortd_client
	echo "Module rt_server done"

lib: rt_server.o parameter_manager.o directory.o rt_server_block.o ortd_buffered_io.o remote_decoder.o global_shared_object.o
	ar rvs module.a rt_server.o parameter_manager.o directory.o  rt_server_block.o ortd_buffered_io.o remote_decoder.o global_shared_object.o # scope_block.o
#	ar rvs module_Linux.a scope_block.o   # Only include for "Linux" Target
#	$(LD) -static -o scope.o rt_preempt_scope.o rt_preempt_scope_decoder.o


clean:
	rm -f *.o module.a ortd_client


rt_server_test: rt_server.cpp parameter_manager.cpp directory.cpp rt_server_test.cpp
	$(CPP) -I../.. -L. -O2 -lm -lpthread rt_server.cpp parameter_manager.cpp directory.cpp  rt_server_test.cpp ../../log.c -o rt_server_test

remote_client_example:  remote_decoder.cpp remote_client_example.cpp ortd_buffered_io.cpp
	$(CPP) -I../.. -L. -O2 -lm -lpthread remote_decoder.cpp remote_client_example.cpp ortd_buffered_io.cpp  -o remote_client_example

ortd_client:  remote_decoder.cpp remote_client.cpp ortd_buffered_io.cpp
	$(CPP) -I../.. -L. -O2 -lm -lpthread remote_decoder.cpp remote_client.cpp ortd_buffered_io.cpp  -o ortd_client

rt_server_block.o: rt_server_block.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -c rt_server_block.cpp

rt_server.o: rt_server.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -c rt_server.cpp

parameter_manager.o: parameter_manager.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -c parameter_manager.cpp

directory.o: directory.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -c directory.cpp

ortd_buffered_io.o: ortd_buffered_io.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -c ortd_buffered_io.cpp

remote_decoder.o: remote_decoder.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -c remote_decoder.cpp

global_shared_object.o: global_shared_object.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -c global_shared_object.cpp

