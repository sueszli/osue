CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_VID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

OBJECTS = ispalindrom.o

.PHONY: all clean docs

all: ispalindrom

ispalindrom: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o:
	$(CC) $(CFLAGS) -c -o $@ $<

ispalindrom.o: ispalindrom.c ispalindrom.h

docs:  html/index.html

html/index.html: ispalindrom.c ispalindrom.h
	doxygen Doxyfile

clean:
	rm -rf *.o ispalindrom html latex
