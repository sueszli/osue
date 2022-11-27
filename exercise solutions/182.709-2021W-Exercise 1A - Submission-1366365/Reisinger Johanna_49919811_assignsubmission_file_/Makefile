CC=gcc
DEFS=-D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS=-std=c99 -pedantic -Wall -g $(DEFS)
LDFLAGS=

OBJECTS = main.o

.PHONY: all clean

all: is_palindrome

is_palindrome: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf *.o is_palindrome
