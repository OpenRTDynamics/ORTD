CC = cc
CPP = c++
LD = ld

#CFLAGS = -I../.. -O2 

# detect system type
host-type := $(shell arch)

ifeq ($(host-type),x86_64)
# 64 Bit
CFLAGS = -fPIC -O2 -I../.. -Iinclude
LDFLAGS = -shared
else
# 32 Bit
CFLAGS = -O2 -I../.. -Iinclude
LDFLAGS = -shared 
endif


objs=muParserBase.o muParserBytecode.o muParserCallback.o muParser.o muParserError.o muParserTokenReader.o

#all: lib
#	echo " muparser done"

lib: $(objs)
	ar rvs libmuparser.a $(objs)   # Will always be included
	#ar rvs module_Linux.a special_linux_code.o   # Only include for "Linux" Target


clean:
	rm -f *.o libmuparser.a



muParserBase.o: muParserBase.cpp
	$(CC) $(CFLAGS) -c muParserBase.cpp

muParserBytecode.o: muParserBytecode.cpp
	$(CC) $(CFLAGS) -c muParserBytecode.cpp

muParserCallback.o: muParserCallback.cpp
	$(CC) $(CFLAGS) -c muParserCallback.cpp

muParser.o: muParser.cpp
	$(CC) $(CFLAGS) -c muParser.cpp

muParserError.o: muParserError.cpp
	$(CC) $(CFLAGS) -c muParserError.cpp

muParserTokenReader.o: muParserTokenReader.cpp
	$(CC) $(CFLAGS) -c muParserTokenReader.cpp


