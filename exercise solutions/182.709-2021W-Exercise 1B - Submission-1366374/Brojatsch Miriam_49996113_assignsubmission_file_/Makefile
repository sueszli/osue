all:	generator	supervisor

FLAGS=	-std=c99	-pedantic	-Wall	-D_DEFAULT_SOURCE	-D_BSD_SOURCE	-D_SVID_SOURCE	-D_POSIX_C_SOURCE=200809L	-g

generator:	generator.o	general.h
	gcc	-o generator	generator.o	general.h	-lpthread -lrt

generator.o:	generator.c	general.h 
	gcc	$(FLAGS)	-c	-o generator.o generator.c


supervisor:	supervisor.o	general.h
	gcc	-o supervisor	supervisor.o	general.h	-lpthread -lrt

supervisor.o:	supervisor.c	general.h
	gcc	$(FLAGS)	-c	-o supervisor.o supervisor.c


clean:
	rm	-f	generator	supervisor	*.o