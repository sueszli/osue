FLAGS = -std=c99 -pedantic -Wall -Wextra -Wwrite-strings -Wconversion -g3 -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L

.PHONY: all clean
all: fifo

fifo: fifo.o
	gcc -o fifo fifo.o $(FLAGS) -lrt -pthread

fifo.o: fifo.c
	gcc -c -o fifo.o fifo.c $(FLAGS)

clean:
	rm -rf *.o fifo
