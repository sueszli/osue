# @author Nußbaumer Moritz
# @matriculation number 12026652
# @program name myexpand

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L 
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

.PHONY: all clean
all: myexpand

myexpand: myexpand.o
	$(CC) -o $@ $^

myexpand.o : myexpand.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf *.o myexpand