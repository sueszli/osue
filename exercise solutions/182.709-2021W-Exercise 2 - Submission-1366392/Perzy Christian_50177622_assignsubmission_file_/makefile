# forksort
# Christian Perzy [11809921]

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_SVID_SOURCE -D_BSD_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

OBJECTS = forksort.o

.PHONY: all clean
all: forksort

forksort: $(OBJECTS)
	$(CC) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

forksort.o: forksort.c forksort.h

clean:
	rm -rf *.o forksort