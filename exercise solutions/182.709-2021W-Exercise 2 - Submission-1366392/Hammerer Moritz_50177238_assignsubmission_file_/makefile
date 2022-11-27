CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

.PHONY: all clean
all: forksort

forksort: forksort.o
	$(CC)  -o $@ $^ -lm

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf *.o forksort