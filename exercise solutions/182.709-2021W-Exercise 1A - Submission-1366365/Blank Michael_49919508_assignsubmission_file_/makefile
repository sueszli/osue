# @author: Michael Blank 11909459 <e11909459@student.tuwien.ac.at>
# @programs: mycompress

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -pedantic -Wall $(DEFS) -g

OBJECTS = mycompress.o

.PHONY: all clean
all: mycompress

mycompress: $(OBJECTS)
	$(CC) -o $@ $^ -lm

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf *.o mycompress