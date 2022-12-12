##
# Makefile for exercise 1.
##

# config

DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809
override CFLAGS += -Wall -g -std=c99 -pedantic $(DEFS)
override LDFLAGS +=
override LIBS +=

# target and its object files to build
TARGET = client
OBJS = client.o

# objects to link (already built)
LDOBJS = client_lib.o

# rules

.PHONY : all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	gcc $(LDFLAGS) -o $@ $^ $(LDOBJS) $(LIBS)

%.o: %.c
	gcc $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET) $(OBJS)

# dependencies

client.o: client.c client.h
