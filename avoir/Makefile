CC=g++
CFLAGS=-Wall -pie -fPIE -s
INCLUDES=../common/
SHARED=../common/sharedfuncs.c
EXECUTABLE=avoir
SOURCES=avoir.cc user.cc finode.cc base64.cc md5.cc $(SHARED)

all:
	$(CC) $(SOURCES) $(CFLAGS) -I $(INCLUDES) -o $(EXECUTABLE)

debug:
	$(CC) $(SOURCES) $(CFLAGS) -DDEBUG -I $(INCLUDES) -o $(EXECUTABLE)
