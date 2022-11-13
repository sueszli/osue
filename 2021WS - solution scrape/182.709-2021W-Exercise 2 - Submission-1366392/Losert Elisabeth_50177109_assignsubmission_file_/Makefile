#-----------------------------------
# @author 11908100 Elisabeth Losert
# @programname forksort
#-----------------------------------

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
LINKS = -lm

.PHONY: all clean
all: forksort

forksort: forksort.o
	$(CC) -o $@ forksort.o $(LINKS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

forksort.o: forksort.c

clean:
	rm -rf *.o forksort