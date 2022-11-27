# @file Makefile
# @author Alexander Gschnitzer (01652750) <e1652750@student.tuwien.ac.at>
# @date 19.10.2021
#
# @brief Makefile for 3color program. Generates supervisor and generator executable.

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -pedantic -Wall -g  $(DEFS)
LDFLAGS = -lpthread -lrt

TARGETS = supervisor generator
SOURCE = generator.c supervisor.c util.c
HEADERS = util.h
G_OBJECTS = generator.o util.o
S_OBJECTS = supervisor.o util.o
COMPRESS = 3color.tgz

.PHONY: all clean build
all: supervisor generator

supervisor: $(S_OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

generator: $(G_OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

supervisor.o: supervisor.c util.h
generator.o: generator.c util.h
util.o: util.c util.h

clean:
	rm -rf *.o $(TARGETS) $(COMPRESS) $(G_OBJECTS) $(S_OBJECTS)

build:
	tar -cvzf $(COMPRESS) $(HEADERS) $(SOURCE) Makefile

