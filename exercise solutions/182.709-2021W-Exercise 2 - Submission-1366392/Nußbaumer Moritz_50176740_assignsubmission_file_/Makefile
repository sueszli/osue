# @author Nußbaumer Moritz
# @matriculation number 12026652
# @program name forksort

CC = gcc
DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
LDFLAGS = -lm
OBJECTS = forksort.o

.PHONY: all clean
all: forksort

forksort2: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<


clean:
	rm -rf *.o forksort