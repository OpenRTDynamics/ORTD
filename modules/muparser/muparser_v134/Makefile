# CC = cc
# CPP = c++
# LD = ld

# detect system type
# host-type := $(shell arch)
# 
# ifeq ($(host-type),x86_64)
# # 64 Bit
# CFLAGS = -fPIC -O2 -I../.. -Iinclude
# LDFLAGS = -shared
# else
# # 32 Bit
# CFLAGS = -O2 -I../.. -Iinclude
# LDFLAGS = -shared 
# endif

CFLAGS_N = $(CFLAGS) -Iinclude

objs=muParserBase.o muParserBytecode.o muParserCallback.o muParser.o muParserError.o muParserTokenReader.o

#all: lib
#	echo " muparser done"

lib: $(objs)
	ar rvs libmuparser.a $(objs)   # Will always be included
	#ar rvs module_Linux.a special_linux_code.o   # Only include for "Linux" Target


clean:
	rm -f *.o libmuparser.a



muParserBase.o: muParserBase.cpp
	$(CC) $(CFLAGS_N) -c muParserBase.cpp

muParserBytecode.o: muParserBytecode.cpp
	$(CC) $(CFLAGS_N) -c muParserBytecode.cpp

muParserCallback.o: muParserCallback.cpp
	$(CC) $(CFLAGS_N) -c muParserCallback.cpp

muParser.o: muParser.cpp
	$(CC) $(CFLAGS_N) -c muParser.cpp

muParserError.o: muParserError.cpp
	$(CC) $(CFLAGS_N) -c muParserError.cpp

muParserTokenReader.o: muParserTokenReader.cpp
	$(CC) $(CFLAGS_N) -c muParserTokenReader.cpp


