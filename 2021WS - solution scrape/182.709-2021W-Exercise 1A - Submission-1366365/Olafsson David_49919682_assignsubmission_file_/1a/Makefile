CC = gcc

.PHONY: all clean
all: myCompress

myCompress: main.o compressfunc.o
	$(CC) -o $@ $^ 

main.o: main.c compress.h
	$(CC) -c -o $@ $<

compressfunc.o: compressfunc.c
	$(CC) -c -o $@ $<

clean:
	 rm -rf *.o myCompress
