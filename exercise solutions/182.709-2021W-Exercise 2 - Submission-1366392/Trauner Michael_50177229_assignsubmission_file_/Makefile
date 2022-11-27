 # @file make
 # @author Michael Trauner <e12019868@student.tuwien.ac.at>
 # @date 09.12.2021

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L 
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
LDFLAGS = -pthread -lrt

OBJECTS_MAIN= cpair.o

.PHONY: all clean
all: cpair

cpair: $(OBJECTS_MAIN)
	$(CC) $(LDFLAGS) -o $@ $^ -lm

cpair.o: cpair.c
	$(CC) $(CFLAGS) -c -o $@ $^ 

clean:
	rm -rf *.o cpair