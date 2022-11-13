#author Johanna Reisinger
#program name intmul

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -pedantic -Wall -g $(DEFS)

OBJECTS = main.o

.PHONY: all clean
all: intmul

intmul: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^ -lm

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

main.o: main.c 

clean:
	rm -rf *.o intmul