CC = gcc
DEFS: -D_DEFAULT SOURCE -D_BSD_SOURCE - D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS: -std=c99 -pedantic -Wall -g $(DEFS)

all: supervisor generator

supervisor: supervisor.o
	$(CC) $(CFLAGS) -o supervisor supervisor.o -pthread -lrt

generator: generator.o
	$(CC) $(CFLAGS) -o generator generator.o -pthread -lrt

supervisor.o: supervisor.c header.h
	$(CC) $(CFLAGS) -c -o supervisor.o supervisor.c

generator.o: generator.c header.h
	$(CC) $(CFLAGS) -c -o generator.o generator.c


clean:
	rm -rf *.o supervisor generator