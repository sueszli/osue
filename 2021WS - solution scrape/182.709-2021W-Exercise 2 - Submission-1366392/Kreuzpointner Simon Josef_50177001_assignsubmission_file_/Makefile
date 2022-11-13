# 
# Simon Josef Kreuzpointner 12021358
# 14 November 2021
#
# This is the makefile for cpair.c
#

CC = gcc # the c compiler
DEFS = -DERROR -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L # definitions
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS) # compiler flags
LDFLAGS = -lm # linker flags

OBJECTS = cpair.o debugUtility.o pointActions.o utility.o pipeActions.o

.PHONY: all clean

all: cpair

cpair: $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

cpair.o: cpair.c debugUtility.h pointActions.h pipeActions.h
debugUtility.o: debugUtility.c debugUtility.h
pointActions.o: pointActions.c pointActions.h debugUtility.h utility.h
utility.o: utility.c utility.h
pipeActions.o: pipeActions.c pipeActions.h forkConfig.h

clean:
	rm -rf *.o cpair
