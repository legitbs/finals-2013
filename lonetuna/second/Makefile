CC=gcc
CFLAGS=-Wall -pie -fPIE -pthread -fno-stack-protector -s
INCLUDES=../../common/
SHARED=../../common/sharedfuncs.c
EXECUTABLE=lonetuna-inetd
SOURCES=lonetuna-inetd.c $(SHARED)

all: 
	$(CC) $(SOURCES) $(CFLAGS) -I $(INCLUDES) -o $(EXECUTABLE)

debug:
	$(CC) $(SOURCES) $(CFLAGS) -DDEBUG -I $(INCLUDES) -o $(EXECUTABLE)
