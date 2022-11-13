/**
 * @file mycompress.c
 * @author Josef Rabmer 11911128
 * 
 * @brief implements a compression algorithm, where it compresses the input and writes it to an output
 * 
 * @details the programm implements a compression algorithm, where repeating characters are compressed
 *          replaced by one character and the number of occurances. It takes it`s input from one or more 
 *          files or from stdin, if no files are specified. If an output file is specified, it writes
 *          it into the output file. If not it writes it into stdout.
 * 
 *          The main programm handles arguments and which output / input files to select. Once that is
 *          done, it calls compress_input_print_to_output to compress the input and store it in output.
 * 
 * @version 0.1
 * @date 2021-11-14
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "compression.h"
#include <getopt.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

char* prog_name;

/**
 * @brief is used to print the correct usage of a programm
 *
 * @details specifies the correct usage in the implementation and prints it to stdout.
 * 			It then terminates the programm. 
 * 
 */
void usage(void);

int main(int argc, char *argv[]) {
    prog_name = argv[0];
    int c;
    char* output_path = NULL;
    int opt_o = 0;
    // Parse options
    while ( (c = getopt(argc, argv, "o:")) != -1 ) {
        switch ( c ) {
            case 'o': 
                output_path = optarg;
                opt_o++;
                break;
            case '?': 
                usage();
            default:
                assert(0);
        }
    }
    
    FILE *in, *out;

    // Output
    if (opt_o > 1) // user tried specifying multiple output files
    {    
        usage();
    } else if (opt_o == 1){
        if ((out = fopen(output_path, "w")) == NULL){
            fprintf(stderr, "[%s] fopen failed: %s\n", prog_name, strerror(errno));
            exit(EXIT_FAILURE);
        }
    } else {
        out = stdout;
        if (out == NULL){
            fprintf(stderr, "[%s] stdout is closed: %s\n", prog_name, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }  
    
    // Input
    // Compress all specified input files
    // If no input files are specified, read from stdin
    int char_count_read_written[] = {0,0};
    if (optind < argc)
    {
        for(; optind < argc; optind++){
    
        if ((in = fopen(argv[optind], "r")) == NULL){
            fprintf(stderr, "[%s] fopen failed: %s\n", prog_name, strerror(errno));
            exit(EXIT_FAILURE);
        }
    
        compress_input_print_to_output(in, out, char_count_read_written);

        fclose(in);

        }

    } else {
        in = stdin;

        if (in == NULL){
            fprintf(stderr, "[%s] stdin is closed: %s\n", prog_name, strerror(errno));
            exit(EXIT_FAILURE);
        }

        compress_input_print_to_output(in, out, char_count_read_written);
    }
    
    // Print statistics
    double ratio;
    if(char_count_read_written[0] == 0){
        ratio = 0;
    } else {
        ratio = (double) char_count_read_written[1] / (double)char_count_read_written[0] * 100;
    }
    fprintf(stderr,"\nRead:      %d characters\nWritten:   %d characters\nCompression ratio: %.1f%%\n", char_count_read_written[0], char_count_read_written[1], ratio);  
    
    return EXIT_SUCCESS;   
}

/* ------------------------ Helper Function --------------------------*/

void usage(void) {
    fprintf(stderr,"Usage: %s [-o file] [file...]\n", prog_name);
    exit(EXIT_FAILURE);
}