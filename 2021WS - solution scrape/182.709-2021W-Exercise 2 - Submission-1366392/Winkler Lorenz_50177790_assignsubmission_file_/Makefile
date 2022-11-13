CC=gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
FLAGS = -std=c99 -pedantic -Wall -g

OBJECTS = forksort.o

.PHONY: all clean
all: forksort

forksort: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(FLAGS) $(DEFS) -c -o $@ $<

forksort.o: forksort.c

clean:
	rm -rf *.o forksort