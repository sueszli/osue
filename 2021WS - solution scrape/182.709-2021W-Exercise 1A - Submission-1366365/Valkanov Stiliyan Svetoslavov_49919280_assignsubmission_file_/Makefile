DFLAGS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -pedantic -Wall -g 

all: mycompress

mycompress: mycompress.o
	gcc $(DFLAGS) -o mycompress mycompress.o

mycompress.o: mycompress.c
	gcc $(CFLAGS) -c -o mycompress.o mycompress.c

clean:
	rm -rf %.o mycompress