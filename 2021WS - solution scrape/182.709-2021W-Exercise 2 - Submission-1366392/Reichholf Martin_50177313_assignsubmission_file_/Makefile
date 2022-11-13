CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c 
LFLAGS = -lrt -lpthread -lm

all: fft 
 
fft: fft.o 
	$(CC) -o fft fft.o $(LFLAGS) 
 
fft.o: fft.c
	$(CC) $(CFLAGS) fft.c
 
clean:
	rm -f fft.o fft
