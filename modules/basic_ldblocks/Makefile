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
	echo "Module basic_ldblocks done"

lib: basic_ldblocks.o
	ar rvs module.a basic_ldblocks.o   # Will always be included
	#ar rvs module_Linux.a special_linux_code.o   # Only include for "Linux" Target


clean:
	rm -f *.o module.a module_Linux.a


basic_ldblocks.o: basic_ldblocks.c
	$(CC) $(CFLAGS) -c basic_ldblocks.c 



