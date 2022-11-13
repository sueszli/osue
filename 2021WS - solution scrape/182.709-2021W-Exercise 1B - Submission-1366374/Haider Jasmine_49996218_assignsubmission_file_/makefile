#author: Jasmine Haider 11943664
#supervisor and generator processes for exercise 1b - 3coloring

DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D_DEFAULT_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -c -std=c99 -pedantic $(DEFS)

OBJECTS = supervisor.o generator.o

.PHONY: all clean
all: supervisor generator

supervisor: supervisor.o
	gcc $(LDFLAGS) -lrt -pthread -o $@ $^

generator: generator.o
	gcc $(LDFLAGS) -lrt -pthread -o $@ $^

%.o: %.c 
	gcc $(CFLAGS) -c -o $@ $<

supervisor.o: supervisor.c supervisor.h
generator.o: generator.c supervisor.h generator.h

clean:
	rm -rf *.o supervisor generator
