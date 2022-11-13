/**
 * @file main.c
 * @author Michael Trauner <e12019868@student.tuwien.ac.at>
 * @date 09.11.2021
 *
 * @brief Main program module.
 *
 * This program reads from given files or stdin, run-length encodes the input and outputs on a given file or stdout
 **/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <errno.h>

#include "functions.h"

char *prog_name;

void usage(void) {
    fprintf(stderr, "Usage: %s [-o outputfile] inputfile1 inputfile2 ...\n", prog_name);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    prog_name = argv[0];

    char *o_arg = NULL;
    int c;

    int input_size = 0;

    char *output = malloc((sizeof(char)) * 1);

    if (output == NULL) {
        fprintf(stderr, " [%s] ERROR: malloc for output failed: %s\n", prog_name,
                strerror(errno));
        exit(EXIT_FAILURE);
    }

    output[0] = '\0'; // terminate the allocated string

    while ((c = getopt(argc, argv, "o:")) != -1) {
        switch (c) {
            case 'o':
                if(o_arg != NULL) {
                    usage();
                }
                o_arg = optarg;
                break;
            case '?':
                usage();
            default:
                assert(0);
                usage();
        }
    }

    if ((argc - optind) == 0) {
        /* No input -> Input from stdin */

        int ret_print;
        int out_index = 0;

        ret_print = printf("Please enter data, to end the input press CTRL+D CTRL+D: \n");

        if (ret_print == -1) {
            fprintf(stderr, " [%s] ERROR: printf failed: %s\n", prog_name, strerror(errno));
            exit(EXIT_FAILURE);
        }

        char *new_output = runlengthEncode(prog_name, stdin, output, out_index, &input_size);

        output = new_output;

    } else {
        /* there are input arguments -> input from given file(s) */

        // Process each positional argument(file)
        for (int i = optind; i < argc; ++i) {
            int out_index = (int) strlen(output);
            char *input_filename = argv[i];

            FILE *input_file = fopen(input_filename, "r");

            if (input_file == NULL) {
                fprintf(stderr, " [%s] ERROR: fopen failed: %s\n", prog_name, strerror(errno));
                exit(EXIT_FAILURE);
            }

            char *new_output = runlengthEncode(prog_name, input_file, output, out_index, &input_size);

            output = new_output;

            if (fclose(input_file) == EOF) {
                fprintf(stderr, " [%s] ERROR: fclose failed: %s\n", prog_name, strerror(errno));
                exit(EXIT_FAILURE);
            }
        }
    }

    if (strlen(output) != 0) { // if there is an output, process it
        if (o_arg == NULL) { // No output file given -> output on stdout
            int ret_print;

            ret_print = printf("OUTPUT:\n%s\n", output);

            if (ret_print == -1) {
                fprintf(stderr, " [%s] ERROR: printf failed: %s\n", prog_name, strerror(errno));
                exit(EXIT_FAILURE);
            }
        } else { // Output file given -> output on given file
            char *outputfile = o_arg;

            FILE *output_file = fopen(outputfile, "w");

            if (output_file == NULL) {
                fprintf(stderr, " [%s] ERROR: fopen failed: %s\n", prog_name, strerror(errno));
                exit(EXIT_FAILURE);
            }

            fputs(output, output_file);

            if (fclose(output_file) == EOF) {
                fprintf(stderr, " [%s] ERROR: fclose failed: %s\n", prog_name, strerror(errno));
                exit(EXIT_FAILURE);
            }
        }
    }

    // Print read/written characters and the Compression ratio to stderr
    fprintf(stderr, "Read:\t\t\t%d characters\n", input_size);
    int output_size = (int) strlen(output);
    fprintf(stderr, "Written:\t\t%d characters\n", output_size);
    float comp_ratio = input_size > 0 ? ((float) output_size / (float) input_size) : 0;
    fprintf(stderr, "Compression ratio:\t%.1f%%\n", (comp_ratio * 100.0));

    free(output);
}
