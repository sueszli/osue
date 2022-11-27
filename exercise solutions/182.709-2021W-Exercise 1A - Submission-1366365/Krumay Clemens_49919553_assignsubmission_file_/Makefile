# @name mydiff
# @author Krumay Clemens (e11831160@student.tuwien.ac.at)

all: mydiff

clean:
	rm mydiff.o mydiff

mydiff: mydiff.o
	gcc -o mydiff mydiff.o

mydiff.o: mydiff.c
	gcc -std=c99 -pedantic -Wall -D_DEAFULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c mydiff.c
