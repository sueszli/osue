# author: Moritz Bauer, 0649647
# creates: ispalindrom
# date: 21.11.11

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

EXEC_NAME = ispalindrom
OBJECTS = main.o

.PHONY: all clean

runAll: clean all

all: $(EXEC_NAME)

clean:
	rm -rf *.o $(EXEC_NAME)


$(EXEC_NAME): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

archive: delete_archive
	tar -cvzf 00649647.1a.tgz Makefile *.c

delete_archive:
	rm -rf *.tgz