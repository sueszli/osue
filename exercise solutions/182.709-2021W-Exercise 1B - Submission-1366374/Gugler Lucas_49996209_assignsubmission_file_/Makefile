# @file Makefile
# @author Lucas Gugler 12019849
#
# @brief Makefile for supervisor and generator

CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g 

all: generator supervisor

generator: generator.o
	gcc $(CFLAGS) -o $@ $^ -lrt -lpthread

supervisor: supervisor.o
	gcc $(CFLAGS) -o $@ $^ -lrt -lpthread

%.o: %.c
	gcc $(CFLAGS) -c -o $@ $<

clean:
	rm -f generator generator.o supervisor supervisor.o

generator.o: generator.c shared_resources.h
supervisor.o: supervisor.c shared_resources.h