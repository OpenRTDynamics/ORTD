MODULENAME := TemplateModule_V2
blockid_start := 999911111


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

# Automatically compile and link all C/C++ sources found in src/
program_C_SRCS := $(wildcard src/*.c)
program_CXX_SRCS := $(wildcard src/*.cpp)
program_C_OBJS := ${program_C_SRCS:.c=.o}
program_CXX_OBJS := ${program_CXX_SRCS:.cpp=.o}
program_OBJS := $(program_C_OBJS) $(program_CXX_OBJS)
program_INCLUDE_DIRS :=
program_LIBRARY_DIRS :=
program_LIBRARIES :=

INCLUDE += -Ihart_includes -Imodule_src

CFLAGS += $(INCLUDE) -D_TTY_POSIX_
CPPFLAGS =  $(CFLAGS)



all: lib
	@echo "Module" $(MODULENAME) "done"

plugin: lib pluginLoader.o 
	g++ -shared TemplateModule_V2_blocks.o pluginLoader.o  $(program_OBJS) -o ortd_plugin.so
#	g++ -shared module.a pluginLoader.o -o ortd_plugin.so

lib: TemplateModule_V2_blocks.o  $(program_OBJS)
	ar rvs module.a TemplateModule_V2_blocks.o  $(program_OBJS)

config: scilab_loader.sce
	echo $(blockid_start) > blockid_start.conf.tmp
	
scilab_loader.sce: scilab_loader_Prototype.sce
	cat scilab_loader_Prototype.sce | sed 's/ORTD_BLOCKIDSTART/$(blockid_start)/g' > scilab_loader.sce

clean:
	rm -f *.o module.a module_Linux.a ortd_plugin.so blockid_start.conf.tmp scilab_loader.sce $(program_OBJS)


TemplateModule_V2_blocks.o: config TemplateModule_V2_blocks.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -c TemplateModule_V2_blocks.cpp

#Template_SharedObjects.o: Template_SharedObjects.cpp	
#	$(CC) $(CFLAGS) $(INCLUDE) -c Template_SharedObjects.cpp
	
pluginLoader.o: pluginLoader.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -c pluginLoader.cpp

