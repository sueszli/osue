FLAGS = -std=c99 -pedantic -Wall -Wextra -Wwrite-strings -Wconversion -g3 -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L

.PHONY: all clean
all: mirror

mirror: mirror.o
	gcc -o mirror mirror.o $(FLAGS)

mirror.o: mirror.c
	gcc -c -o mirror.o mirror.c $(FLAGS)

clean:
	rm -rf *.o mirror