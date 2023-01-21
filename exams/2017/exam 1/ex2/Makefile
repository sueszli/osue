override CFLAGS += -Wall -g -std=c99 -pedantic -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809 \
                   -Wno-unused-variable -Wno-unused-function

LDOBJS.server = server_lib.o devices.o utils.o
LDOBJS.client = client_lib.o utils.o

.PHONY : all clean

all: server client

server: server.o
	gcc -o $@ $^ $(LDOBJS.server)

client: client.o
	gcc -o $@ $^ $(LDOBJS.client)

server.o: server.c server.h common.h
	gcc $(CFLAGS) -c -o $@ $<

client.o: client.c client.h common.h
	gcc $(CFLAGS) -c -o $@ $<

clean:
	rm -f server server.o client client.o
