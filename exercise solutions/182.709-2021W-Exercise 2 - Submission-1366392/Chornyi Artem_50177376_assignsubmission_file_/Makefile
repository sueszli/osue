# Author: Artem Chornyi. 11922295
# Programs: forkFFT

GCC_ARGS=-std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -DDEBUG
GCC_ARGS_AFTER=-lm

all: forkFFT

forkFFT: forkFFT.o rai_number.o
	gcc ${GCC_ARGS} -o $@ forkFFT.o rai_number.o ${GCC_ARGS_AFTER}

%: %.o
	gcc ${GCC_ARGS} -o $@ $< ${GCC_ARGS_AFTER}

%.o: %.c 
	gcc ${GCC_ARGS} -c $<

clear:
	rm -f *.o forkFFT  
