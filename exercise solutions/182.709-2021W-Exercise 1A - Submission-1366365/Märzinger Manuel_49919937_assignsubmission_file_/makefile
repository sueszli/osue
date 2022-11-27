all :		mydiff.o
			gcc -Wall -o mydiff mydiff.o

mydiff.o :	mydiff.c
			gcc -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c -o mydiff.o mydiff.c

clean: 
			rm *.o mydiff 	
