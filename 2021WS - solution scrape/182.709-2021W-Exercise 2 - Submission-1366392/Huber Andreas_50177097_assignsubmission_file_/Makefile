#author Andreas Huber 11809629
#program cpair

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -pedantic -Wall -lm $(DEFS)

.PHONY: all clean
all: cpair

cpair: cpair.o 
	$(CC) $(CFLAGS) -o cpair cpair.o

cpair.o: cpair.c
	$(CC) $(CFLAGS) -g -c -o cpair.o cpair.c

clean:
	rm -f *.o cpair
	

