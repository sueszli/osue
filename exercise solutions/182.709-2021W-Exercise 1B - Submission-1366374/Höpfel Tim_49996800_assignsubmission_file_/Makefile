CC = gcc #C compiler
DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
LDFLAGS  = -pthread -lrt
OBJECTS = generator.o supervisor.o

.PHONY: all clean
#PHONY executed without checking for existing files

all: supervisor generator 

supervisor: supervisor.o
	$(CC) -o $@ $^ $(LDFLAGS)

generator: generator.o
	$(CC) -o $@ $^ $(LDFLAGS)		#linking

%.o:  %.c
	$(CC) $(CFLAGS) -c -o $@ $< $(LDFLAGS)		#compiling

supervisor.o: supervisor.c #compresshelper.h
generator.o: generator.c #generator.h

clean:
	rm -rf *.o generator supervisor
#use clean before uploading!
