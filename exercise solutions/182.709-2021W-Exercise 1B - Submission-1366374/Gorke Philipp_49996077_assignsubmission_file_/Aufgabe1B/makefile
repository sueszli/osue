FLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c

all: generator.o supervisor.o helpFunctions.h
	gcc -g generator.o -o generator -lrt -lpthread
	gcc -g supervisor.o -o supervisor -lrt -lpthread

generator.o: generator.c helpFunctions.h
	gcc $(FLAGS) generator.c

supervisor.o: supervisor.c helpFunctions.h
	gcc $(FLAGS) supervisor.c 

clean: 
	rm -rf *.o generator supervisor	