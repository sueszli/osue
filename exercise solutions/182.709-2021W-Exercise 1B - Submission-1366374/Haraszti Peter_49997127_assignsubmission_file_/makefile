# 3coloring by Peter Haraszti, e12019844@student.tuwien.ac.at
all: supervisor generator

supervisor: supervisor.o
	gcc -o supervisor supervisor.o structs.h -lrt -lpthread
supervisor.o: supervisor.c
	gcc -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c supervisor.c
generator: generator.o structs.h
	gcc -o generator generator.o structs.h  -lrt -lpthread
generator.o: generator.c
	gcc -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c generator.c
clean:
	rm *.o