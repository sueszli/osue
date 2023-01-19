FLAGS = -std=c99 -pedantic -Wall -Wextra -Wwrite-strings -Wconversion -g3 -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
libs=-lcrypt

.PHONY: all clean

all: listtool

listtool: listtool.o
	gcc -o listtool listtool.o list.o $(libs)

listtool.o: listtool.c list.h
	gcc -c -o listtool.o listtool.c $(CFLAGS) 

clean:
	rm -f listtool listtool.o
