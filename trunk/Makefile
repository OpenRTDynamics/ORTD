CC = cc
CPP = c++
LD = ld


# detect system type
host-type := $(shell arch)

ifeq ($(host-type),x86_64)
# 64 Bit
CFLAGS = -fPIC -O2
LDFLAGS = -shared
else
# 32 Bit
CFLAGS = -O2
LDFLAGS = -shared
endif

# create list of modules
#MODULES = scope
MODULES := $(shell ls modules)


install: libdyn_generic_exec lib
	sudo cp libortd.so /usr/local/lib
	sudo cp libortd.a /usr/local/lib
	sudo cp libortd_hart.so /usr/local/lib
	sudo cp libortd_hart.a /usr/local/lib
	sudo ldconfig
	sudo cp libdyn_generic_exec /usr/local/bin

libdyn_generic_exec: lib
	$(CPP) -I.. -L. -O2 -lortd -lm libdyn_generic_exec.cpp -o libdyn_generic_exec
 
lib: $(MODULES) libdyn.o libdyn_blocks.o libdyn_cpp.o block_lookup.o irpar.o log.o libilc.o
	$(LD) $(LDFLAGS) libdyn.o libdyn_blocks.o libdyn_cpp.o block_lookup.o irpar.o log.o libilc.o Linux_Target/*.o all_Targets/*.o -lm -lpthread -lrt -o libortd.so
	ar rvs libortd.a libdyn.o libdyn_blocks.o libdyn_cpp.o block_lookup.o irpar.o log.o libilc.o Linux_Target/*.o all_Targets/*.o
	$(LD) $(LDFLAGS) libdyn.o libdyn_blocks.o libdyn_cpp.o block_lookup.o irpar.o log.o libilc.o all_Targets/*.o -lm -lpthread -lrt -o libortd_hart.so
	ar rvs libortd_hart.a libdyn.o libdyn_blocks.o libdyn_cpp.o block_lookup.o irpar.o log.o libilc.o all_Targets/*.o

clean:
	rm -f *.o *.so libdyn_generic_exec Linux_Target/* all_Targets/*
	for d in modules/$(MODULES); do (cd $$d; $(MAKE) clean ); done



#$(MODULES)clean:
#	$(MAKE) --directory=modules/$@ clean

.PHONY: clear_scilab_modules
clear_scilab_modules:
	rm -f scilab/modules_loader.sce scilab/ld_toolbox/initialrun/modules_loader.sce
	echo "cleaned up scilab modules"


# all .o files of modules are collected within Linux_Target
.PHONY: $(MODULES) 
$(MODULES): clear_scilab_modules
	$(MAKE) --directory=modules/$@
	cd all_Targets ; \
	ar -x ../modules/$@/module.a
	cd Linux_Target ; \
	ar -x ../modules/$@/module_Linux.a
	cat modules/$@/scilab_loader.sce >> scilab/modules_loader.sce
	cat modules/$@/scilab_loader.sce >> scilab/ld_toolbox/initialrun/modules_loader.sce
	echo "--------> Processed module" $@



libdyn.o: libdyn.c
	$(CC) $(CFLAGS) -c libdyn.c
 
libdyn_blocks.o: libdyn_blocks.c
	$(CC) $(CFLAGS) -c libdyn_blocks.c

block_lookup.o: block_lookup.c
	$(CC) $(CFLAGS) -c block_lookup.c

irpar.o: irpar.c
	$(CC) $(CFLAGS) -c irpar.c

log.o: log.c
	$(CC) $(CFLAGS) -c log.c

libilc.o: libilc.c
	$(CC) $(CFLAGS) -c libilc.c

libdyn_cpp.o: libdyn_cpp.cpp
	$(CC) $(CFLAGS) -c libdyn_cpp.cpp


