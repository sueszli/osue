all: main.o 
	gcc -Wall -o intmul intmul.o 

compile_infos = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SCID_SOURCE -D_POSIX_C_SOURCE=200809L -g

main.o: intmul.c
	gcc $(compile_infos) -c intmul.c	

clean: 
	rm -f *.o intmul