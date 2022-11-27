# author: Leonie Greber (11801674)
# programm name: mygrep.c

CC = gcc
DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

TARGETS := mygrep

.PHONY: all clean
all: $(TARGETS)

mygrep : mygrep.o
	$(CC) $(CFLAGS) -o mygrep mygrep.o
	
%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf *.o mygrep