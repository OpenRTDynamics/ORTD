export CC = cc
export CPP = c++
#LD = ld
# Use g++ as linker because of c++
export LD = g++
export SH = bash


# create list of modules
#MODULES = scope template muparser basic_ldblocks rt_server
MODULES := $(shell ls modules)

# if modules provide header file needed by the base component
#MODULES_INCLUDE := modules/rt_server
# -I$(MODULES_INCLUDE)

# detect system type
export host-type := $(shell arch)
export ortd_root := $(shell pwd)

# Configure target
export target := $(shell cat target.conf)

targetmacro=__HMM

ifeq ($(target),LINUX)
  targetmacro=__ORTD_TARGET_LINUX

  # Detect system type and set Fflags
  ifeq ($(host-type),x86_64)
    # 64 Bit
    export CFLAGS = -g -fPIC -O2 -D$(targetmacro)
    export INCLUDE =  -I$(ortd_root)
    export LDFLAGS = 
  else
    # 32 Bit
    export CFLAGS = -g -O2 -D$(targetmacro)
    export INCLUDE =  -I$(ortd_root)
    export LDFLAGS = 
  endif

endif

ifeq ($(target),LINUX_AND_HART)
  targetmacro=__ORTD_TARGET_LINUX

  # Detect system type and set Fflags
  ifeq ($(host-type),x86_64)
    # 64 Bit
    export CFLAGS = -g -fPIC -O2 -D$(targetmacro)
    export INCLUDE =  -I$(ortd_root)
    export LDFLAGS = -l hart
  else
    # 32 Bit
    export CFLAGS = -g -O2 -D$(targetmacro)
    export INCLUDE =  -I$(ortd_root)
    export LDFLAGS = -l hart
  endif

endif


ifeq ($(target),LINUX_x86_32)
  targetmacro=__ORTD_TARGET_LINUX

    # 32 Bit
    export CFLAGS = -m32 -g -O2 -D$(targetmacro)
    export INCLUDE =  -I$(ortd_root)
    export LDFLAGS = -m32 
endif

ifeq ($(target),RTAI_COMPATIBLE_x86_32)
  targetmacro=__ORTD_TARGET_RTAI

    # 32 Bit
    export CFLAGS = -m32 -g -O2 -D$(targetmacro)
    export INCLUDE =  -I$(ortd_root)
    export LDFLAGS = -m32 
endif


ifeq ($(target),LINUX_pentium)
  targetmacro=__ORTD_TARGET_LINUX

    # 32 Bit
    export CFLAGS = -m32 -march=pentium -O2 -D$(targetmacro)
    export INCLUDE =  -I$(ortd_root)
    export LDFLAGS = -m32

endif


ifeq ($(target),RTAI_COMPATIBLE) 
  targetmacro=__ORTD_TARGET_RTAI

  # Detect system type and set Fflags
  ifeq ($(host-type),x86_64)
    # 64 Bit
    export CFLAGS = -g -fPIC      -D$(targetmacro)
    export INCLUDE =  -I$(ortd_root)
    export LDFLAGS = 
  else
    # 32 Bit
    export CFLAGS = -g -O2 -D$(targetmacro)
    export INCLUDE =  -I$(ortd_root)
    export LDFLAGS = 
  endif

endif

ifeq ($(target),CYGWIN) 
  targetmacro=__ORTD_TARGET_CYGWIN

  export CFLAGS = -g -O2 -D$(targetmacro)
  export INCLUDE =  -I$(ortd_root)
  export LDFLAGS = 
endif

# Tell the sub makefiles that the variables from this makefile are available
export main_makefile_invoked := yes

# Detect system type and set Fflags
# ifeq ($(host-type),x86_64)
# # 64 Bit
# export CFLAGS = -g -fPIC -O2 -D$(targetmacro)
# export INCLUDE =  -I$(ortd_root)
# export LDFLAGS = -shared
# else
# # 32 Bit
# export CFLAGS = -g -O2 -D$(targetmacro)
# export INCLUDE =  -I$(ortd_root)
# export LDFLAGS = -shared
# endif

all: libdyn_generic_exec_static libdyn_generic_exec lib
	#echo "------- Build finished: Now you can do > make install <  -------"
	cat documentation/finish_info.txt

#
# FIXME: lib: wird nicht geupdated, wenn etwas in den Modulen geändert wird
#
libdyn_generic_exec_static: lib libdyn_generic_exec.o
	echo "Static binary is disabled"
	$(LD) $(LDFLAGS) libdyn_generic_exec.o libortd.a `cat tmp/LDFALGS.list` -lm -lpthread -lrt -ldl -o bin/libdyn_generic_exec_static
 
libdyn_generic_exec: lib libdyn_generic_exec.o
#	$(CPP) -I.. -L. -O2 -lortd -lm libdyn_generic_exec.cpp -o libdyn_generic_exec
	$(LD) $(LDFLAGS)  libdyn_generic_exec.o -L. -lortd `cat tmp/LDFALGS.list` -lm -lpthread -lrt -ldl -o bin/libdyn_generic_exec
 
libdyn_generic_exec.o: libdyn_generic_exec.cpp lib
	$(CPP) -I.. -L. $(CFLAGS) -c libdyn_generic_exec.cpp

lib: $(MODULES) module_list__.o libdyn.o libdyn_blocks.o libdyn_cpp.o block_lookup.o plugin_loader.o irpar.o log.o realtime.o libilc.o
	$(LD) -shared $(LDFLAGS)      module_list__.o libdyn.o libdyn_blocks.o libdyn_cpp.o block_lookup.o plugin_loader.o irpar.o log.o realtime.o libilc.o                  all_Targets/*.o -lm -lpthread -lrt -ldl -o libortd.so
	ar rvs libortd.a      module_list__.o libdyn.o libdyn_blocks.o libdyn_cpp.o block_lookup.o plugin_loader.o irpar.o log.o realtime.o libilc.o                  all_Targets/*.o
	$(LD) -shared $(LDFLAGS)      module_list__.o libdyn.o libdyn_blocks.o libdyn_cpp.o block_lookup.o plugin_loader.o irpar.o log.o realtime.o libilc.o                  all_Targets/*.o -lm -lpthread -lrt -ldl -o libortd_hart.so

	# This is used for RTAI code generation within the Hart-Toolbox. Therefore, some parts are skipped
	ar rvs libortd_hart.a module_list__.o libdyn.o libdyn_blocks.o libdyn_cpp.o block_lookup.o plugin_loader.o irpar.o log.o realtime.o libilc.o                  all_Targets/*.o

scilabhelp:
	(cd scilab; ./build_toolbox.sh)

clean:
	rm -f *.o *.so *.a libdyn_generic_exec libdyn_generic_exec_static Linux_Target/* all_Targets/* bin/*
	rm -f module_list module_list__.c module_list__.h
	for d in $(MODULES); do (cd modules/$$d; $(MAKE) clean ); done
	#for d in $(MODULES); do (echo "cd to modules/$$d"; cd modules/$$d; pwd; cd $(ortd_root)  ); done

superclean:
	find . -name "*~" -print0 | xargs -0 rm

#$(MODULES)clean:
#	$(MAKE) --directory=modules/$@ clean

# Better call this init modules
.PHONY: clear_scilab_modules
clear_scilab_modules:
	rm -f scilab/modules_loader.sce scilab/ld_toolbox/initialrun/modules_loader.sce
	rm -f module_list module_list__.c module_list__.h

	# Clear Documentation 
	rm -f tmp/block_list.txt
	touch tmp/block_list.txt

	# Create new LDFLAGS list
	rm -f tmp/LDFALGS.list  # clear list of libraries
	touch tmp/LDFALGS.list # create a new one

	rm -f tmp/MACROS.list  # clear list of MACROS
	touch tmp/MACROS.list # create a new one

	# Collect the LDFLAGS and MACROS ...
	sh collect_config.sh

	export CFLAGS="$(CFLAGS) $(cat tmp/MACROS.list)"

	echo "New CFLAGS are: "$(CFLAGS)


	# Create header for module_list.c_
	echo "int libdyn_siminit_modules(struct dynlib_simulation_t *sim);" > module_list__.h

	echo "#include \"libdyn.h\"" > module_list__.c
	echo "#include \"module_list__.h\"" >> module_list__.c
	echo "int libdyn_siminit_modules(struct dynlib_simulation_t *sim) {" >> module_list__.c


	echo "cleaned up scilab modules"



# all .o files of modules are collected within Linux_Target
.PHONY: $(MODULES) 
$(MODULES): clear_scilab_modules
	@echo
	@echo "............................................................."
	@echo "               Going to module $@ "
	@echo "............................................................."
	@echo
	

	# Compile the module
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

	# Create Documentation
	perl extract_documentation.pl $@ modules/$@/scilab_loader.sce >> tmp/block_list.txt

	# Create loader C-Code
	echo "  libdyn_module_$@_siminit(sim, 0);" >> module_list__.c

	echo "int libdyn_module_$@_siminit(struct dynlib_simulation_t *sim, int bid_ofs);" >> module_list__.h

	@echo "Successfully Processed module" $@


.PHONY: finish_module_list
finish_module_list: $(MODULES)
	# Create post header for module_list.c_
	echo "}" >> module_list__.c

	# copy the documentation
	cp tmp/block_list.txt documentation/List_of_Module_Blocks.txt

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

realtime.o: realtime.c
	$(CC) $(CFLAGS) -c realtime.c

libilc.o: libilc.c
	$(CC) $(CFLAGS) -c libilc.c

libdyn_cpp.o: libdyn_cpp.cpp
	$(CC) $(CFLAGS) -c libdyn_cpp.cpp

#
# Installation
#

.PHONY: install

install: all
	sudo cp libortd.so /usr/local/lib
	sudo cp libortd.a /usr/local/lib
	sudo cp libortd_hart.so /usr/local/lib
	sudo cp libortd_hart.a /usr/local/lib
	sudo ldconfig
	sudo cp bin/libdyn_generic_exec /usr/local/bin




clean_scilabdir:
	rm -f scilabdir.conf

## install toolbox may require compile the toolbox again through make install, since scilabdir.conf is now available which is used by the scilab-module
install_toolbox: scilabdir.conf 
	$(SH) install_toolbox

.PHONY: scilabdir.conf
scilabdir.conf:
	$(SH) find_scilab


