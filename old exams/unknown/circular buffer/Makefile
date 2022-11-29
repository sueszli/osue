##
# Makefile for server and client.
#

# config

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809 -D_XOPEN_SOURCE
override CFLAGS += -Wall -g -std=c99 -pedantic $(DEFS)
override LDFLAGS +=
override LIBS += -pthread -lrt

# objects to build
OBJS = client.o server.o

# rules

.PHONY : all clean

all: client server

client: $(OBJS) $(LDOBJS)
	$(CC) $(LDFLAGS) -o $@ client.o $(LIBS)

server: $(OBJS) $(LDOBJS)
	$(CC) $(LDFLAGS) -o $@ server.o $(LIBS)


%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

client.o: client.c
server.o: server.c

clean:
	rm -rf $(OBJS) client server
