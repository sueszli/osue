# @file Makfile
# @author Lena Jankoschek - 12019852
# @date 2021-11-12
# @brief Makefile for 3coloring (programs supervisor and generator)
#
# copyright Copyright (c) 2021

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
#-lpthread = for linking semaphores, -lrt = for linking shared mem
LDFLAGS = -lpthread -lrt

OBJECTS_SUPERVISOR = supervisor.o  circularbuffer.o 
OBJECTS_GENERATOR = generator.o circularbuffer.o

.PHONY = all clean release
all: supervisor generator

supervisor: $(OBJECTS_SUPERVISOR)
		$(CC) $(LDFLAGS) -o $@ $^
generator: $(OBJECTS_GENERATOR)
		$(CC) $(LDFLAGS) -o $@ $^
%.o: %.c
		$(CC) $(CFLAGS) -c -o $@ $<


clean:
		rm -rf *.o supervisor generator UE1B.tgz

release:
		tar -cvzf UE1B.tgz *.c *.h Makefile
