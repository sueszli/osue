## @author: David Brand <e11905164@student.tuwien.ac.at>
## @program: mygrep

CC		= gcc
DEFS	= -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS	= -std=c99 -pedantic -Wall -g $(DEFS)

./PHONY: all clean
all: mygrep

mygrep: mygrep.o
	gcc -o mygrep mygrep.o

mygrep.o: mygrep.c
	gcc $(CFLAGS) -c mygrep.c

clean:
	rm -rf *.o mygrep