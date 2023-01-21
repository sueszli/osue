defs = -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809
cflags = -std=c99 -pedantic -g3 -Wall -Wextra -Wwrite-strings -Wconversion $(defs)

LDOBJS.server = server_lib.o devices.o utils.o
LDOBJS.client = client_lib.o utils.o

.PHONY : all clean

all: server client

server: server.o
	gcc -o server server.o $(LDOBJS.server)

client: client.o
	gcc -o client client.o $(LDOBJS.client)

server.o: server.c server.h common.h
	gcc -c -o server.o server.c $(cflags) 

client.o: client.c client.h common.h
	gcc -c -o client.o client.c $(cflags) 

clean:
	rm -f server server.o client client.o
