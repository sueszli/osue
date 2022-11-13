# 
# Simon Josef Kreuzpointner 12021358
# 22 October 2021
#
# This is the makefile for mycompress.c
#


CC = gcc # the c compiler
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L # definitions
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS) # compiler flags
LDFLAGS = # linker flags

OBJECTS = mycompress.o utility.o debugUtility.o fileActions.o

.PHONY: all clean

all: mycompress

mycompress: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

mycompress.o: mycompress.c utility.h debugUtility.h fileActions.h
utility.o: utility.c utility.h debugUtility.h
debugUtility.o: debugUtility.c debugUtility.h
fileActions.o: fileActions.c fileActions.h utility.h debugUtility.h

clean:
	rm -rf *.o mycompress
