
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
	@echo "Module Template done"

plugin: lib pluginLoader.o
	g++ -shared Template_blocks.o pluginLoader.o -o ortd_plugin.so
#	g++ -shared module.a pluginLoader.o -o ortd_plugin.so

lib: Template_blocks.o
	ar rvs module.a Template_blocks.o  

clean:
	rm -f *.o module.a module_Linux.a


Template_blocks.o: Template_blocks.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -c Template_blocks.cpp

pluginLoader.o: pluginLoader.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -c pluginLoader.cpp

