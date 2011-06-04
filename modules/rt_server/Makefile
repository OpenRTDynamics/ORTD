CC = cc
CPP = c++
LD = ld

#CFLAGS = -I../.. -O2 

# detect system type
host-type := $(shell arch)

ifeq ($(host-type),x86_64)
# 64 Bit
CFLAGS = -fPIC -O2 -I../.. 
LDFLAGS = -shared
else
# 32 Bit
CFLAGS = -g -I../..
LDFLAGS = -shared 
endif


all: lib rt_server_test
	echo "Module rt_server done"

lib: rt_server.o parameter_manager.o directory.o
	ar rvs module.a rt_server.o parameter_manager.o directory.o # scope_block.o
#	ar rvs module_Linux.a scope_block.o   # Only include for "Linux" Target
#	$(LD) -static -o scope.o rt_preempt_scope.o rt_preempt_scope_decoder.o


clean:
	rm -f *.o module.a


rt_server_test: rt_server.o rt_server_test.cpp
	$(CPP) -I../.. -L. -O2 -lm -lpthread rt_server.cpp rt_server_test.cpp -o rt_server_test

rt_server.o: rt_server.cpp
	$(CC) $(CFLAGS) -c rt_server.cpp

parameter_manager.o: parameter_manager.cpp
	$(CC) $(CFLAGS) -c parameter_manager.cpp

directory.o: directory.cpp
	$(CC) $(CFLAGS) -c directory.cpp
