## @file: Makefile
## @author: Maximilian Scharizer 016047709
## @date 11.12.2021
## @brief File to automate the task of generating an executable from C-File 


CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS) -fdiagnostics-color=always
LDFLAGS = -lrt -pthread -lm

OBJECTS = cpair.o

.PHONY: all clean release
all: cpair

cpair: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf *.o cpair
