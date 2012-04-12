# Makefile generate from template rt_preempt.mak
# ========================================

all: ../rtai_generic_exec

SCIDIR = /home/control/scilab/scilab-4.1.2
HARTDIR= /home/control/scilab/hart_svn/macros/RtaiCodeGen/../..
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
OTHERLIBS =  -lpthread -lpthread -lortd  -lpthread -lortd  -lpthread -lQtCore -lPhysioSense -lqextserialport -ldl -L/usr/local/lib/  -lpthread -lgsl -lblas -lrt -lpthread 
ULIBRARY = $(HARTDIR)/lib/libsciblk_preempt.a

CFLAGS = $(CC_OPTIONS) -O2 -I$(SCIDIR)/routines -I$(HARTDIR)/includes -I$(HARTDIR)/includes/scicos $(C_FLAGS) -DMODEL=$(MODEL) -DMODELN=$(MODEL).c

rt_preempt_main.c: $(HARTDIR)/RT_templates/rt_preempt_main.c $(MODEL).c
	cp $< .

../rtai_generic_exec: $(OBJSSTAN) $(ULIBRARY)
	gcc -o $@  $(OBJSSTAN) $(SCILIBS) $(ULIBRARY) $(OTHERLIBS) -lpthread -lstdc++ -lrt -lm  -Wl,-R,'$$ORIGIN/:$$ORIGIN/lib'
	@echo "### Created executable: $(MODEL) ###"

clean::
	@$(RM) $(FILES_TO_CLEAN)
