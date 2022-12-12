##
# Makefile for exercise 2.
##

# config

DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809
override CFLAGS += -Wall -g -std=c99 -pedantic $(DEFS)
override LDFLAGS +=
override LIBS += -pthread -lrt

# objects to link (already built)
LDOBJS.server = server_lib.o

# rules

.PHONY : all clean

all: server

server: server.o
	gcc $(LDFLAGS) -o $@ $^ $(LDOBJS.server) $(LIBS)

%.o: %.c
	gcc $(CFLAGS) -c -o $@ $<

clean:
	rm -f server server.o

# dependencies

server.o: server.c server.h common.h
