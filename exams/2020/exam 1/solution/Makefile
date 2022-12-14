defs = -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809
cflags = -std=c99 -pedantic -g3 -Wall -Wextra -Wwrite-strings -Wconversion $(defs)
libs = -pthread -lrt

.PHONY : all clean

all: client

client: client.o client_lib.o
	gcc -o client client.o client_lib.o $(libs)

client.o: client.c
	gcc -c -o client.o client.c $(cflags)

clean:
	rm -f client client.o