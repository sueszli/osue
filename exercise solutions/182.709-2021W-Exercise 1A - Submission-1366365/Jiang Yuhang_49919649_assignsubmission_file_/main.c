/**
 * @file main.c
 * @author Yuhang Jiang <e12019854@student.tuwien.ac.at>
 * @brief Start point of the program.
 * @details Takes in the command flags and arguments and sets the input files and the output file.
 * It then read each line and outputs the line to the output file if a substring matches the keyword
 * (can be case-insensitive).
 * @date 26.10.2021
 */


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include "compare.h"

char *prog_name;    /**< The program name */

static void print_usage(void);

/**
 * @brief Takes in the passed arguments and parses them. Depending on the input values the input is either
 * stdin or one or multiple files. The output is either stdout or a file.
 * @param argc Argument count
 * @param argv Argument vector
 * @return EXIT_SUCCESS if the program runs as expected. EXIT_FAILURE if the input is invalid or there is an error.
 */
int main(int argc, char *argv[]) {
    prog_name = argv[0];

    int c;
    int case_insensitive = 0;
    char *outfile = NULL;

    while ((c = getopt(argc, argv, "io:")) != -1) {
        switch (c) {
            case 'i': case_insensitive = 1;
                break;
            case 'o': outfile = optarg;
                break;
            case '?': print_usage();
                break;
            default:
                assert(0);
        }
    }

    // first argument
    if (argv[optind] == NULL) {
        print_usage();
        exit(EXIT_FAILURE);
    }

    int read_from_stdin = 0;

    // no files specified -> read from stdin
    if (argv[optind + 1] == NULL) {
        read_from_stdin = 1;
    }

    char *keyword = argv[optind];
    int nr_of_files = argc - optind - 1;
    char *files[nr_of_files];
    int index = 0;

    for (int i = optind + 1; i < argc; ++i) {
        files[index] = argv[i];
        index++;
    }

    FILE *f_outfile;

    if (outfile != NULL) {
        f_outfile = fopen(outfile, "w");
    } else {
        f_outfile = stdout;
    }

    if (f_outfile == NULL) {
        fprintf(stderr,  "[%s] ERROR: fopen failed: %s\n", prog_name, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (read_from_stdin == 0) {
        for (int i = 0; i < nr_of_files; ++i) {
            FILE *file = fopen(files[i], "r");

            if (file == NULL) {
                fprintf(stderr, "[%s] ERROR: fopen failed: %s\n", prog_name, strerror(errno));
                exit(EXIT_FAILURE);
            }

            compare_lines(file, f_outfile, keyword, case_insensitive);

            fclose(file);
        }
    } else {
        FILE *file = stdin;

        compare_lines(file, f_outfile, keyword, case_insensitive);

        fclose(file);
    }

    fclose(f_outfile);

    return EXIT_SUCCESS;
}

/**
 * @brief prints usage information
 */
static void print_usage(void) {
    fprintf(stderr, "Usage: %s [-i] [-o outfile] keyword [file...]\n", prog_name);
}
