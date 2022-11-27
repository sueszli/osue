#author Andreas Huber 11809629
#program mycompress

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -pedantic -Wall $(DEFS)

.PHONY: all clean
all: mycompress

mycompress: mycompress.o 
	$(CC) $(CFLAGS) -o mycompress mycompress.o

mycompress.o: mycompress.c
	$(CC) $(CFLAGS) -g -c -o mycompress.o mycompress.c

clean:
	rm -f *.o mycompress
	

