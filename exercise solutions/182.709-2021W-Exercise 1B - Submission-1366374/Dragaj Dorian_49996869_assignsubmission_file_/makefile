CC = gcc 
DEFS = -D_BSD_SOURCE -D_DEFAULT_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
LDFLAGS = -lrt -pthread

OBJS = shared_mem.o circ_buff.o

EXE = supervisor.elf generator.elf

.PHONY: all clean
all: $(EXE)

%.elf: %.o $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $< $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

tar:
	tar -cvzf archive-Task1B.tgz makefile *.c *.h

clean:
	rm -rf *.o $(EXE)

