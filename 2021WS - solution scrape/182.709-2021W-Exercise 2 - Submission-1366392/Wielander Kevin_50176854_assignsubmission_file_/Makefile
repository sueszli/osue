information:
	@echo "Author: Kevin Wielander, 11908531"
	@echo "Program name: forksort"

CC = gcc
DEVS = -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L 
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

OBJECTS = forksort.o

.PHONY: all clean
all: forksort
clean:
	rm -rf *.o forksort

forksort: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^


%.o: %.c	
	$(CC) $(CFLAGS) -c -o $@ $<

forksort.o: forksort.c
