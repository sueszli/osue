#author:		Alexander Gube
#program:	mycompress
#date:		22.10.2021

DFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFS = -std=c99 -pedantic -Wall $(DFS)

.PHONY: all clean
all: mycompress

mycompress: mycompress.o
	gcc -o mycompress mycompress.o

mycompress.o: mycompress.c
	gcc $(CFS) -g -c -o mycompress.o mycompress.c

clean:
	rm -rf *.o mycompress


