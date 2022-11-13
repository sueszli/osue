FLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g

all: mydiff
	gcc -o mydiff mydiff.o
mydiff: mydiff.c 
	gcc $(FLAGS) -c mydiff.c

clean: 
	rm mydiff.o mydiff