/**
 * @file mydiff.c
 * @author Filip Coja <e12023141@student.tuwien.ac.at>
 * @date 07.11.2021
 *
 * @brief This program compares two files line by line
 *
 * @details The program takes in two files, e.g. file1.txt and file2.txt
 * Those two files are checked line by line. The differences are only checked until one of them reaches the \n sequence or ends.
 * So for example: '1234\n' and ’1234plew’ are identical to this program. If a line contains mismatches, they number of differences and
 * the line number are printed to stdout or to a file if specified with [-o outfile]
 *
 * The option [-i] removes the case sensitivity of the comparison.
 */

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <getopt.h>

/**
 * @brief This struct is used to manage all settings and arguments coming from the command line
 * @details Is only used once in main, the default settings are set there, be sure to give some starter settings.
 */
typedef struct {
    bool case_sensitive;
    bool to_stdout;
    char *output;
    char *file1;
    char *file2;
} options_t;

/**
 * @brief Global variable name to be used in the debugging functions
 * @details Is only set in handle_args()
 */
static char *prog_name;

/**
 * Debug function
 * @brief Prints program usage to command line.
 * @details Also exits the program correctly.
 */
static void print_usage(void) {
    fprintf(stderr, "Usage: mydiff [-i] [-o outfile] file1 file2\n");
    exit(EXIT_FAILURE);
}

/**
 * Debug function
 * @brief Prints program usage and an error message with a format to command line.
 * @details Also exits the program correctly.
 *
 * @param error_msg_format If you want to add a formatted string, you can add this here
 * @param error_msg_replacement Here you have to add the replacement then.
 */
static void print_error_usage(char *error_msg_format, char *error_msg_replacement) {
    if (error_msg_format == NULL || error_msg_replacement == NULL) {
        fprintf(stderr, "[%s] INTERNAL ERROR: print_error_usage() called with NULL pointer", prog_name);
        exit(EXIT_FAILURE);
    }

    char buffer[sizeof(error_msg_format)];
    sprintf(buffer, error_msg_format, error_msg_replacement);
    fprintf(stderr, "[%s] ERROR: %s", prog_name, buffer);
    print_usage();
}

/**
 * Handle arguments
 * @brief Handles all arguments from the command line and responds accordingly.
 * @details To start off, argc has to be greater then one to even be correct in the start. After that getopt()
 * is used to obtain the flags and args, opterr is set to 0 so getopt() doesnt print any error messages. This is done so
 * we can print our own specialized error messages.
 *
 * In this function prog_name is set which is used in the debug functions.
 *
 * @param argc This is just argc from main()
 * @param argv This is just argv from main()
 * @param options The options struct where the settings are set in this function.
 */
static void handle_args(int argc, char **argv, options_t *options) {
    /** Exit if too few args */
    if (argc <= 1) print_usage();
    prog_name = argv[0];

    /** Parse all command line options and arguments */
    int c;
    opterr = 0;
    while ((c = getopt(argc, argv, "io:")) != -1) {
        switch (c) {
            case 'i':
                options->case_sensitive = false;
                break;
            case 'o':
                options->output = optarg;
                options->to_stdout = false;
                break;
            case '?':
                if (optopt == 'o') {
                    print_error_usage("Option -o requires an argument. \n", "");
                } else if (isprint(optopt)) {
                    fprintf(stderr, "[%s] ERROR: Unknown option `-%c'. \n", prog_name, optopt);
                    print_usage();
                }
            default:
                print_error_usage("Unknown options received. \n", "");
        }
    }

    options->file1 = argv[optind];
    options->file2 = argv[optind + 1];
    if (options->file1 == NULL || options->file2 == NULL) {
        print_error_usage("Not received enough file arguments. \n", "");
    }
}

/**
 * @brief Checks two files for differences and writes it to *output.
 * @details The function uses getline() to get each line, we set the buffers to NULL so getline() does the malloc() for us
 * but it has to be free'd later on with free(). The function stops if one file ends, and only compares lines until
 * one of them reaches the \n flag or the file is ended.
 *
 * @param file1 The first file to be compared.
 * @param file2 The second file to be compared.
 * @param case_sensitive If false, uppercase and lowercase letters are treated as the same characters
 * @param output The output stream.
 */
static void diff(char *file1, char *file2, bool case_sensitive, FILE *output) {
    /** File handling */
    FILE *f1 = fopen(file1, "r");
    FILE *f2 = fopen(file2, "r");
    if (f1 == NULL) {
        print_error_usage("File `%s` couldn't be opened. \n", file1);
    } else if (f2 == NULL) {
        print_error_usage("File `%s` couldn't be opened. \n", file2);
    }

    /** Creating buffers */
    size_t buff_size_1 = 0;
    size_t buff_size_2 = 0;
    char *buffer_1 = NULL;
    char *buffer_2 = NULL;

    u_int64_t line = 1;
    /** Comparison loop which checks for differences */
    while (true) {
        /** Get each line from each file */
        size_t read1 = getline(&buffer_1, &buff_size_1, f1);
        size_t read2 = getline(&buffer_2, &buff_size_2, f2);
        if (read1 == -1 || read2 == -1) break;

        /** Get minimum of read characters */
        u_int64_t length;
        if (read1 >= read2) {
            length = read2;
        } else {
            length = read1;
        }
        length /= sizeof(char);

        /** Compare each character from both lines */
        u_int64_t differences = 0;
        for (int i = 0; i < length; ++i) {
            if (!case_sensitive) {
                buffer_1[i] = (char) tolower(buffer_1[i]);
                buffer_2[i] = (char) tolower(buffer_2[i]);
            }
            if (buffer_1[i] != buffer_2[i] && buffer_1[i] != '\n' && buffer_2[i] != '\n') {
                differences++;
            }
        }

        if (differences > 0) fprintf(output, "Line: %lu, characters: %lu\n", line, differences);
        line++;
    }

    /** Free all buffers and close file streams */
    free(buffer_1);
    free(buffer_2);
    fclose(f1);
    fclose(f2);
}

/**
 * @brief Main entry point
 * @details Main function. Options are created and default settings are set.
 *
 * @param argc
 * @param argv
 * @return exit code
 */
int main(int argc, char **argv) {
    /** Default settings */
    options_t options;
    options.case_sensitive = true;
    options.to_stdout = true;

    /**  Handle args */
    handle_args(argc, argv, &options);

    /** Try setting up output file */
    FILE *output;
    if (options.to_stdout) {
        output = stdout;
    } else {
        output = fopen(options.output, "w");
        if (output == NULL) {
            print_error_usage("File `%s` couldn't be opened. \n", options.output);
        }
    }

    /** Check for differences and write to output */
    diff(options.file1, options.file2, options.case_sensitive, output);

    /** Close File stream if it wasn't stdin */
    if (!options.to_stdout) fclose(output);

    return EXIT_SUCCESS;
}
