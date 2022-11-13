# Exercise 2 // cpair
# ===============================
# @author Lucia Lechner <e0701149@student.tuwien.ac.at>
# @date 06.12.2021

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
LDFLAGS = -lrt -pthread -lm

A = cpair

.PHONY: all clean
all:	$(A) 

$(A):	$(A).o
	$(CC) -o $@ $^ $(LDFLAGS)

%.o:	%.c 
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf *.o $(A) 
