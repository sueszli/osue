# @author: Michael Blank 11909459 <e11909459@student.tuwien.ac.at>
# @programs: supervisor generator

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -pedantic -Wall $(DEFS) -g

.PHONY: all clean
all: supervisor generator

supervisor: supervisor.o
	$(CC)  -o $@ $^ -lrt -lpthread

generator: generator.o
	$(CC)  -o $@ $^ -lrt -lpthread

%.o: %.c
	$(CC) $(CFLAGS) -c $< 

clean:
	rm -rf *.o supervisor generator