# @file Makefile
# @author Munir Yousif Elagabani <e12022518@student.tuwien.ac.at>
# @date 14.11.2021
# @brief build with make
# program name: supervisor generator

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
LDFLAGS = -lrt -pthread

SUPOBJECTS = supervisor.o circularbuffer.o
GENOBJECTS = generator.o circularbuffer.o

.PHONY: all clean
all: supervisor generator 

supervisor: $(SUPOBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

generator: $(GENOBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^


%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

supervisor.o: supervisor.c
generator.o: generator.c
circularbuffer.o: circularbuffer.c

clean:
	rm -rf *.o generator supervisor