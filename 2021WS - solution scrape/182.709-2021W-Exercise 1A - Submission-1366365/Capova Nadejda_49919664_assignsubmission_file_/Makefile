# @file mydiff
# @author Nadejda Capova (11923550)
# @date 26.10.2021
CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -pedantic -Wall -g $(DEFS)

.PHONY: all clean
all: mydiff

mydiff: mydiff.o
	$(CC) -o $@ $^

mydiff.o: mydiff.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf *.o mydiff
