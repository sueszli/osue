# Author: Artem Chornyi. 11922295
# Programs: generator supervisor

GCC_ARGS=-std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -DDEBUG -pthread
GCC_ARGS_SHM=-lrt
HEADER_FILES=
all: generator supervisor

generator: generator.o
	gcc ${GCC_ARGS} -o $@ $< ${HEADER_FILES} ${GCC_ARGS_SHM}

supervisor: supervisor.o
	gcc ${GCC_ARGS} -o $@ $< ${HEADER_FILES} ${GCC_ARGS_SHM}

%.o: %.c 
	gcc ${GCC_ARGS} -c $< ${HEADER_FILES}

clear:
	rm -f *.o generator supervisor
