# Author: Artem Chornyi. 11922295
# Programs: mygrep

CODE_FILES=mygrep.c
OBJECT_FILES=mygrep.o
GCC_ARGS=-std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -DDEBUG

all: mygrep

mygrep: ${OBJECT_FILES}
	gcc ${GCC_ARGS} -o $@ ${OBJECT_FILES}

%.o: %.c 
	gcc ${GCC_ARGS} -c $<

clear:
	rm -f *.o mygrep