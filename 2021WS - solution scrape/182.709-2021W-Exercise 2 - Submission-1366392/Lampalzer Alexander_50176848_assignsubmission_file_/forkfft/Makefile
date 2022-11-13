# forkFFT - Calculate Fourier Transformation in multiple processes
# Author: Alexander Lampalzer <e12023145@student.tuwien.ac.at>

all: forkfft

forkfft:
	gcc -std=c99 -Iinclude -pthread -Wall -pedantic -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -o forkFFT -g src/main.c -lrt -lm

debug:
	gcc -fsanitize=address -g3 -std=c99 -Iinclude -pthread -Wall -pedantic -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -o forkFFT -g src/main.c -lrt -lm


clean:
	rm -f *.o
	rm -f forkFFT
	rm -rf doc/man
	rm -rf doc/latex
	rm -rf doc/html
	rm -rf build
	rm -rf cmake-build-debug

docs:
	doxygen doc/Doxyfile