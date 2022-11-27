all: main.o mydiff.o 
	gcc -o mydiff main.o mydiff.o

main.o: main.c
	gcc -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c main.c

mydiff.o: mydiff.c
	gcc -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c mydiff.c

clean: 
	rm -f *.o mydiff
