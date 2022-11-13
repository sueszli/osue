CC = gcc
DEFS = -D_BDS_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

.PHONY: all clean
all: myexpand

myexpand: ./src/myexpand.o
	$(CC) $(LDFLAGS) -o $@ $^

./src/myexpand.o: ./src/myexpand.c
	$(CC) -c -o $@ $<

clean:
	rm -rf ./src/*.o myexpand