information:
	@echo "Author: Kevin Wielander, 11908531"
	@echo "Program name: mydiff"

CC = gcc
DEVS = -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L 
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

OBJECTS = mydiff.o

.PHONY: all clean
all: mydiff
clean:
	rm -rf *.o mydiff

mydiff: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^


%.o: %.c	
	$(CC) $(CFLAGS) -c -o $@ $<

mydiff.o: mydiff.c
