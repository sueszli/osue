# author: Márton Károlyi
# program name: myexpand

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -pedantic -Wall  $(DEFS) -g

OBJECTS = myexpand.o

.PHONY: all clean
all: myexpand

myexpand: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

myexpand.o: myexpand.c


clean:
	rm -rf *.o myexpand