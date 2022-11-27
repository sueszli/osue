/**
 * @file mycompress.c
 * @author Philipp Holzer <e12028208@student.tuwien.ac.at>
 * @date 2021-11-14
 * @brief mycompress
 * @details small programm that returns the run-length-encoding of given input(s) to output file or console 
 **/

#include "mycompress.h"

static void usage(void);
static void fileerror(void);
static void compress(FILE *out, FILE *in);
int main(int argc, char *argv[]);

static char *name; // name of the program

/**
 * @brief provide correct usage of programm written to stderr and exit program with a EXIT_FAILURE
 * @details global variable: name
 **/
static void usage(void) {
    fprintf(stderr, "USAGE: %s [-o outfile] [file...]", name);
    exit(EXIT_FAILURE);
}

/**
 * @brief throw error if fopen failed and exit program with a EXIT_FAILURE
 * @details global variable: name
 **/
static void fileerror(void) {
    fprintf(stderr, "%s: fopen failed: %s\n", name, strerror(errno));
    exit(EXIT_FAILURE);
}

/**
 * @brief applies run-length encoding to in and writes it to out 
 * (also outputs general information about the convertion)
 * @param out writing to
 * @param in reading from 
 **/
static void compress(FILE *out, FILE *in) {
    int read = 0, 
        written = 0, 
        current = EOF,
        next = 0,
        amount = 0;

    while(1) {
        next = fgetc(in);

        if(current == EOF) {
            current = next;
            amount = 1;
            if(next == EOF){
			    break;
		    }
        }
        if(current != next) {
            fprintf(out, "%c%d", current, amount);
            written += 2;
            
            current = next;
            amount = 1;
            if(next == EOF){
			    break;
		    }
        } else {
            amount++;
        }
        
        read++;
    }

    fprintf(out, "\n"); // add newline to out file

    if(ferror(in)) { // error reading file
        fileerror();
    }

    fprintf(stderr, "Read:\t\t%d characters\n", read);
	fprintf(stderr, "Written:\t%d characters\n", written);

    float compressr = 0; // compress ratio
    if(read != 0) { // avoid dividing by 0
        compressr = ((float)written / read) * 100;
    }
	fprintf(stderr, "Compression ratio:\t%.1f%%\n", compressr);

    fflush(out); // clear (or flush) the output buffer of a stream
    fflush(stderr);
}

/**
 * @brief main function: processes arguments (or throw errors), read input and output files (or console) and call the compress function (if used correctly).
 * @param argc number of arguments
 * @param argv array of arguments
 * @return EXIT_SUCCESS
 * @details global variable: name
 **/
int main(int argc, char *argv[]) {
    name = argv[0];

    char *arg = NULL;
    int c;

    FILE *out = NULL,
         *in = NULL;

    while((c = getopt(argc, argv, "o:")) != -1) {
        switch(c) {
            case 'o': 
                if(arg == NULL) {
                    arg = optarg;
                } else {
                    fprintf(stderr, "%s: provide only one -o option!\n", name); usage();
                }
                break;
            default: fprintf(stderr, "%s: invalid option!\n", name); usage();
        }
    }

    if(arg != NULL) {
        out = fopen(arg, "w");
        if(!out) {
            fileerror();
        }
    } else {
        out = stdout;
    }

    int fileCount = argc - optind;
    if(fileCount != 0) { // at least one input file provided
        for(int i = optind; i < argc; i++) {
            in = fopen(argv[i], "r"); // opens inFile in read mode (file must exist)
            if(!in) {
                fileerror();
            }
            compress(out, in);
            fclose(in); // closes the stream and flushes all buffers
        }
    } else { // no input file provided
        if(isatty(fileno(stdin))) { // stdin is a terminal (tty)
            fprintf(stderr, "%s: enter a string:\n", name);
        }
        compress(out, stdin);
    }
    
    return EXIT_SUCCESS; // EXIT_FAILURE thrown above
}