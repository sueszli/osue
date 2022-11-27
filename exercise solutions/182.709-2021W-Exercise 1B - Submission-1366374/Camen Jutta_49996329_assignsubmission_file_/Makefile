#@file Makefile
#@author Jutta Camen <e01452468@student.tuwien.ac.at>
#@date 12.11.2021

CC = gcc -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g

.PHONY: all clean zip doc

all: supervisor generator

supervisor: src/supervisor.c src/configuration.h
	$(CC) -o $@ $< -pthread

generator: src/generator.c src/configuration.h
	$(CC) -o $@ $< -pthread

clean:
	rm -rf supervisor generator
	rm -rf doc/html doc/latex

doc:
	cd doc
	doxygen doc/Doxyfile

zip:
	rm -rf OSUE-1B-01452468.tgz
	tar -cvzf OSUE-1B-01452468.tgz Makefile doc/Doxyfile src
