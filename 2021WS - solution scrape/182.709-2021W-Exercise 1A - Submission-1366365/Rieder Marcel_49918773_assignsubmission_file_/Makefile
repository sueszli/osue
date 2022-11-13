# @Author Marcel Rieder <e11742405@student.tuwien.ac.at>
# @Programname mygrep
# @Date 27.10.2021

CC=gcc
DEFS=-D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS=-Wall -g -std=c99 -pedantic $(DEFS)

.PHONY: all clean
all: mygrep

mygrep: mygrep.o
	$(CC) -o $@ $<

mygrep.o: mygrep.c
	$(CC) $(CFLAGS) -c -o $@ $<


clean:
	rm -rf *.o mygrep
