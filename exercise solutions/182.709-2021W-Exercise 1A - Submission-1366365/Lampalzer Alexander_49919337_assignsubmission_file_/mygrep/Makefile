# mygrep - print lines that match patterns
# Author: Alexander Lampalzer <e12023145@student.tuwien.ac.at>

all:
	gcc -O3 -std=c99 -Wall -pedantic -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -o mygrep -g grep.c main.c

clean:
	rm -f *.o
	rm -f mygrep
	rm -rf doc/html
	rm -rf doc/man
	rm -rf doc/latex
