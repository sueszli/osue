##
# Makefile for exercise 2.
##

# config

DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809
override CFLAGS += -Wall -g -std=c99 -pedantic $(DEFS) \
                   -Wno-unused-variable -Wno-unused-function
override LDFLAGS +=
override LIBS +=

# objects to link (already built)
LDOBJS.server = server_lib.o devices.o utils.o
LDOBJS.client = client_lib.o utils.o

# rules

.PHONY : all clean

all: server client

server: server.o
	gcc $(LDFLAGS) -o $@ $^ $(LDOBJS.server) $(LIBS)

client: client.o
	gcc $(LDFLAGS) -o $@ $^ $(LDOBJS.client) $(LIBS)

%.o: %.c
	gcc $(CFLAGS) -c -o $@ $<

clean:
	rm -f server server.o client client.o

# dependencies

server.o: server.c server.h common.h
client.o: client.c client.h common.h
