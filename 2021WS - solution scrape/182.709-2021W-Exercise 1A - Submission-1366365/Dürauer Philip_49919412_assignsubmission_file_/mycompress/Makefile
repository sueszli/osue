#name http
#author Philip Duerauer 11811848
#brief Makefile for mycompress


# config

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

# objects to build
OBJS = mycompress.o 




.PHONY : all clean

all: mycompress 


mycompress: $(OBJS) $(LDOBJS)
	gcc $(LDFLAGS) -o $@ $^ $(LIBS)


%.o: %.c
	gcc $(CFLAGS) -c -o $@ $<

clean:
	rm -f mycompress $(OBJS)


