CC = gcc
DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
OBJECTS = main.o

.PHONY: all clean # these are not files but building and cleaning commands

# run main binary
all: main

hello: $(OBJECTS)
	$(CC) -o $@ $^

# create object files for all .c files
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# define dependencies
main.o: main.c hello.h
hello.o: hello.c hello.h

# remove all object files and remove ./main binary
clean:
	rm -rf *.o main