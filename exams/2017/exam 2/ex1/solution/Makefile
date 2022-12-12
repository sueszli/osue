defs = -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809
cflags = -std=c99 -pedantic -g3 -Wall -Wextra -Wwrite-strings -Wconversion $(defs)
libs = -pthread -lrt

.PHONY : all clean

all: validate

validate: validate.o validate_lib.o
	gcc -o validate validate.o validate_lib.o $(libs)

validate.o: validate.c validate.h
	gcc  -c -o validate.c $(cflags)

clean:
	rm -f validate validate.o