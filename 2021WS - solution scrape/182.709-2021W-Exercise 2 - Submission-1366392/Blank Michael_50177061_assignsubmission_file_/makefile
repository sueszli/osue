# @author: Michael Blank 11909459 <e11909459@student.tuwien.ac.at>
# @programs: cpair

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -pedantic -Wall $(DEFS) -g

.PHONY: all clean
all: cpair

cpair: cpair.o
	$(CC) -o $@ $^ -lm

%.o: %.c
	$(CC) $(CFLAGS) -c $< 

clean:
	rm -rf *.o cpair