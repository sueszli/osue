SOURCE	= generator.c supervisor.c

CC	= gcc
FLAGS	= -g -c -Wall -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
LFLAGS	= -lrt -lpthread


all: mycompress.o
	gcc -g mycompress.o -o mycompress $(LFLAGS)

mycompress.o: mycompress.c
	gcc $(FLAGS) mycompress.c -std=c99

clean:
	rm -f mycompress mycompress.o