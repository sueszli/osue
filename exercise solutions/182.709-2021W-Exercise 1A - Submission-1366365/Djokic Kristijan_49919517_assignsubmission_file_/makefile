CC = gcc 
DEFS = -D_DEFAULT-SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L 
CFLAGS = -std=c99 -pedantic -Wall -g $(DEFS)

OBJECTS = mydiff.o

.PHONY: all clean
all: mydiff

mydiff: $(OBJECTS)
	$(CC) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

mydiff.o: mydiff.c

clean:
	rm -rf *.o mydiff