#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>

/**
 * @author Mathias Möller, 12019833
 * @date 10.11.2021
 * @brief Variation of unix-command expand, replaces tabs in files with spaces
 */

/**
 * Prints the correct usage of the program and exits with error code EXIT_FAILURE
 */
#define USAGE() \
    fprintf(stdout, "USAGE: %s [-t tab-diatance] [-o outfile] [infile...]", argv[0]); \
    exit(EXIT_FAILURE);

/**
 * reads characters from input_file and writes them to output_file, while replacing tabs-characters with spaces
 * @param input_file a FILE object with read-permission
 * @param output_file a FILE object with write-permission
 * @param tab_distance the required number of space-characters to replace (a full) tab-character
 */
static void readAndOutputFile(FILE *input_file, FILE *output_file, int tab_distance) {
    char input_char; // variable for the read character
    int position = 0; // position of the read character within a line
    while ((input_char = fgetc(input_file)) != EOF) { // read a single character
        int char_count = 1;  // for any normal character, it should be written exactly once

        // only tab-characters are replaced by a specific amount of spaces
        if (input_char == '\t') {
            char_count = (tab_distance * ((position / tab_distance) + 1)) - position;
            input_char = ' ';
        }

        for (int i = 0; i < char_count; ++i) { // write the input_char char_count many times to the output
            // try to write to the character to the output once:
            if (fputc(input_char, output_file) == EOF) {
                // some sort of error occurred when
                fclose(input_file);
                fclose(output_file);
                fprintf(stderr,
                        "Error when trying to write to the specified output (stdout if none was specified): %s\n",
                        strerror(errno));
                exit(EXIT_FAILURE);
            }
            position++;
        }

        if (input_char == '\n') {
            position = 0;
        }
    }
}

int main(int argc, char **argv) {
    int tab_distance = 8;
    FILE *output_file = stdout;
    char opt;
    while ((opt = getopt(argc, argv, "t:o:")) != -1) {
        switch (opt) {
            case 't': {
                char *end;
                tab_distance = (int) strtol(optarg, &end, 10);
                if (*end != '\0') {
                    // error when converting optarg to int
                    fprintf(stderr, "unable to convert tab-distance option to integer!\n");
                    USAGE();
                }
                break;
            }
            case 'o':
                output_file = fopen(optarg, "w");
                break;
            default:
            USAGE();
        }
    }

    if (tab_distance < 1) {
        // not a valid tab-distance
        fprintf(stderr, "invalid tab-distance: %d\n", tab_distance);
        USAGE();
    }

    if (output_file == NULL) {
        fprintf(stderr, "unable to open output file: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if(optind == argc) { // no input file given, i.e. stdin should be the input
        readAndOutputFile(stdin, output_file, tab_distance);
    }

    for (int i = optind; i < argc; i++) { // process every input file separately and serially
        FILE *input_file = fopen(argv[i], "r");
        if (input_file == NULL) {
            fprintf(stderr, "unable to open an input file: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        readAndOutputFile(input_file, output_file, tab_distance);
        if (fclose(input_file) == EOF) {
            fprintf(stderr, "Error while closing an input file: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    if (fclose(output_file) == EOF) {
        fprintf(stderr, "Error while closing the output file: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}