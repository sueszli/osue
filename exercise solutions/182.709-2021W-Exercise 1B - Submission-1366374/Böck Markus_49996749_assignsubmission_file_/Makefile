# generator & supervisor
# Author: Markus Böck 12020632

CC=gcc
CFLAGS=-std=c99 -pedantic -Wall -DDEFAULT_SOURCE -DBSD_SOURCE -DSVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -pthread
LDFLAGS=-lrt

HEADERS = API.h CircularBuffer.h

.PHONY: all
all:    generator supervisor

generator:   generator.o libUtil.a
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS) 
	
supervisor:   supervisor.o libUtil.a
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS) 
	
%.o:	%.c	 $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@
	
libUtil.a: CircularBuffer.o
	ar rcs libUtil.a $^
	
.PHONY: clean
clean: 
	rm -f *.o *.a generator supervisor
