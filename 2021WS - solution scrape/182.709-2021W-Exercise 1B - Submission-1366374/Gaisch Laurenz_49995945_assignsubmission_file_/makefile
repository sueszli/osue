# @programname fb-arc-set
# @author Laurenz Gaisch <e11808218@student.tuwien.ac.at>

all: supervisor generator

compile_flags = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SCID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c
link_libaries = -lpthread -lrt

supervisor: supervisor.o circular-buffer.o
	gcc -o supervisor supervisor.o circular-buffer.o $(link_libaries)

supervisor.o: supervisor.c
	gcc $(compile_flags) supervisor.c

generator: generator.o circular-buffer.o
	gcc -o generator generator.o circular-buffer.o $(link_libaries)

generator.o: generator.c
	gcc $(compile_flags) generator.c

circular-buffer.o: circular-buffer.c
	gcc $(compile_flags) circular-buffer.c

clean:
	rm -f *.o
	rm -f generator
	rm -f supervisor

