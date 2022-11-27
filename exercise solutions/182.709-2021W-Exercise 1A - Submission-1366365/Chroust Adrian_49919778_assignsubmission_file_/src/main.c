/**
 * @file main.c
 * @author Adrian Chroust <e12023146@student.tuwien.ac.at>
 * @date 9.11.2021
 * @brief Main program module.
 * @details Entrance point for a command line program which subsequently
 *          reads lines in files and checks if the are palindromes.
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>
#include "palindrome.h"

/**
 * @brief A flag for signaling an error concerning a file.
 * @details The flag is used as a return value for functions that handle files.
 */
#define FILE_ERROR 1

/**
 * @brief The program name detected by the main function.
 * @details Required for printing out the proper usage of the program through the usage function.
 * */
static char *myprog;

/**
 * @brief This function prints the expected input parameters of the program to stderr.
 * @details The function is usually called when a call with unexpected inputs is detected.
 */
static void usage(void) {
    fprintf(stderr, "[%s] Usage: %s [-s] [-i] [-o outfile] [file...]\n", myprog, myprog);
    exit(EXIT_FAILURE);
}

/**
 * @brief The function checks if both the input file and the output file were opened.
 * @details If the files could not be opened the function prints error messages to stderr.
 * @param input The input file, can be stdin.
 * @param output The output file, can be stdout.
 * @param input_filename The name of the input file, required for printing an error message.
 * @param output_filename The name of the output file, required for printing an error message.
 * @return The FILE_ERROR flag if one of the files does not have a valid pointer, 0 otherwise.
 */
static int verify_files(FILE *input, FILE *output, char *input_filename, char *output_filename) {
    bool error = false;

    // Checks, if the input file points somewhere. If not, an error message is printed.
    if (input == NULL) {
        error = true;
        fprintf(stderr, "[%s] Error: could not open input file %s\n", myprog, input_filename);
    }
    // Checks, if the output file points somewhere. If not, an error message is printed.
    if (output == NULL) {
        error = true;
        fprintf(stderr, "[%s] Error: could not open output file %s\n", myprog, output_filename);
    }

    // If one of the files point to NULL, the error code is returned.
    return error ? FILE_ERROR : 0;
}

/**
 * @brief This function closes the input and the output file. In case of the files being stdin or stdout the files are not closed.
 * @brief If a file close is unsuccessful an error message is printed to stderr for the file that could not be closed.
 * @param input The input file, can be stdin.
 * @param output The output file, can be stdout.
 * @param input_filename The name of the input file, required for printing an error message.
 * @param output_filename The name of the output file, required for printing an error message.
 * @return The FILE_ERROR flag if one of the files could not be closed, 0 otherwise.
 */
static int close_files(FILE *input, FILE *output, char *input_filename, char *output_filename) {
    bool error = false;

    // Checks, if the input file points somewhere other than stdin.
    if ((input != NULL) && (input != stdin)) {
        // Prints an error message, if the file cannot be closed.
        if (fclose(input) != 0) {
            error = true;
            fprintf(stderr, "[%s] Error: could not close input file %s\n", myprog, input_filename);
        }
    }
    // Checks, if the input file points somewhere other than stdout.
    if ((output != NULL) && (output != stdout)) {
        // Prints an error message, if the file cannot be closed.
        if (fclose(output) != 0) {
            error = true;
            fprintf(stderr, "[%s] Error: could not close output file %s\n", myprog, output_filename);
        }
    }

    return error ? FILE_ERROR : 0;
}

/**
 * @brief Program entry point.
 * @details This function reads and decodes the arguments provided by the user and passes input
 *          and output files for the palindromes to the print_palindromes function.
 * @param argc The argument count of argv.
 * @param argv The arguments that the user inputted into the program. For more information please refer to the usage function.
 * @return The flag EXIT_SUCCESS on successful read and write of the files, EXIT_FAILURE otherwise.
 */
int main(int argc, char *argv[]) {
    // Sets the global program name.
    myprog = argv[0];

    // Refer to usage, if the program has more than for arguments.
    if (argc > 4) usage();

    // Default values for input parameters
    bool ignore_whitespace = false;
    bool case_insensitive = false;

    // Default output file
    char *outfilename = "stdout";
    FILE *outfile = stdout;

    int c;

    // Reads all input parameter options
    while ((c = getopt(argc, argv, "sio:")) != -1) {
        switch (c) {
            case 's': // Option for ignoring whitespace
                if (optarg != 0) usage();
                ignore_whitespace = true;
                break;
            case 'i': // Option for ignoring case of characters.
                if (optarg != 0) usage();
                case_insensitive = true;
                break;
            case 'o': // Option for setting a custom output file.
                if (optarg == 0) usage();

                // Opens a custom output file.
                outfilename = optarg;
                outfile = fopen(outfilename, "w");
                break;
            default: // Refers to usage, if unknown option is given in input.
                usage();
                break;
        }
    }

    // Default input file
    char *infilename = "stdin";
    FILE *infile = stdin;

    // Checks if a custom input file is given in input.
    if (optind < argc) {
        // Refers to usage, if another argument is given after input file.
        if (optind + 1 < argc) usage();

        // Opens a custom input file.
        infilename = argv[optind];
        infile = fopen(infilename, "r");
    }

    // Checks that the files are both open.
    int verify_status = verify_files(infile, outfile, infilename, outfilename);

    int find_status = 0;

    // Finds palindromes, if both files are open.
    if (verify_status == 0) {
        find_status = find_palindromes(infile, outfile, case_insensitive, ignore_whitespace);

        // Checks if the function find_palindromes emitted any errors.
        if (find_status == FIND_PALINDROMES_OUT_OF_MEMORY_ERROR) {
            fprintf(stderr, "[%s] Error: could not allocate enough memory while reading palindromes.\n", myprog);
        } else if (find_status == FIND_PALINDROMES_WRITE_ERROR) {
            fprintf(stderr, "[%s] Error: could not write to output file %s\n", myprog, outfilename);
        }
    }

    // Tries to close both files.
    int close_status = close_files(infile, outfile, infilename, outfilename);

    // If files could not be opened, written or closed properly, returns the flag EXIT_FAILURE, EXIT_SUCCESS otherwise.
    return (verify_status | find_status | close_status) == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
