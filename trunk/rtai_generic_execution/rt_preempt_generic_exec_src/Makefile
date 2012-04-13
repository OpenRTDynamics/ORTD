


# Path to Scilab
SCIDIR = /opt/scilab-4.1.2
#SCIDIR = /home/control/scilab/scilab-4.1.2

# Path to hart
HARTDIR= /home/control/svn/hart/macros/RtaiCodeGen/../..
#HARTDIR= /home/control/scilab/hart_svn/macros/RtaiCodeGen/../..

# Libraries to fetch from the system
OTHERLIBS =  -lortd  -lgsl -lblas -lgsl -lblas -lgfortran -lrt -lpthread 

# MACROS: include -DORTD_REGISTER_SCICOSBLOCKS if you want some Scicos-Blocks to be registeres to ORTD,
#         so you can nest them into ORTD schematics
#         Enabling this will call C-function named ORTD_register_scicosblocks() that has to be included within
#         an archive e.g. within the libs/ subdirectory.
MACROS = -DORTD_REGISTER_SCICOSBLOCKS

###################################################


all: ../rtai_generic_exec

# Fetch the static libs from the libs/ subdir, where the user can place its *.a files
USERLIBS=$(wildcard libs/*.a)

RM = rm -f
FILES_TO_CLEAN = *.o ../rtai_generic_exec

CC = gcc
CC_OPTIONS = -O -DNDEBUG -Dlinux -DNARROWPROTO -D_GNU_SOURCE

MODEL = rtai_generic_exec
OBJSSTAN = rt_preempt_main.o common.o rtai_generic_exec.o rtai_generic_exec_Cblocks.o

SCILIBS = \
	  $(SCIDIR)/libs/scicos.a \
	  $(SCIDIR)/libs/poly.a \
	  $(SCIDIR)/libs/calelm.a \
	  $(SCIDIR)/libs/blas.a \
	  $(SCIDIR)/libs/lapack.a \
	  $(SCIDIR)/libs/os_specific.a 
OTHERLIBS =  -lpthread -lpthread -lortd  -lpthread -lortd  -lpthread -ldl -L/usr/local/lib/  -lpthread -lgsl -lblas -lrt -lpthread 
ULIBRARY = $(HARTDIR)/lib/libsciblk_preempt.a

#CFLAGS = $(CC_OPTIONS) -O2 -I$(SCIDIR)/routines -I$(HARTDIR)/includes -I$(HARTDIR)/includes/scicos $(C_FLAGS) -DMODEL=$(MODEL) -DMODELN=$(MODEL).c

CFLAGS = $(CC_OPTIONS) -O2 -I$(SCIDIR)/routines -I$(HARTDIR)/includes -I$(HARTDIR)/includes/scicos $(C_FLAGS) $(MACROS) -DMODEL=$(MODEL) -DMODELN=$(MODEL).c

rt_preempt_main.c: $(HARTDIR)/RT_templates/rt_preempt_main.c $(MODEL).c
	cp $< .

../rtai_generic_exec: $(OBJSSTAN) $(ULIBRARY)
	gcc -o $@  $(OBJSSTAN)  $(USERLIBS) $(SCILIBS) $(ULIBRARY) $(OTHERLIBS) -lpthread -lstdc++ -lrt -lm  -Wl,-R,'$$ORIGIN/:$$ORIGIN/lib'
	@echo "### Created executable: $(MODEL) ###"

clean::
	@$(RM) $(FILES_TO_CLEAN)
