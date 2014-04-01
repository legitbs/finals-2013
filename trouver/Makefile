CC=gcc
CFLAGS=-Wall -pie -fPIE -pthread -marm -fno-stack-protector -s
INCLUDES=../common/
SHARED=../common/sharedfuncs.c
EXECUTABLE=trouver
SOURCES=trouver.c chroot.c $(SHARED)

all:
	$(CC) $(SOURCES) $(CFLAGS) -I $(INCLUDES) -o $(EXECUTABLE) -D_GNU_SOURCE

debug:
	$(CC) $(SOURCES) $(CFLAGS) -DDEBUG -I $(INCLUDES) -o $(EXECUTABLE)
