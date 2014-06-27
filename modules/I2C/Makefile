MODULENAME := I2C
blockid_start := 39301


# Get the include path to the ORTD framework
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
	@echo "Module" $(MODULENAME) "done"

plugin: lib pluginLoader.o 
	g++ -shared I2C_blocks.o pluginLoader.o -o ortd_plugin.so
#	g++ -shared module.a pluginLoader.o -o ortd_plugin.so

lib: I2C_blocks.o 
	ar rvs module.a I2C_blocks.o 

config: scilab_loader.sce
	echo $(blockid_start) > blockid_start.conf.tmp
	
scilab_loader.sce: scilab_loader_Prototype.sce
	cat scilab_loader_Prototype.sce | sed 's/ORTD_BLOCKIDSTART/$(blockid_start)/' > scilab_loader.sce

clean:
	rm -f *.o module.a module_Linux.a ortd_plugin.so blockid_start.conf.tmp scilab_loader.sce


I2C_blocks.o: config I2C_blocks.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -c I2C_blocks.cpp

#Template_SharedObjects.o: Template_SharedObjects.cpp	
#	$(CC) $(CFLAGS) $(INCLUDE) -c Template_SharedObjects.cpp
	
pluginLoader.o: pluginLoader.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -c pluginLoader.cpp

