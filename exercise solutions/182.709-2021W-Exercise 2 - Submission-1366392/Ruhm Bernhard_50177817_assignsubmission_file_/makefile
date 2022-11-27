CC = gcc
FLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g

OBJECTS = forksort.o

.PHONY: all clean
all: forksort

forksort: $(OBJECTS)
	$(CC) $(FLAGS) -o $@ $^

forksort.o: forksort.c
	$(CC) $(FLAGS) -c -o $@ $<

clean:
	rm -rf *.o forksort
