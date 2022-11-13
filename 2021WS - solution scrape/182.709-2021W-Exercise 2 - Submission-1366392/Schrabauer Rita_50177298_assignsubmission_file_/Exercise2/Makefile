CC = gcc
DEFS = -D_BDS_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

.PHONY: all clean
all: forksort

forksort: forksort.o
	$(CC) $(LDFLAGS) -o $@ $^

forksort.o: forksort.c
	$(CC) -c -o $@ $<

clean:
	rm -rf *.o forksort