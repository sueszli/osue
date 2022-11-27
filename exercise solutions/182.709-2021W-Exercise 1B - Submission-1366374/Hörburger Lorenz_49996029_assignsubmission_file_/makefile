comp_info = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L 

all: generator.o supervisor.o circular_buffer.o error.o
	gcc -o generator generator.o error.o
	gcc -o supervisor supervisor.o error.o
	gcc -o circular_buffer circular_buffer.o error.o
generator.o: generator.c
	gcc $(comp_info) -g -c -o generator.o generator.c
supervisor.o: supervisor.c
	gcc $(comp_info) -g -c -o supervisor.o supervisor.c
circular_buffer.o: circular_buffer.c
	gcc $(comp_info) -g -c -o circular_buffer.o circular_buffer.c
error.o: error.c
	gcc $(comp_info) -g -c -o error.o error.c
clean:
	rm -rf *.o