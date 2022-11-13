# @author Vladyslav Hnatiuk(01613669)
# @name Arc Set

all: supervisor generator

supervisor: supervisor.c
	gcc -o supervisor -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -lrt -lpthread supervisor.c

generator: generator.c
	gcc -o generator -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -lrt -lpthread generator.c

clean:
	rm generator supervisor
