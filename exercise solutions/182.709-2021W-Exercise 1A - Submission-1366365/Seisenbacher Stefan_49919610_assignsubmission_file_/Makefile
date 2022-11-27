CC = gcc
DEFS = -D_BSD_SORUCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L 
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

.PHONY: all clean
all: mycompress

mycompress: main.o compressor.o filehandler.o
	$(CC) $(DEFS) $(LDFLAGS) -o $@ $^
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

main.o: main.c

compressor.o: compressor.c compressor.h

filehandler.o: filehandler.c filehandler.h

clean:
	rm -rf *.o mycompress