.PHONY: all clean

defs = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
default_mode = -std=c99 -pedantic -g -Wall  $(defs) 
verbose_mode = -std=c99 -pedantic -g3 -Wall -Wextra -Wwrite-strings -Wconversion $(defs)
libs = -lpthread -pthread -lrt

all: supervisor generator

supervisor: supervisor.o
	gcc -o supervisor supervisor.o $(libs)

generator: generator.o
	gcc -o generator generator.o $(libs)

supervisor.o: supervisor.c common.h
	gcc $(verbose_mode) -c -o supervisor.o supervisor.c

generator.o: generator.c common.h
	gcc $(verbose_mode) -c -o generator.o generator.c

clean:
	rm -rf *.o supervisor generator