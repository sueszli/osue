mycompress: mycompress.o
	gcc -g -o mycompress mycompress.o
mycompress.o: mycompress.c
	gcc -c -g -o mycompress.o mycompress.c
clean:
	rm -rf *.o mycompress
tar:
	tar -cvzf mycompress.tgz mycompress.c Makefile