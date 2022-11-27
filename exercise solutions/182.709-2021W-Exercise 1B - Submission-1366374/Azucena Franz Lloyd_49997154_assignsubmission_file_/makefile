all:  supervisor generator

flags = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g

# -l linkds libraries
#  pthread: libthread
#  rt: librt (real-time extension) 
libs = -lpthread -lrt

circularbuffer.o: circularbuffer.c
	gcc $(flags) -c circularbuffer.c $(libs)

supervisor.o: supervisor.c
	gcc $(flags) -c supervisor.c

supervisor: supervisor.o circularbuffer.o
	gcc -o supervisor supervisor.o circularbuffer.o $(libs)

generator.o: generator.c
	gcc $(flags) -c generator.c

generator: generator.o circularbuffer.o
	gcc -o generator generator.o circularbuffer.o $(libs)

clean:
	rm -f *.o
	rm -f supervisor
	rm -f generator
