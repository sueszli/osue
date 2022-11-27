## ----------------------------------------------------------------------
## Program: Exercise 2 - Assignment - Integer Multiplication
## Author: Merlin Zalodek, 12019836
## ----------------------------------------------------------------------

CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c

.PHONY: all clean

all: intmul

intmul: intmul.c
	$(CC) $(CFLAGS) intmul.c
	$(CC) intmul.o -o intmul

clean:
	rm *.o

package:
	tar -cvzf Zalodek_Merlin_12019836_2.tgz intmul.c makefile

commit:
	tar -cvzf Zalodek_Merlin_12019836_2.tgz intmul.c makefile
	scp -r Zalodek_Merlin_12019836_2.tgz y12019836@ssh.inflab.tuwien.ac.at:Documents/2/
