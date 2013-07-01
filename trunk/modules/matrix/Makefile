
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



With_gsl_and_blas := $(shell cat CONFIG_WITH_blas_AND_gsl)
ifeq ($(With_gsl_and_blas),yes)
	CFLAGS += -DGSL_INCLUDED -DBLAS_INCLUDED  
endif



all: lib
	@echo "Module MATRIX done"

lib: matrix.o
	ar rvs module.a matrix.o   # Will always be included
	#ar rvs module_Linux.a special_linux_code.o   # Only include for "Linux" Target


clean:
	rm -f *.o module.a module_Linux.a


config:
	@echo "Creating config: Checking for libblas and libgsl"

	@bash config.sh $(CC) $(LD)

clearconfig:
	@echo "Clear matrix build configuration"
	# Create default config
	echo " " > LDFLAGS
	echo "no" > CONFIG_WITH_blas_AND_gsl


## EITHER THIS

matrix.o: matrix.c
	$(CC) $(CFLAGS) $(INCLUDE) -c matrix.c 

## OR THIS

#template_block.o: template_block.cpp
#	$(CC) $(CFLAGS) $(INCLUDE) -c template_block.cpp


