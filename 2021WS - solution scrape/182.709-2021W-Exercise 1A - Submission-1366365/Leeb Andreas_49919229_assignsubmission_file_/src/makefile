# @author Andreas Leeb
# 12019848
# mycompress
CC = gcc
DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEGS)

.PHONY: all clean
all: mycompress

mycompress: mycompress.c mycompress.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ mycompress.c

clean:
	rm -rf *.o mycompress