# Author: Yuhang Jiang
# Program name: forkFFT

CC = gcc
DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -D_DEFAULT_SOURCE
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
LDFLAGS = -lm

PROG_NAME = forkFFT

OBJECTS = main.o

.PHONY: all clean
all: $(PROG_NAME)

$(PROG_NAME): $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf *.o $(PROG_NAME)