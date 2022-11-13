# @file Makefile
# @author Nursultan Imanov 01528474
# @date 01.11.2021
# @progname mygrep

CC=gcc

compile_flags = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SCID_SOURCE -D_POSIX_C_SOURCE=200809L -g

.PHONY: all clean
all: mygrep

%.o: %.c
	$(CC) $(compile_flags) -c -o $@ $<

mygrep.o: mygrep.c

clean:
	rm -rf *.o mygrep html latex