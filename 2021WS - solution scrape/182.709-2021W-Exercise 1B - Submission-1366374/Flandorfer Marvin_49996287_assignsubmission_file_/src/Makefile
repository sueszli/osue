#Author: Marvin Flandorfer, 52004069
#Program: supervisor, generator

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -Wall -pedantic $(DEFS) -g
LDLIBS = -lrt -pthread

OBJECTS_SUPERVISOR = supervisor.o shmsupvis.o list.o misc.o
OBJECTS_GENERATOR = generator.o algorithm.o shmgen.o list.o misc.o

.PHONY: all clean
all: supervisor generator

supervisor: $(OBJECTS_SUPERVISOR)
		$(CC) -o $@ $^ $(LDLIBS)
generator: $(OBJECTS_GENERATOR)
		$(CC) -o $@ $^ $(LDLIBS)
%.o: %.c
		$(CC) $(CFLAGS) -c -o $@ $<

supervisor.o: supervisor.c shmsupvis.h list.h semaphores.h misc.h
generator.o: generator.c shmgen.h algorithm.h list.h semaphores.h misc.h
shmsupvis.o: shmsupvis.c shmsupvis.h misc.h
shmgen.o: shmgen.c shmgen.h misc.h
algorithm.o: algorithm.c list.h misc.h
list.o: list.c list.h misc.h
misc.o: misc.c misc.h

clean:
		rm -rf *.o supervisor generator