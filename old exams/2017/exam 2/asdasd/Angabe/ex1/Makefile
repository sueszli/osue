##
# Makefile for exercise 1.
##

# config

DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809
override CFLAGS += -Wall -g -std=c99 -pedantic $(DEFS)
override LDFLAGS +=
override LIBS += -pthread -lrt

# objects to build
OBJS = validate.o

# objects to link (already built)
LDOBJS = validate_lib.o

# rules

.PHONY : all clean

all: validate

validate: $(OBJS) $(LDOBJS)
	gcc $(LDFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	gcc $(CFLAGS) -c -o $@ $<

clean:
	rm -f validate $(OBJS)

# dependencies

validate.o: validate.c validate.h
