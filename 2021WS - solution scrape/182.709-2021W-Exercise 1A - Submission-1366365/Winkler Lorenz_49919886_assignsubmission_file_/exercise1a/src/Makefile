CC=gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
FLAGS = -std=c99 -pedantic -Wall -g

OBJECTS = mydiff.o

.PHONY: all clean
all: mydiff

mydiff: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(FLAGS) $(DEFS) -c -o $@ $<

mydiff.o: mydiff.c

clean:
	rm -rf *.o mydiff