# @author Leopold Fajtak (e01525033@student.tuwien.ac.at)
# @programName myexpand

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCHE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -pedantic -Wall $(DEFS) -g

OBJECTS = main.o fileutil.o strutil.o expand.o

.PHONY: all clean
all: myexpand
expand.o: strutil.o

myexpand: $(OBJECTS) 
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf *.o myexpand
