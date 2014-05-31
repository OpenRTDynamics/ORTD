# Change this to the folder where you installed ORTD
# if you plan to use this as a module, just put "../..".
ORTDFOLDER := $(shell ortd --GetIncludePath)


ifeq ($(main_makefile_invoked),yes)

else
	# If this makefile is just run within this directory

	CC = cc
	CPP = c++
	LD = ld

	# detect system type
	host-type := $(shell arch)

	ifeq ($(host-type),x86_64)
	# 64 Bit
	CFLAGS = -fPIC -O2 -g -I$(ORTDFOLDER)
	INCLUDE = -I$(ORTDFOLDER)
	LDFLAGS = -shared
	else
	# 32 Bit
	CFLAGS = -O2 -I$(ORTDFOLDER) -g
	INCLUDE = -I$(ORTDFOLDER)
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
	rm -f *.o module.a module_Linux.a ortd_plugin.so


Template_blocks.o: Template_blocks.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -c Template_blocks.cpp

#Template_SharedObjects.o: Template_SharedObjects.cpp	
#	$(CC) $(CFLAGS) $(INCLUDE) -c Template_SharedObjects.cpp
	
pluginLoader.o: pluginLoader.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -c pluginLoader.cpp

