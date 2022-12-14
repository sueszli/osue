defs = -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809
cflags = -std=c99 -pedantic -g3 -Wall -Wextra -Wwrite-strings -Wconversion $(defs)
libs = -pthread -lrt

.PHONY : all clean

all: server

server: server.o
	gcc -o server server.o server_lib.o $(libs)

server.o: server.c server.h common.h
	gcc -c -o server.o server.c $(cflags)

clean:
	rm -f server server.o
