CC = gcc
CFLAGS = -std=c99 -pedantic
DEFS = -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c
FULLCOMPILE = $(CC) $(CFLAGS) $(DEFS)

.PHONY: all clean

all: cpair

cpair: cpair.o
	$(CC) -o $@ $< -lm

cpair.o: cpair.c
	$(FULLCOMPILE) $<

clean:
	rm -rf *.o cpair

scp ./cpair.c inflab:~/Documents/UE/2
scp ./Makefile inflab:~/Documents/UE/2
