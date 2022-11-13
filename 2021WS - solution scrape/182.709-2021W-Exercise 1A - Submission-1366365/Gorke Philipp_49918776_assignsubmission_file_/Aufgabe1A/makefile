FLAGS = -g -c -Wall -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L

all: mydiff

mydiff: main.o help.o 
	gcc -g main.o help.o -o mydiff  

main.o: main.c help.h
	gcc $(FLAGS) main.c -std=c99

help.o: help.c help.h
	gcc $(FLAGS) help.c -std=c99

clean: 
	rm -rf *.o *.out mydiff	