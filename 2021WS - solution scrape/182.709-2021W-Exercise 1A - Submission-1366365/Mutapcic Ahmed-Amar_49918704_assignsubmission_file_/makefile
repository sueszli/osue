CFLAG = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L

build: clean mydiff.c mydiff.o
	  	 gcc -o mydiff mydiff.o

myexpand.o: mydiff.c
	        gcc $(CFLAG) -c mydiff.c -o mydiff.o

clean:
