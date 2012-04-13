


# Path to Scilab
SCIDIR = /opt/scilab-4.1.2

# Path to hart
HARTDIR= /home/control/svn/hart/macros/RtaiCodeGen/../..

# Libraries to fetch from the system
OTHERLIBS =  -lortd  -lgsl -lblas -lgsl -lblas -lgfortran -lrt -lpthread 

# MACROS: include -DORTD_REGISTER_SCICOSBLOCKS if you want some Scicos-Blocks to be registeres to ORTD,
#         so you can nest them into ORTD schematics
#         Enabling this will call C-function named ORTD_register_scicosblocks() that has to be included within
#         an archive e.g. within the libs/ subdirectory.
MACROS = -DORTD_REGISTER_SCICOSBLOCKS

ORTD_INCLUDE = -I..

###################################################


all: ortd_scicos_exec


# Fetch the static libs from the libs/ subdir, where the user can place its *.a files
USERLIBS=$(wildcard libs/*.a)


RTAIDIR = $(shell rtai-config --prefix)
C_FLAGS = -O2 -g $(ORTD_INCLUDE)

RM = rm -f
FILES_TO_CLEAN = *.o ortd_scicos_exec

CC = gcc
CC_OPTIONS = -O -DNDEBUG -Dlinux -DNARROWPROTO -D_GNU_SOURCE

MODEL = ortd_scicos_exec

OBJS = ortd_scicos_exec.o common.o
SRCS = ortd_scicos_exec.cpp common.c


SCILIBS = \
	  $(SCIDIR)/libs/scicos.a \
	  $(SCIDIR)/libs/poly.a \
	  $(SCIDIR)/libs/calelm.a \
	  $(SCIDIR)/libs/blas.a \
	  $(SCIDIR)/libs/lapack.a \
	  $(SCIDIR)/libs/os_specific.a 

ULIBRARY = $(HARTDIR)/lib/libsciblk.a $(RTAIDIR)/lib/liblxrt.a

CFLAGS = $(CC_OPTIONS) -O2 -I$(SCIDIR)/routines -I$(HARTDIR)/includes -I$(HARTDIR)/includes/scicos $(C_FLAGS) $(MACROS) -DMODEL=$(MODEL) -DMODELN=$(MODEL).c

#rtmain.c: $(HARTDIR)/RT_templates/rtmain.c $(MODEL).c
	#cp $< .

ortd_scicos_exec: $(OBJS) $(ULIBRARY)
	gcc -static -o $@  $(OBJS) $(USERLIBS) $(SCILIBS) $(ULIBRARY)  $(OTHERLIBS) -lpthread -lstdc++ -lrt -lm
	@echo "### Created executable: $(MODEL) ###"


#$(OBJS): $(SRCS)
#	$(CC) -c -o $@ $< $(C_FLAGS) -Iscicos_include $(MACROS)
	
ortd_scicos_exec.o: ortd_scicos_exec.cpp
	$(CC) -c -o $@ $< $(CFLAGS) -Iscicos_include $(MACROS)

common.o: common.c
	$(CC) -c -o $@ $< $(CFLAGS) -Iscicos_include $(MACROS)

clean::
	@$(RM) $(FILES_TO_CLEAN)
