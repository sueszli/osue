#author: Manuel Hochreiter e0671428@student.tuwien.ac.at
#program_name: cpair
CC = gcc
DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
LDFLAGS = -pthread

OBJECT = cpair.o 

.PHONY: all clean
all: cpair

cpair: $(OBJECT)
	$(CC) $(LDFLAGS) -o $@ $^ -lm

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean: 
	rm -rf *.o cpair