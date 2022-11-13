# Exercise 1A // mycompress
# ===============================
# @author Lucia Lechner <e0701149@student.tuwien.ac.at>
# @date 12.11.2021

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -pedantic -Wall $(DEFS) -g

A = mycompress

all:	$(A)

$(A):	$(A).c
	$(CC) $(CFLAGS) -o $(A) $(A).c

clean:
	$(RM) $(A)
