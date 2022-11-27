/**
 * @file main.c
 * @author Nicolas Hrubec <e11722652@student.tuwien.ac.at>
 * @date 11.11.2021
 *
 * @brief myexpand application
 *
 * This program replaces tabs with spaces for multiple files.
 * Output to either stdout or a file.
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#define BUFFER_SIZE 2048 /**< Size of the buffer to read in file lines. */

/**
 * Program entry point.
 * This function implements the whole program.
 * It first reads in and stores given input parameters. Then it reads each given input file
 * line by line and replaces all tabs with spaces such that the next character is placed at
 * the next multiple of the tabstop distance.
 * The output is either printed to stdout if no output file is given or written to the output
 * file otherwise.
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCESS or EXIT_FAILURE.
 **/
int main (int argc, char *argv[]) {
    char buffer[BUFFER_SIZE]; /**< Buffer to hold a line of the file. */
    char c; /**< Used to temporarily store and then parse arguments. */
    FILE *input = NULL, *output = NULL; /**< Pointers for the input and output files. */
    char *outFilename = NULL, *inFilename = NULL; /**< File names for input and output. */
    int tabdist = 8; /**< Number of spaces that make up one tab. */
    char *myprog = argv[0];

    // parse arguments
    while ( (c = getopt(argc, argv, "t:o:")) != -1) {
        switch (c) {
            case 't':
                tabdist = strtol(optarg, NULL, 10);
                break;
            case 'o':
                outFilename = optarg;
                break;
            case '?':
                fprintf(stderr, "Usage: %s [-t tabstop] [-o outfile] [file...]", myprog);
                exit(EXIT_FAILURE);
        }
    }

    // if outFilename == NULL we will print to stdout
    // else output to file with filename
    if (outFilename != NULL) {
        if ((output = fopen(outFilename, "w")) == NULL) {
            fprintf(stderr, "[%s] ERROR: Opening output file failed", myprog);
            exit(EXIT_FAILURE);
        }
    }

    if (optind < argc) {
        while (optind < argc) {
            inFilename = argv[optind];

            // open input file
            if ((input = fopen(inFilename, "r+")) == NULL) {
                fprintf(stderr, "[%s] ERROR: Opening input file failed", myprog);
                exit(EXIT_FAILURE);
            }

            char curr; /**< Current char in line. */
            int next_char, write_i; /**< indices for next chars to be written  */
            // read in line by line
            // for each line replace all the tabs with spaces
            // then write the line either to provided file or print to stdout
            while ( fgets(buffer, sizeof(buffer), input) != NULL) {
                write_i = 0;
                for (int read_i = 0; read_i < strnlen(buffer, BUFFER_SIZE); read_i++) {
                    curr = buffer[read_i];

                    if (curr == '\t') { // tabstop
                        next_char = tabdist * ((write_i / tabdist) + 1); // find index of next char

                        // fill with spaces until index of next char
                        while (write_i < next_char) {
                            if (output != NULL) {
                                fputc(' ', output);
                            } else {
                                printf(" ");
                            }
                            printf(" ");
                            write_i++;
                        }
                    } else { // no tabstop
                        if (output != NULL) {
                            fputc(curr, output);
                        } else {
                            printf("%c", curr);
                        }
                        write_i++;
                    }
                }
            }

            fclose(input);

            optind++;
        }

    } else {
        fprintf(stderr, "[%s] ERROR: No input file provided", myprog);
        exit(EXIT_FAILURE);
    }

    // close output file
    if (output != NULL) {
        fclose(output);
    }

    return 0;
}
