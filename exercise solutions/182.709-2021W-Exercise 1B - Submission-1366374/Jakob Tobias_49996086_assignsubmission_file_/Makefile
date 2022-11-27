SRC = arcset
MODULES = generator.o supervisor.o

CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g
LDFLAGS = -lpthread -lrt

all: generator supervisor

%: %.o
	gcc $(CFLAGS) -o $@ $^ $(LDFLAGS)
 
%.o: %.c
	gcc $(CFLAGS) -c -o $@ $<
 
clean: 
	rm -rf ./*.o intro
