# Author: 	Elias GALL - 12019857
# Programs:	supervisor
#			generator

all: supervisor generator

supervisor: supervisor.c circular_buffer.o
	gcc -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -o supervisor supervisor.c circular_buffer.o -lrt -pthread

generator: generator.c circular_buffer.o
	gcc -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -o generator generator.c circular_buffer.o -lrt -pthread

circular_buffer.o: circular_buffer.c
	gcc -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c -o circular_buffer.o circular_buffer.c

clean:
	rm supervisor generator circular_buffer.o