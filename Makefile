CC = cc
CPP = c++
#LD = ld
# Use g++ as linker because of c++
LD = g++
SH = bash


# create list of modules
#MODULES = scope template muparser basic_ldblocks rt_server
MODULES := $(shell ls modules)

# if modules provide header file needed by the base component
#MODULES_INCLUDE := modules/rt_server
# -I$(MODULES_INCLUDE)

# detect system type
host-type := $(shell arch)
ortd_root := $(shell pwd)

ifeq ($(host-type),x86_64)
# 64 Bit
CFLAGS = -fPIC -O2
LDFLAGS = -shared
else
# 32 Bit
CFLAGS = -O2 
LDFLAGS = -shared
endif



libdyn_generic_exec: lib
	$(CPP) -I.. -L. -O2 -lortd -lm libdyn_generic_exec.cpp -o libdyn_generic_exec
 
lib: $(MODULES) module_list__.o libdyn.o libdyn_blocks.o libdyn_cpp.o block_lookup.o plugin_loader.o irpar.o log.o libilc.o
	$(LD) $(LDFLAGS) module_list__.o libdyn.o libdyn_blocks.o libdyn_cpp.o block_lookup.o plugin_loader.o irpar.o log.o libilc.o Linux_Target/*.o all_Targets/*.o -lm -lpthread -lrt -ldl -o libortd.so
	ar rvs libortd.a module_list__.o libdyn.o libdyn_blocks.o libdyn_cpp.o block_lookup.o irpar.o log.o libilc.o Linux_Target/*.o all_Targets/*.o
	$(LD) $(LDFLAGS) module_list__.o libdyn.o libdyn_blocks.o libdyn_cpp.o block_lookup.o plugin_loader.o irpar.o log.o libilc.o all_Targets/*.o -lm -lpthread -lrt -ldl -o libortd_hart.so
	ar rvs libortd_hart.a module_list__.o libdyn.o libdyn_blocks.o libdyn_cpp.o block_lookup.o irpar.o log.o libilc.o all_Targets/*.o

clean:
	rm -f *.o *.so *.a libdyn_generic_exec Linux_Target/* all_Targets/*
	rm -f module_list module_list__.c module_list__.h
	for d in $(MODULES); do (cd modules/$$d; $(MAKE) clean ); done
	#for d in $(MODULES); do (echo "cd to modules/$$d"; cd modules/$$d; pwd; cd $(ortd_root)  ); done


#$(MODULES)clean:
#	$(MAKE) --directory=modules/$@ clean

.PHONY: clear_scilab_modules
clear_scilab_modules:
	rm -f scilab/modules_loader.sce scilab/ld_toolbox/initialrun/modules_loader.sce
	rm -f module_list module_list__.c module_list__.h

	# Create header for module_list.c_
	echo "int libdyn_siminit_modules(struct dynlib_simulation_t *sim);" > module_list__.h

	echo "#include \"libdyn.h\"" > module_list__.c
	echo "#include \"module_list__.h\"" >> module_list__.c
	echo "int libdyn_siminit_modules(struct dynlib_simulation_t *sim) {" >> module_list__.c

	echo "cleaned up scilab modules"


# all .o files of modules are collected within Linux_Target
.PHONY: $(MODULES) 
$(MODULES): clear_scilab_modules
	$(MAKE) --directory=modules/$@

	# extract all object files from evry module for all targets 
	cd all_Targets ; \
	if [ -f ../modules/$@/module.a ] ; then ar -x ../modules/$@/module.a  ; fi  

	cd Linux_Target ; \
	if [ -f ../modules/$@/module_Linux.a ] ; then ar -x ../modules/$@/module_Linux.a  ; fi  

	if [ -f modules/$@/scilab_loader.sce ] ; \
	then \
	  /bin/cat modules/$@/scilab_loader.sce >> scilab/modules_loader.sce ; \
	  /bin/cat modules/$@/scilab_loader.sce >> scilab/ld_toolbox/initialrun/modules_loader.sce ; \
	fi

	# Create module list
	echo $@ >> module_list

	# Create loader C-Code
	echo "  libdyn_module_$@_siminit(sim, 0);" >> module_list__.c

	echo "int libdyn_module_$@_siminit(struct dynlib_simulation_t *sim, int bid_ofs);" >> module_list__.h

	echo "--------> Processed module" $@


.PHONY: finish_module_list
finish_module_list: $(MODULES)
	# Create post header for module_list.c_
	echo "}" >> module_list__.c

# Baustelle
.PHONY: blockid_offsets
blockid_offsets:
	start_ofs=10000 ; \
	count=1000 ; \
	for d in $(MODULES); do (cd modules/$$d; echo gak $$count; count=$$[count+1000] ; echo gak $$count   ); done

module_list__.o: finish_module_list module_list__.c
	$(CC) $(CFLAGS) -c module_list__.c


libdyn.o: libdyn.c
	$(CC) $(CFLAGS) -c libdyn.c
 
libdyn_blocks.o: libdyn_blocks.c
	$(CC) $(CFLAGS) -c libdyn_blocks.c

block_lookup.o: block_lookup.c
	$(CC) $(CFLAGS) -c block_lookup.c

plugin_loader.o: plugin_loader.c
	$(CC) $(CFLAGS) -c plugin_loader.c

irpar.o: irpar.c
	$(CC) $(CFLAGS) -c irpar.c

log.o: log.c
	$(CC) $(CFLAGS) -c log.c

libilc.o: libilc.c
	$(CC) $(CFLAGS) -c libilc.c

libdyn_cpp.o: libdyn_cpp.cpp
	$(CC) $(CFLAGS) -c libdyn_cpp.cpp

#
# Installation
#

install: libdyn_generic_exec lib
	sudo cp libortd.so /usr/local/lib
	sudo cp libortd.a /usr/local/lib
	sudo cp libortd_hart.so /usr/local/lib
	sudo cp libortd_hart.a /usr/local/lib
	sudo ldconfig
	sudo cp libdyn_generic_exec /usr/local/bin




clean_scilabdir:
	rm -f scilabdir.conf

install_toolbox: scilabdir.conf
	$(SH) install_toolbox

scilabdir.conf:
	$(SH) find_scilab
