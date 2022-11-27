CC = gcc
PNG = generator
PNS = supervisor
TUFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
LRT = -lrt
LPT = -lpthread

.PHONY: begin execute-gen execute-sup clean cleano all

all: generator.o myutil.o supervisor.o
	$(CC) -o $(PNG) myutil.o generator.o $(LRT) $(LPT)
	$(CC) -o $(PNS) myutil.o supervisor.o $(LRT) $(LPT)

generator.o: generator.c
	$(CC) $(TUFLAGS) -g -c -o generator.o generator.c

supervisor.o: supervisor.c
	$(CC) $(TUFLAGS) -g -c -o supervisor.o supervisor.c

myutil.o: myutil.c myutil.h
	$(CC) $(TUFLAGS) -g -c -o myutil.o myutil.c

execute-gen: all
	./$(PNG) 1-2 2-3 3-4 4-5 5-6 6-7 7-8 8-1

execute-sup: all
	./$(PNS)

clean:
	rm $(PNG)
	rm $(PNS)
	rm generator.o 
	rm myutil.o
	rm supervisor.o

cleano:
	rm generator.o 
	rm myutil.o
	rm supervisor.o 