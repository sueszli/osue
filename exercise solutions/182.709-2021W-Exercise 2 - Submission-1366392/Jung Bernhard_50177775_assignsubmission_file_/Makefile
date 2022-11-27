CC = gcc
PN = forkFFT
TUFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
LRT = -lrt
LPT = -lpthread
LM  = -lm

.PHONY: begin execute-gen execute-sup clean cleano all

all: myline.o
	$(CC) $(TUFLAGS) -g -o $(PN) main.c $(LM)

myline.o: myline.c myline.h
	$(CC) $(TUFLAGS) -g -c -o myline.o myline.c

execute: all
	./$(PN) < input.txt

clean:
	rm $(PN)