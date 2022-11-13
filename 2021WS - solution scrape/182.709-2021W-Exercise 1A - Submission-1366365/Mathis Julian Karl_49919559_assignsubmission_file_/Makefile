author = Julian Mathis 12024756
program_name = ispalindrom

all: main.o 
	gcc -Wall -o ispalindrom main.o 

compile_infos = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SCID_SOURCE -D_POSIX_C_SOURCE=200809L -g

main.o: main.c
	gcc $(compile_infos) -c main.c	

clean: 
	rm -f *.o ispalindrom