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


all: lib
	echo "Module nested done"

lib: nested_block.o
	ar rvs module.a nested_block.o   # Will always be included
	#ar rvs module_Linux.a special_linux_code.o   # Only include for "Linux" Target


clean:
	rm -f *.o module.a module_Linux.a


nested_block.o: nested_block.cpp
	$(CC) $(CFLAGS) -c nested_block.cpp



