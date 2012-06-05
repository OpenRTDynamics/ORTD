
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

lib: scilab_block.o run_scilab.o
	ar rvs module.a scilab_block.o run_scilab.o # Will always be included
	#ar rvs module_Linux.a special_linux_code.o   # Only include for "Linux" Target

clean:
	rm -f *.o module.a module_Linux.a scilabconf.h


# FIXME: This required GNU's readlink implementation ?
.PHONY: scilabconfig
scilabconfig:
	./config_scilab.sh



## EITHER THIS

#template.o: template.c
#	$(CC) $(CFLAGS) $(INCLUDE) -c template.c 

## OR THIS

scilab_block.o: scilab_block.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -c scilab_block.cpp
run_scilab.o: run_scilab.cpp scilabconfig
	$(CC) $(CFLAGS) $(INCLUDE) -c run_scilab.cpp


