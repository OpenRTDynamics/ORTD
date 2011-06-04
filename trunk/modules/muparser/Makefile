CC = cc
CPP = c++
LD = ld

muparser_libdir=muparser_v134

muparser_include=$(muparser_libdir)/include

#CFLAGS = -I../.. -O2 

# detect system type
host-type := $(shell arch)

ifeq ($(host-type),x86_64)
# 64 Bit
CFLAGS = -fPIC -O2 -I../.. -I$(muparser_include)
LDFLAGS = -shared
else
# 32 Bit
CFLAGS = -O2 -I../.. -I$(muparser_include)
LDFLAGS = -shared 
endif



all: lib
	echo "Module muparser done"

lib: muparser_block.o libmuparser
	ar rvs module.a muparser_block.o $(muparser_libdir)/*.o  # Will always be included
	#ar rvs module_Linux.a special_linux_code.o   # Only include for "Linux" Target

libmuparser:
	cd $(muparser_libdir); make

clean:
	rm -f *.o module.a module_Linux.a
	cd muparser_v134; make clean


muparser_block.o: muparser_block.cpp
	$(CC) $(CFLAGS) -c muparser_block.cpp



