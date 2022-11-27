# Author: Yuhang Jiang
# Program name: mygrep

CC = gcc
DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -D_DEFAULT_SOURCE
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

PROG_NAME = mygrep

OBJECTS = main.o compare.o

.PHONY: all clean
all: $(PROG_NAME)

$(PROG_NAME): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

main.o: main.c compare.h

clean:
	rm -rf *.o $(PROG_NAME)