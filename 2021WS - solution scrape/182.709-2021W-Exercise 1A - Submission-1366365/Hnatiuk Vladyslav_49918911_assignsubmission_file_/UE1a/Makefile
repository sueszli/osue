# @author Vladyslav Hnatiuk(01613669)
# @name mycompress

all: mycompress

mycompress: main.c
	gcc -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -lm -g -o mycompress main.c

clean:
	rm mycompress
