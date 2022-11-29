##
# Makefile for client.
#

# config

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809 -D_XOPEN_SOURCE
override CFLAGS += -Wall -g -std=c99 -pedantic $(DEFS)
override LDFLAGS +=
override LIBS += -pthread -lrt

# objects to build
OBJS = client.o

# rules

.PHONY : all clean

all: client

client: $(OBJS) $(LDOBJS)
	$(CC) $(LDFLAGS) -o $@ client.o $(LIBS)


%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

client.o: client.c

clean:
	rm -rf $(OBJS) client
