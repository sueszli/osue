# Author: Valentin Futterer, 11904654
# Program names: supervisor, generator

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -pedantic -Wall -g $(DEFS)

.PHONY: all clean
all: supervisor generator

#lrt and pthread needed to link libraries for shm and semaphores
supervisor: supervisor.o 3-coloringUtil.o
	$(CC) $(LDFLAGS) -o $@ $^ -lrt -pthread

generator: generator.o 3-coloringUtil.o 
	$(CC) $(LDFLAGS) -o $@ $^ -lrt -pthread

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

supervisor.o: supervisor.c 3-coloringUtil.h
generator.o: generator.c 3-coloringUtil.h
3-coloringUtil.o: 3-coloringUtil.c 3-coloringUtil.h

clean:
	rm -rf *.o supervisor generator 3-coloringUtil