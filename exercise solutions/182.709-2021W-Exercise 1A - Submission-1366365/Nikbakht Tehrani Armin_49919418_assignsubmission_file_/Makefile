# Armin Nikbakht-Tehrani 11928288
# 1a mycompress.c
CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

.PHONY: all clean
all: mycompress

mycompress: mycompress.c
	$(CC) $(LDFLAGS) -o $@ $^ -lm



%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<


mycompress.o: mycompress.c

clean:
	rm -rf *.o mycompress
