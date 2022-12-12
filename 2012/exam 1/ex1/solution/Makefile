CFLAGS=-std=c99 -pedantic -Wall -Wextra -Wwrite-strings -Wconversion -g3 -D_XOPEN_SOURCE=500 -D_BSD_SOURCE
# --> added -Wall -Wextra -Wwrite-strings -Wconversion -g3

.PHONY: clean
all: listtool

listtool: list.o listtool.o
	gcc -o list.o listtool.o -lcrypt 

listtool.o: listtool.c list.h
	gcc -c -o listtool.c list.h $(CFLAGS) 

clean:
	rm -rf listtool listtool.o
