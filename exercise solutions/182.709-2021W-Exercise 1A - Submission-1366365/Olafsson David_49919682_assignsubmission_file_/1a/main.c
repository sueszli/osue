#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <malloc.h>

#include "compress.h"

int main(int argc, char **argv) {
    int option = getopt(argc, argv, "o:");
    FILE *output;
    // Choose output destination:
    if (option == 'o') {
        // file as output
        output = fopen(optarg, "a");
        if (output == NULL){
            fprintf(stderr, "Failed to open specified output file: %s\n", strerror(errno));
        }
    } else {
        // stdout as output
        output = stdout;
    }

    //Keeps track of compression ratio
    int *readChars = malloc(sizeof(int));
    *readChars = 0;
    int *writtenChars = malloc(sizeof(int));
    *writtenChars = 0;

 
    char *buffer;
    int bufferSize = 100;
    //choose input source
    // if no positional arguments, get from stdin
    if (optind >= argc) {
        // size of buffer?
        buffer = malloc(bufferSize * sizeof(char));
    	// write compressed to buffer while incrementing writtenChars, readChars
	 compress(stdin, buffer, readChars, writtenChars);
    	if (fputs(buffer, output) == EOF){
	    fprintf(stderr, "Failed to write to output: %s", strerror(errno));
	    exit(EXIT_FAILURE);
	}
    } else {
        // get from arguments

        int argIndex = optind;
        buffer = malloc(bufferSize * sizeof(char));
        while (argIndex < argc) {

            FILE *inputFile = fopen(argv[argIndex], "r");
            if (inputFile == NULL){
                fprintf(stderr, "Failed to open specified input file: %s\n", strerror(errno));
            	exit(EXIT_FAILURE);
	    }
            // empty buffer
            memset(buffer, 0, bufferSize*sizeof(typeof(*buffer)));
	    
    	    // write compressed to buffer while incrementing writtenChars, readChars
	    compress(inputFile, buffer, readChars, writtenChars);
            argIndex++;
            //write to output (stdout or file)
    	    if (fputs(buffer, output) == EOF){
	    	fprintf(stderr, "Failed to write to output: %s", strerror(errno));
		exit(EXIT_FAILURE);
	    }
        }
    }


    free(buffer);

    if (option == 'o')
        fclose(output);
    float compRatio = 100.*((float) *writtenChars/ (float) *readChars); 
    fprintf(stderr, "\nRead:\t\t%i characters\nWritten:\t%i characters\nCompression Ratio: %9.1f\%\n",*readChars, *writtenChars, compRatio);    
    free(readChars);
    free(writtenChars);
    return 0;
}
