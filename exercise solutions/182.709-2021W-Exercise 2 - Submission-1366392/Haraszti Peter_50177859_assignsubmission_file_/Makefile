# forkFFT by Peter Haraszti, e12019844@student.tuwien.ac.at

all: forkFFT bonus

forkFFT: forkFFT.o structs.h
	gcc -o forkFFT forkFFT.o structs.h -lm

forkFFT.o: forkFFT.c
	gcc -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c forkFFT.c

bonus: bonus.o structs.h
	gcc -o bonus bonus.o structs.h -lm

bonus.o: bonus.c
	gcc -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c bonus.c


clean:
	rm *.o