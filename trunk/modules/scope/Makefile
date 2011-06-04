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
CFLAGS = -O2 -I../..
LDFLAGS = -shared 
endif


all: lib termrtailab
	echo "Module scope done"

lib: register.o rt_preempt_scope.o scope_block.o
	ar rvs module.a register.o rt_preempt_scope.o  # scope_block.o
	ar rvs module_Linux.a scope_block.o   # Only include for "Linux" Target
#	$(LD) -static -o scope.o rt_preempt_scope.o rt_preempt_scope_decoder.o

termrtailab: rt_preempt_scope_decoder.o rt_preempt_example.cpp
	$(CPP) -I../.. -L. -O2 -lm rt_preempt_scope_decoder.cpp rt_preempt_example.cpp -o termrtailab
#	$(LD) -o example rt_preempt_scope_decoder.o rt_preempt_example.o

termrtailab_thread: rt_preempt_scope_decoder.o rt_preempt_example_thread.cpp
	$(CPP) -I../.. -L. -O2 -lm -lpthread rt_preempt_scope_decoder.cpp rt_preempt_example_thread.cpp -o termrtailab_thread


clean:
	rm -f *.o module.a module_Linux.a termrtailab termrtailab_thread

#rt_preempt_example.o: rt_preempt_example.cpp
#	$(CC) $(CFLAGS) -c rt_preempt_example.cpp

rt_preempt_scope.o: rt_preempt_scope.c
	$(CC) $(CFLAGS) -c rt_preempt_scope.c

# does not belong to realtime lib part
rt_preempt_scope_decoder.o: rt_preempt_scope_decoder.cpp
	$(CC) $(CFLAGS) -c rt_preempt_scope_decoder.cpp

# Compile for "Linux"-Target
scope_block.o: scope_block.c
	$(CC) $(CFLAGS) -c scope_block.c 


register.o: register.c
	$(CC) $(CFLAGS) -c register.c 




