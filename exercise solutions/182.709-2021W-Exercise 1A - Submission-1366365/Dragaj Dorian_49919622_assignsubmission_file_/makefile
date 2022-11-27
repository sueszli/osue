CC = gcc 
DEFS = -D_BSD_SOURCE -D_DEFAULT_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

OBJECTS = mydiff.o linecmp.o

.PHONY: all clean
all: mydiff

mydiff: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

mydiff.o: mydiff.c linecmp.h
linecmp.o: linecmp.c linecmp.h

tar:
	tar -cvzf archive-Task1A.tgz makefile *.c *.h

clean:
	rm -rf *.o mydiff

