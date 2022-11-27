CFLAGS="-std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g"
LDFLAGS="-g"

all: mydiff

%.o: %.c
	gcc "$(CFLAGS)" -c $< -o $@

mydiff: mydiff.o
	gcc "$(LDFLAGS)" $< -o $@

clean:
	rm *.o

.PHONY: clean
