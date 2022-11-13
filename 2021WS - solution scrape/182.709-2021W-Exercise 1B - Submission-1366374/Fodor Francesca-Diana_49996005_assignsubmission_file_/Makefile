#@author Fodor Francesca Diana, 11808223
#@program-name 3-coloring
#@date 08.11.2021
#@file makefile

CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g

LDFLAGS = -lrt -pthread

.PHONY: clean

all: supervisor generator

supervisor: supervisor.o sharedfunctions.o #+header-objects
	gcc $(LDFLAGS) -o $@ $^

supervisor.o: supervisor.c sharedfunctions.h #+headers
	gcc $(CFLAGS) -c -o $@ $<

generator: generator.o sharedfunctions.o #...
	gcc $(LDFLAGS) -o $@ $^

generator.o: generator.c sharedfunctions.h #...
	gcc $(CFLAGS) -c -o $@ $<

#header

sharedfunctions.o: sharedfunctions.c sharedfunctions.h
	gcc $(CFLAGS) -c -o $@ $<

clean:
	rm -rf *.o supervisor generator
