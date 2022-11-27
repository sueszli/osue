CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g

.PHONY: all clean

all: supervisor generator

supervisor: supervisor.o sem_util.o
	$(CC) $(CFLAGS) -lpthread -lrt -o $@ $^

supervisor.o: supervisor.c
	$(CC) $(CFLAGS) -c -o $@ $^

generator: generator.o
	$(CC) $(CFLAGS) -lpthread -lrt -o $@ $^

generator.o: generator.c
	$(CC) $(CFLAGS) -c -o $@ $<

sem_util.o: sem_util.c sem_util.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm supervisor supervisor.o generator generator.o