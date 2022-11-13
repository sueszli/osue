# @file Makefile
# @author Philipp Holzer <e12028208@student.tuwien.ac.at>
# @date 14.11.2021

CC = gcc
DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809LL

CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
LDFLAGS = -lrt -pthread

SOBJECTS = supervisor.o
GOBJECTS = generator.o

.PHONY: all clean tar
all: supervisor generator

supervisor: $(SOBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^ 

generator: $(GOBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# generates tgz file with all .c and .h files
tar:
	tar -cvzf Exercise1B.tgz Makefile *.c *.h

clean:
	rm -rf *.o supervisor generator Exercise1B.tgz
