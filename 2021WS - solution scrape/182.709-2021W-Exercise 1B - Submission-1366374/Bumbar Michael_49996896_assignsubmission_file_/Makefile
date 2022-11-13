FLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIC_C_SOURCE=200809L -g
LINKING = -lrt -pthread

.PHONY: all clean

all: supervisor generator

supervisor: supervisor.o circularbuffer.o
	gcc $(FLAGS) -o $@ $^ $(LINKING)

generator: generator.o circularbuffer.o 
	gcc $(FLAGS) -o $@ $^ $(LINKING)

%.o: %.c
	gcc $(FLAGS) -c -o $@ $<

supervisor.o: supervisor.c circularbuffer.h
generator.o: generator.c circularbuffer.h
circularbuffer.o: circularbuffer.c circularbuffer.h

clean:
	rm -rf *.o supervisor generator
