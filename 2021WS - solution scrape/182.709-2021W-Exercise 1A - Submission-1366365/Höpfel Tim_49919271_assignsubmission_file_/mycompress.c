/**
 * @file mycompress.c
 * @author Tim Höpfel Matr.Nr.: 01207099 <e01207099@student.tuwien.ac.at>
 * @date 12.11.2021
 *
 * @brief Main program module
 *
 * This program compresses text with a simple algorithm. The input is compressed by substituting subsequent identical characters by only one occurence of the
 * character followed by the number of characters. For example, if you encounter the sequence aaa, then it is replaced by a3.
 **/

#include "compresshelper.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
//for open
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

static char *myprog; /** The Program name. */

/**
 * Mandatory usage function.
 * @brief This function writes helpful usage information about the program to stderr.
 * @details global variables: myprog
 */
static void usage(void) {
	(void) fprintf(stderr, "USAGE: %s [-o outfile] [inputfile...]\n", myprog);
	exit(EXIT_FAILURE);
}

/**
 * Program entry point.
 * @brief The program starts here. This function takes care about parameters and the file-handling of input and output.
 * @details The program reads the input with getopt. A buffer for writing and a buffer for reading are created. The inputfiles are read. If there are no
 * Inputfiles provided, stdin is used for input. The outputfile is opend if a name is provided. If the file doesn't exit, it's being created. If no
 * outputfile is provided, stdout is used for output.
 * The compression is handled by the compress function of compresshelper.c.
 * In the end of the programm, the number of charakters read and written and the compression ration are printed to sterr.
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS.
 */
int main (int argc, char *argv[]) {

    myprog = argv[0];

    char *o_arg = NULL; //pointer on argument of o
    int c; //saves return of getopt

    while((c = getopt(argc, argv, "o:")) != -1 ){
        switch(c) {
            case 'o': o_arg = optarg;
                break;
            case '?':
                usage();
        }
    }
    char writebuffer[1024*2]; //double the size of the read buffer, for worst case abc -> a1b1c1
    FILE *out;

    char *output;

    if(o_arg == NULL) {
        //no Outputfile specified, so output to stdout
        out = stdout;
    } else {
        //if no filename is provided, show usage
        if(sizeof(o_arg) <= 1) {
            usage();
        }
        output = o_arg;

        //open outputfile
        if((out = fopen(output, "a")) == NULL) {
            //fopen failed
            fprintf(stderr, "open outputfilefailed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    int number_of_inputfiles = (argc - optind);
    const char *inputfiles[number_of_inputfiles + 1]; //stores the names of the inputfiles
    FILE* in[number_of_inputfiles + 1];
    //get input from stdin if no file is provided
    if (number_of_inputfiles == 0) {
        //set input to stdin
        in[0] = stdin;
        printf("Enter text to be compressed. Press Ctrl+D when finished.\n");
    } else {
        //open every input file
        for(int i = number_of_inputfiles; i > 0; i--){
        //if no output is specified, start with file at argv[1]
            if(o_arg == 0) {
                inputfiles[i - 1] = argv[i];
            } else {
                inputfiles[i - 1] = argv[i + 2];
            }

            if((in[i - 1] = fopen(inputfiles[i - 1], "r")) == NULL) {
                    //fopen failed
                    fprintf(stderr, "open inputfile %s failed: %s\n",inputfiles[i - 1], strerror(errno));
                    fclose(out);
                    exit(EXIT_FAILURE);

            }
        }
    }
    int compressed_from = 0;
    int compressed_to = 0;
    //increase number of input files for no input, so the for-loop gets accessed one time.
    if(number_of_inputfiles == 0) {
        number_of_inputfiles = 1;
    }
    //read every file
    for(int i = 1; i <= number_of_inputfiles; i++) {
        char newline = '\n';
        int newline_int = newline;
        int newlinecounter = 0;

        char buffer[1024] = "\n";

        bool first = true;
        while (fgets(buffer, sizeof(buffer), in[i - 1]) != NULL) {
            if(first) {
                first = false;
            } else {
                newlinecounter++;
                compressed_to++;
            }
            //compress buffer and write it to writebuffer
            int tempint = 0;
            int *tempint_fromsize = &tempint;
            compressed_to += compress(buffer, writebuffer, tempint_fromsize);
            compressed_from += *tempint_fromsize;
            //printf("%c; has size: %li",buffer[0], (sizeof(writebuffer)/sizeof(writebuffer[0])));

            //on first occounter write \n and count
            if(buffer[0] ==  newline_int && newlinecounter < 1) {
                newlinecounter = 1;
            } else if (buffer[0] ==  newline_int && newlinecounter >= 1) {
                //another \n has been detected dont write it again, but increase counter
                continue;
            } else if( newlinecounter >= 1) {
                char intchar[12];
                sprintf(intchar, "%d", newlinecounter);
                if(fputc(intchar[0], out) == EOF) {
                    fprintf(stderr, "write failed: %s\n", strerror(errno));
                    fclose(in[i - 1]);
                    fclose(out);
                    exit(EXIT_FAILURE);
                }
                compressed_to++;
                newlinecounter = 0;
            }

            if(fputs(writebuffer, out) == EOF) {
                fprintf(stderr, "write failed: %s\n", strerror(errno));
                fclose(in[i - 1]);
                fclose(out);
                exit(EXIT_FAILURE);
            } else {
                newlinecounter = 0;
            }

        }
        if(ferror(in[i - 1])) {
            //fgets failed
            fprintf(stderr, "read failed: %s\n", strerror(errno));
                fclose(in[i - 1]);
                fclose(out);
                exit(EXIT_FAILURE);
        }
        //if input ends with \n print nextline and number of nextlines
        if(newlinecounter != 0) {
            char tempstr[12];
            snprintf(tempstr, 12, "%i", newlinecounter);
            if(fputs(tempstr, out) == EOF) {
                fprintf(stderr, "write failed: %s\n", strerror(errno));
                fclose(in[i - 1]);
                fclose(out);
                exit(EXIT_FAILURE);
            }
        }

        //close inputfiles
        fclose(in[i - 1]);
    }
    //correct compress from
    compressed_from--;
    fclose(out);

    fprintf(stderr, "\nRead: %i charakters\n", compressed_from);
    fprintf(stderr, "Written: %i charakters\n", compressed_to);
    if(compressed_from == 0) {
        fprintf(stderr, "Compression ratio not determined.\n");
    } else {
        fprintf(stderr, "Compression ratio: %.1f%%\n", ((float)compressed_to / compressed_from * 100));
    }
    return EXIT_SUCCESS;

}
