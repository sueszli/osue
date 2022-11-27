#author:		Alexander Gube
#program:		fb_arc_set
#date:		10.11.2021

DFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFS = -std=c99 -pedantic -Wall $(DFS)

.PHONY: all clean
all: supervisor generator

supervisor: supervisor.o ErrorHandling.o ShmUtils.o SemUtils.o CircularBufferUtils.o GraphUtils.o
	gcc -o supervisor supervisor.o ErrorHandling.o ShmUtils.o SemUtils.o CircularBufferUtils.o GraphUtils.o -pthread -lrt

supervisor.o: supervisor.c ErrorHandling.h SemUtils.h ShmUtils.h
	gcc $(CFS) -g -c -o supervisor.o supervisor.c

generator: generator.o ErrorHandling.o ShmUtils.o SemUtils.o CircularBufferUtils.o GraphUtils.o
	gcc -o generator generator.o ErrorHandling.o ShmUtils.o SemUtils.o CircularBufferUtils.o GraphUtils.o -pthread -lrt

generator.o: generator.c ErrorHandling.h SemUtils.h ShmUtils.h
	gcc $(CFS) -g -c -o generator.o generator.c


ErrorHandling.o: ErrorHandling.c ErrorHandling.h
	gcc $(CFS) -g -c -o ErrorHandling.o ErrorHandling.c

SemUtils.o: SemUtils.c SemUtils.h ErrorHandling.h
	gcc $(CFS) -g -c -o SemUtils.o SemUtils.c

ShmUtils.o: ShmUtils.c ShmUtils.h CircularBufferUtils.h
	gcc $(CFS) -g -c -o ShmUtils.o ShmUtils.c

CircularBufferUtils.o: CircularBufferUtils.c CircularBufferUtils.h GraphUtils.h
	gcc $(CFS) -g -c -o CircularBufferUtils.o CircularBufferUtils.c

GraphUtils.o: GraphUtils.c GraphUtils.h ErrorHandling.h
	gcc $(CFS) -g -c -o GraphUtils.o GraphUtils.c

clean:
	rm -rf *.o supervisor
	rm -rf *.o generator


