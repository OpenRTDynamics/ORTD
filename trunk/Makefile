# Default Toolchain
export CC = cc
export CPP = c++

# Use g++ as linker because of c++
export LD = g++
export SH = sh

#
# CFGLAGS, some more are added later on depending on the target
#

#export CFLAGS += -DDEBUG





# create list of modules
MODULES := $(shell ls modules)

# get the current dir and assume it is the one of ortd
export ortd_root := $(shell pwd)

# Configure target
export target := $(shell cat target.conf)

# Default which should be overwritten
targetmacro=__HMM

# Name of the resulting ortd binary
ORTD_INTERPRETERNAME=ortd

# System configuration
SYSTEM_LIBRARY_FOLDER=/usr/local/lib
SYSTEM_BINARY_FOLDER=/usr/local/bin

ifeq ($(target),LINUX)
  targetmacro=__ORTD_TARGET_LINUX

  # detect host type
	export host-type := $(shell arch)
	ifeq ($(host-type),x86_64)
	# 64 Bit
	export CFLAGS += -fPIC
	endif

	ifeq ($(host-type),armv7l)
	# 64 Bit
	export CFLAGS += -fPIC
	endif

  # Detect system type and set Fflags
    export CFLAGS += -O2 -D$(targetmacro)
    export CFLAGS += 
    export INCLUDE +=  -I$(ortd_root)
    export LDFLAGS += 
    export LD_LIBRARIES += -lm -lpthread -lrt -ldl
endif

ifeq ($(target),LINUX_DEBUG)
  targetmacro=__ORTD_TARGET_LINUX

  # detect host type
	export host-type := $(shell arch)
	ifeq ($(host-type),x86_64)
	# 64 Bit
	export CFLAGS += -fPIC
	endif

	ifeq ($(host-type),armv7l)
	# 64 Bit
	export CFLAGS += -fPIC
	endif

  # Detect system type and set Fflags
#    export CFLAGS += -O2 -D$(targetmacro) -DDEBUG
    export CFLAGS += -O2 -D$(targetmacro) 
    export CFLAGS += -g 
    export INCLUDE +=  -I$(ortd_root)
    export LDFLAGS += 
    export LD_LIBRARIES += -lm -lpthread -lrt -ldl
endif

ifeq ($(target),LINUX_x86_32)
  targetmacro=__ORTD_TARGET_LINUX

  SYSTEM_LIBRARY_FOLDER=/usr/lib32
  ORTD_INTERPRETERNAME=ortd_x86_32

    # 32 Bit
    export CFLAGS += -m32 -O2 -D$(targetmacro)
    export INCLUDE +=  -I$(ortd_root)
    export LDFLAGS += -m32 
    export LD_LIBRARIES += -lm -lpthread -lrt -ldl
endif

ifeq ($(target),RTAI_COMPATIBLE_x86_32)
  targetmacro=__ORTD_TARGET_RTAI

    # 32 Bit
    export CFLAGS += -m32 -g -O2 -D$(targetmacro)
    export INCLUDE +=  -I$(ortd_root)
    export LDFLAGS += -m32 
    export LD_LIBRARIES += -lm -lpthread -lrt -ldl
endif


ifeq ($(target),LINUX_pentium)
  targetmacro=__ORTD_TARGET_LINUX
  ORTD_INTERPRETERNAME=ortd32

    # 32 Bit
    export CFLAGS += -m32 -march=pentium -O2 -D$(targetmacro)
    export INCLUDE +=  -I$(ortd_root)
    export LDFLAGS += -m32
    export LD_LIBRARIES += -lm -lpthread -lrt -ldl
endif


ifeq ($(target),RTAI_COMPATIBLE) 
  targetmacro=__ORTD_TARGET_RTAI

  # detect host type
	export host-type := $(shell arch)
	ifeq ($(host-type),x86_64)
	# 64 Bit
	export CFLAGS += -fPIC
	endif

	ifeq ($(host-type),armv7l)
	# 64 Bit
	export CFLAGS += -fPIC
	endif


    export CFLAGS += -g -O2 -D$(targetmacro)
    export INCLUDE +=  -I$(ortd_root)
    export LDFLAGS += 
    export LD_LIBRARIES += -lm -lpthread -lrt -ldl
endif

ifeq ($(target),XCOS_RTAI_COMPATIBLE) 
  # for embedding ORTD into XCOS/SCICOS generated rtai code
  targetmacro=__ORTD_TARGET_RTAI -D __ORTD_TARGET_XCOS

  # detect host type
	export host-type := $(shell arch)
	ifeq ($(host-type),x86_64)
	# 64 Bit
	export CFLAGS += -fPIC
	endif

	ifeq ($(host-type),armv7l)
	# 64 Bit
	export CFLAGS += -fPIC
	endif


    export CFLAGS += -g -O2 -D$(targetmacro)
    export INCLUDE +=  -I$(ortd_root)
    export LDFLAGS += 
    export LD_LIBRARIES += -lm -lpthread -lrt -ldl
endif



ifeq ($(target),CYGWIN) # TODO 
  targetmacro=__ORTD_TARGET_CYGWIN

  export CFLAGS += -g -O2 -D$(targetmacro)
  export INCLUDE +=  -I$(ortd_root)
  export LDFLAGS += 
  export LD_LIBRARIES += -lm -lpthread -lrt -ldl
endif

ifeq ($(target),ANDROID_ARM) 
  targetmacro=__ORTD_TARGET_ANDROID

#  export CFLAGS += -I/home/chr/bin/AndroidArmToolchain/sysroot/usr/include --sysroot=/home/chr/bin/AndroidArmToolchain/sysroot -O2 -D$(targetmacro) 
  
  export CFLAGS += -fPIC -O2 -D$(targetmacro) 
  export INCLUDE +=  -I$(ortd_root)
  export LDFLAGS += 

  # -lpthread  & -lrt are not needed in Android
  export LD_LIBRARIES += -lm -ldl -llog -landroid -lOpenSLES

  # use cross compile tool-chain from Android NDK
  export CC = arm-linux-androideabi-gcc
  export CPP = arm-linux-androideabi-c++
  export LD = arm-linux-androideabi-g++
  export SH = sh
endif

ifeq ($(target),ANDROID_ARM_NEON) 
  targetmacro=__ORTD_TARGET_ANDROID

#  export CFLAGS += -I/home/chr/bin/AndroidArmToolchain/sysroot/usr/include --sysroot=/home/chr/bin/AndroidArmToolchain/sysroot -O2 -D$(targetmacro) 
  export CFLAGS += -fPIC  -O2 -D$(targetmacro) -march=armv7-a -mfloat-abi=softfp -mfpu=neon
  export INCLUDE +=  -I$(ortd_root)
  export LDFLAGS += -Wl,--fix-cortex-a8

  # -lpthread  & -lrt are not needed in Android
  export LD_LIBRARIES += -lm -ldl -llog -landroid 

  # use cross compile tool-chain from Android NDK
  export CC = arm-linux-androideabi-gcc
  export CPP = arm-linux-androideabi-c++
  export LD = arm-linux-androideabi-g++
  export SH = sh
endif


#
# This tells the sub-makefiles that the variables 
#from this makefile are available
#

export main_makefile_invoked := yes






all: libdyn_generic_exec_static libdyn_generic_exec bin/ortd bin/ortd_static lib
	#echo "------- Build finished: Now you can do > make install <  -------"
	cat documentation/finish_info.txt

#
# FIXME: lib: wird nicht geupdated, wenn etwas in den Modulen geändert wird
#

bin/ortd_static: libdyn_generic_exec_static
	cp bin/libdyn_generic_exec_static bin/ortd_static

bin/ortd: libdyn_generic_exec
	cp bin/libdyn_generic_exec bin/ortd

libdyn_generic_exec_static: lib libdyn_generic_exec.o
	echo "Static binary is disabled"
	$(LD) $(LDFLAGS) libdyn_generic_exec.o libortd.a `cat tmp/LDFALGS.list`  $(LD_LIBRARIES) -o bin/libdyn_generic_exec_static 
 
libdyn_generic_exec: lib libdyn_generic_exec.o
#	$(CPP) -I.. -L. -O2 -lortd -lm libdyn_generic_exec.cpp -o libdyn_generic_exec
	$(LD) $(LDFLAGS)  libdyn_generic_exec.o -L. -lortd `cat tmp/LDFALGS.list`  $(LD_LIBRARIES) -o bin/libdyn_generic_exec  -Wl,-R,'$$ORIGIN/:$$ORIGIN/lib'
 
libdyn_generic_exec.o: libdyn_generic_exec.cpp lib IncompiledVariables.h
	$(CPP) -I.. -L. $(CFLAGS) -c libdyn_generic_exec.cpp

lib: $(MODULES) module_list__.o libdyn.o libdyn_blocks.o libdyn_cpp.o block_lookup.o plugin_loader.o irpar.o log.o realtime.o libilc.o
	$(LD) -shared $(LDFLAGS)      module_list__.o libdyn.o libdyn_blocks.o libdyn_cpp.o block_lookup.o plugin_loader.o irpar.o log.o realtime.o libilc.o          all_Targets/*.o `cat tmp/LDFALGS.list` $(LD_LIBRARIES) -o libortd.so
	ar rvs libortd.a      module_list__.o libdyn.o libdyn_blocks.o libdyn_cpp.o block_lookup.o plugin_loader.o irpar.o log.o realtime.o libilc.o                  all_Targets/*.o

	# This is used for RTAI code generation within the Hart-Toolbox. Therefore, some parts are skipped
	# FIXME remove this
	#$(LD) -shared $(LDFLAGS)      module_list__.o libdyn.o libdyn_blocks.o libdyn_cpp.o block_lookup.o plugin_loader.o irpar.o log.o realtime.o libilc.o                  all_Targets/*.o  $(LD_LIBRARIES) -o libortd_hart.so
	#ar rvs libortd_hart.a module_list__.o libdyn.o libdyn_blocks.o libdyn_cpp.o block_lookup.o plugin_loader.o irpar.o log.o realtime.o libilc.o                  all_Targets/*.o

scilabhelp:
	(cd scilab; ./build_toolbox.sh)

cleanBuildFiles:
	rm -f *.o Linux_Target/* all_Targets/* 
	rm -f module_list module_list__.c module_list__.h
	for d in $(MODULES); do (cd modules/$$d; $(MAKE) clean ); done	
	rm -f OpenRTDynamics_source.sh

clean: cleanBuildFiles
	rm -f *.so *.a
	rm -f bin/libdyn_generic_exec bin/libdyn_generic_exec_static bin/ortd bin/ortd_static

superclean: clean
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

	@echo "Macros are: $(cat tmp/MACROS.list)"
	@echo "New CFLAGS are: "$(CFLAGS)


	# Create header for module_list.c_
	echo "int libdyn_siminit_modules(struct dynlib_simulation_t *sim);" > module_list__.h

	echo "#include \"libdyn.h\"" > module_list__.c
	echo "#include \"module_list__.h\"" >> module_list__.c
	echo "int libdyn_siminit_modules(struct dynlib_simulation_t *sim) {" >> module_list__.c


	@echo "cleaned up scilab modules"



# all .o files of modules are collected
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
	perl BuildScripts/extract_documentation.pl $@ modules/$@/scilab_loader.sce >> tmp/block_list.txt

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


IncompiledVariables.h:
	# create in-compiled variables file
	sh Write_IncompiledVariables.sh
	

config:	config_modules IncompiledVariables.h
	@echo "Configuration finished"

# Call make config for every module
config_modules:
	( for d in $(MODULES); do (  make --directory=modules/$$d config   ); done ; exit 0 )

clearconfig:	clearconfig_modules
	rm IncompiledVariables.h
	@echo "Configuration cleared"

# Call make config for every module
clearconfig_modules:
	( for d in $(MODULES); do (  make --directory=modules/$$d clearconfig   ); done ; exit 0 )

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

libdyn_cpp.o: libdyn_cpp.cpp IncompiledVariables.h
	$(CC) $(CFLAGS) -c libdyn_cpp.cpp

#
# Installation
#

.PHONY: install

install: all
	sudo cp libortd.so $(SYSTEM_LIBRARY_FOLDER)
	sudo cp libortd.a $(SYSTEM_LIBRARY_FOLDER)
	sudo ldconfig
	sudo cp bin/libdyn_generic_exec $(SYSTEM_BINARY_FOLDER)
	sudo cp bin/libdyn_generic_exec_scilab $(SYSTEM_BINARY_FOLDER)
	sudo cp bin/ortd  $(SYSTEM_BINARY_FOLDER)/$(ORTD_INTERPRETERNAME)
	sudo cp bin/ortd_static  $(SYSTEM_BINARY_FOLDER)/$(ORTD_INTERPRETERNAME)_static
	sudo chmod +x  $(SYSTEM_BINARY_FOLDER)/libdyn_generic_exec_scilab
	chmod +x bin/libdyn_generic_exec_scilab
	chmod +x bin/libdyn_generic_exec_static_scilab

homeinstall: all
	mkdir -p ~/bin
	cp bin/ortd_static ~/bin/$(ORTD_INTERPRETERNAME)_static
	ln -sf ~/bin/$(ORTD_INTERPRETERNAME)_static ~/bin/$(ORTD_INTERPRETERNAME)
	@echo "Copied binary ortd to ~/bin"
	
package:
	# Create a self extracting shell archieve OpenRTDynamics_source.sh
	bash AutoBuild/LINUX_source/build.sh this . OpenRTDynamics_source

demo: #bin/ortd_static
	( cd examples/demo;  ../../bin/ortd_static --baserate=20 --rtmode 1 -s oscillator -i 901 -l 300 )

clean_scilabdir:
	rm -f scilabdir.conf

## install toolbox may require to compile the toolbox again through make install, since scilabdir.conf is now available which is used by the scilab-module
install_toolbox: scilabdir.conf 
	$(SH) install_toolbox

.PHONY: scilabdir.conf
scilabdir.conf:
	$(SH) find_scilab


