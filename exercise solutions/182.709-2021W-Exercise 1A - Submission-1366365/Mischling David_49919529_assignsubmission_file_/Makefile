flags = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SCID_SOURCE -D_POSIX_C_SOURCE=200809L -g

all: mycompress

mycompress.o: mycompress.c
	gcc $(flags) -c mycompress.c

mycompress: mycompress.o
	gcc -o mycompress mycompress.o

clean:
	rm -rf mycompress mycompress.o
