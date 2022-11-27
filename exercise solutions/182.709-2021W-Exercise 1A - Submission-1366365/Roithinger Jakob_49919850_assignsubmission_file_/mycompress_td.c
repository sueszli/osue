/**
 * @file mycompress.c
 * @author Jakob Roithinger 52009269
 * @date 9.11.2021
 *
 * @brief takes an input of ascii characters and uses a simple algorithm to compress it
 * @details This program takes input from text files, if they are given as arguments, else it reads from
 * stdin. If an output file is specified with -o, then the compressed output is written to the given file.
 * Otherwise the output is printed to stdout. Listing multiple inputs will compress them in series.
 **/

#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char *filename;  /// name of the file

/**
 * @details Takes a character and the amount of times in a row it appeared, compresses it into the format ("%c%d", c, count)
 * and outputs it to the given source. Uses filename to print to stderr. global variables: filename
 * @param c last received char from an input source
 * @param count amount of 'c' in a row that appeared in the input
 * @param output_file the output stream to which the compressed version of the char series should be written to
 * @return Returns the amount of characters written
 */
static int add_to_output(char c, int count, FILE *output_file) {
    int charsPrinted = 1;                                ///< if nothing fails, c is always output and has length 1
    char count_as_str[10];                               ///< will be used to store count as str in sprintf,  10 is Length of INT_MAX as decimal value
    charsPrinted += sprintf(count_as_str, "%d", count);  ///< sprintf returns the amount of characters written into param_0

    if (fputc(c, output_file) == EOF) {
        fprintf(stderr, "%s: fputc failed: %s\n", filename, strerror(errno));
        fclose(output_file);
        exit(EXIT_FAILURE);
    }

    if (fputs(count_as_str, output_file) == EOF) {
        fprintf(stderr, "%s: fputs failed: %s\n", filename, strerror(errno));
        fclose(output_file);
        exit(EXIT_FAILURE);
    }

    return charsPrinted;
}

/**
 * @details Takes a character and adds it to the compressed version of an input stream.
 * @param c last received char from an input source
 * @param output_file the file to which the compressed text should be written to, NULL i
 * @return Returns the amount of characters written.
 */
static int build_compressed(char c, FILE *output_file) {
    static char prevC = '\0'; /* stores previous char, '/0' used as guard for first call */
    static int streak = 1;    /* keeps track of current char streak */
    if (prevC == '\0') {      /* only applies at first call */
        prevC = c;
    } else if (prevC == c) { /* increase streak if current char is the same as previous */
        streak += 1;
    } else {
        /* if current char is different, output compress version of previous char series */
        int charsPrinted = add_to_output(prevC, streak, output_file);
        streak = 1;
        prevC = c;
        return charsPrinted;
    }
    return 0;
}

/**
 * @brief main function of mycompress_td
 * @details This is the main function of supervisor. As argmunments it taks -o followed by the path of the output
 * file. Every further argument is interpreted as a path to an input file, which is to be compress.
 * global variables: filename
 * @param argc amount of arguments from terminal
 * @param argv arguments from terminal
 * @return EXIT_SUCCESS on succes, EXIT_FAILURE on failure
 **/
int main(int argc, char *argv[]) {
    filename = argv[0];
    char *output_path = NULL;  ///< contains string to output path
    char optionChar;           ///< used to temporarily store option chars

    /// standard use of getopt
    while ((optionChar = getopt(argc, argv, "o:")) != -1) {
        switch (optionChar) {
            case 'o':
                /// output to file instead of stdout
                output_path = optarg;
                break;
            case '?':
                fprintf(stderr, "Usage: %s [-o file] file...\n", filename);
                exit(EXIT_FAILURE);
            default:
                assert(0);
        }
    }

    /// attempt to create or rewrite output file, stdout by default if no output argument given
    FILE *output_file = stdout;
    if (output_path) {
        if ((output_file = fopen(output_path, "w")) == NULL) {
            fprintf(stderr, "%s: Failed to open output file with fopen: %s\n", filename, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    int readCharsCount = 0;        ///< stores read chars amount
    int compressedCharsCount = 0;  ///< stores written chars amount
    char c;                        ///< stores current char
    if (optind < argc) {           ///< optind < argc is only true when there are non positional arguments given
        for (; optind < argc; optind++) {
            FILE *input = fopen(argv[optind], "r");
            if (input == NULL) {
                fprintf(stderr, "%s: Failed to open input file \"%s\" with fopen: %s\n", filename, argv[optind], strerror(errno));
                continue;
            } else {
                while ((c = fgetc(input)) != EOF) {
                    readCharsCount++;
                    compressedCharsCount += build_compressed(c, output_file);
                }
            }
            if (fclose(input)) {
                fprintf(stderr, "%s: Failed to close input file \"%s\" with fclose: %s\n", filename, argv[optind], strerror(errno));
            }
        }
    } else {
        /// if no input files are given, read from stdin
        printf("Awaiting input:\n");
        while (1) {
            c = fgetc(stdin);
            if (c == EOF) {
                /// EOF could be error with fgetc, or the charater read
                if (ferror(stdin)) {
                    /// EOF ist error with fgetc
                    fprintf(stderr, "%s: Error: Could not read file: %s", filename, strerror(errno));
                }
                /// Error or EOF read, either way break
                break;
            }
            readCharsCount++;
            compressedCharsCount += build_compressed(c, output_file);
        }
    }

    /// add ongoing char series to output, EOF as a char that cannot yet be taken as an input
    compressedCharsCount += build_compressed(EOF, output_file);

    /// if nothing is read, nothing is compressed, ratio of 0/0 not defined
    if (readCharsCount) {
        fprintf(stderr, "\nRead:\t%d characters\nWritten: %d characters\nCompression ratio: %.1f%%\n", readCharsCount, compressedCharsCount, 100.0 * compressedCharsCount / readCharsCount);
    } else {
        fprintf(stderr, "\nRead:\t0 characters\nWritten: 0 characters\nCompression ratio: NaN");
    }
    fclose(output_file);
    exit(EXIT_SUCCESS);
}
