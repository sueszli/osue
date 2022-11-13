/**
 * @file mydiff.c
 * @author Alexander Gschnitzer (01652750) <e1652750@student.tuwien.ac.at>
 * @date 19.10.2021
 *
 * @brief Compares content of two files and shows their differences.
 * @details Reads in two files and compares their characters line by line. Provides a case-insensitive mode which can be set with the option -i.
 * It writes the differing lines to stdout except the option -o is set, then it writes the result in the specified output file.
 * Checks both files for differing lines first and only then compares the ones with differing lines - character by character.
 * Exits the program with EXIT_FAILURE in case the opening, closing of a file failed.
 */

#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>

/**
 * @brief First command-line parameter of argv array.
 */
const char *prog_name;

/**
 * @brief Displays error message.
 * @details Prints error message to stderr and exists program with EXIT_FAILURE. Uses global variable prog_name.
 * @param message custom error message.
 */
static void error(const char *message) {
    fprintf(stderr, "[%s] ERROR %s: %s\n", prog_name, message, strerror(errno));
    exit(EXIT_FAILURE);
}

/**
 * @brief Displays usage of program.
 * @details Prints short usage description to stdout and exists the program with EXIT_FAILURE.
 */
static void usage(void) {
    fprintf(stdout, "Usage: %s [-i] [-o outfile] file1 file2\n", prog_name);
    exit(EXIT_FAILURE);
}

/**
 * @brief Reads file from file system.
 * @details Attempt to open file with given path. Exists the program with EXIT_FAILURE in case opening failed - by calling error function.
 * @param path filename of file that will be opened.
 * @return Pointer to file on successful open, EXIT_FAILURE on failure.
 */
static FILE *openFile(const char *path, const char *mode) {
    FILE *file;

    if ((file = fopen(path, mode)) == NULL) {
        error("Opening of file failed");
    }

    return file;
}

/**
 * @brief Closes file in order to free resources.
 * @details Attempts to close given file. Exists the program with EXIT_FAILURE in case closing failed - by calling error function.
 * @param file previously opened file that should be closed.
 */
static void closeFile(FILE *file) {
    if (fclose(file) == EOF) {
        error("ERROR: fclose failed:");
    }
}

/**
 * @brief Compares two strings by each character.
 * @details Iterates n times over both strings, whereby n is the length specified by linecap.
 * Uses either strncasecmp or strncmp function to compare chars ('compare' function) - is set by the main function.
 * @param line1 first argument as a string.
 * @param line2 second argument as a string.
 * @param linecap length of shorter line.
 * @param compare pointer to either strncasecmp or strncmp.
 * @return number of differing chars.
 */
static int compareCharacters(const char *line1, const char *line2, size_t linecap,
                             int (*compare)(const char *, const char *, size_t)) {
    int result = 0;
    for (int i = 0; i < linecap; i++) {
        result += compare(&line1[i], &line2[i], 1) == 0 ? 0 : 1;
    }

    return result;
}

/**
 * @brief Main entry point of mydiff.
 * @details Reads options and positional arguments to propagate files for line by line comparison.
 * Sets global variable prog_name and outfile.
 * Prints line number and differing characters in case two lines differ - either to stdout or output file, if specified.
 * @param argc Number of command-line parameters in argv.
 * @param argv Array of command-line parameters, argc elements long.
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on failure.
 */
int main(int argc, char **argv) {
    // set program name
    prog_name = argv[0];

    // opt_i indicates the number of "-i" occurrences
    // opt_o indicates the number of "-o" occurrences
    int c, opt_i = 0, opt_o = 0;
    const char *outfile = NULL;
    while ((c = getopt(argc, argv, "io:")) != -1) {
        switch (c) {
            case 'i':
                opt_i++;
                break;
            case 'o':
                outfile = optarg;
                opt_o++;
                break;
            case '?':
                usage();
                break;
            default:
                // not reachable
                assert(0);
        }
    }

    // check number of positional arguments and option occurrences
    if ((argc - optind) != 2 || opt_i > 1 || opt_o > 1) {
        usage();
    }

    // use case-insensitive function if option -i is set (opt_i == 1), case-sensitive function otherwise (opt_i == 0)
    int (*compare)(const char *, const char *, size_t) = opt_i ? &strncasecmp : &strncmp;

    // read input files with positional arguments as filenames
    FILE *file1 = openFile(argv[optind], "r"), *file2 = openFile(argv[optind + 1], "r");

    // use output file from option argument if option -o is set (opt_o == 1), stdout otherwise (opt_o == 0)
    // opens file in append mode to prevent possible overwrite of file content
    FILE *output = opt_o ? openFile(outfile, "a") : stdout;

    char *line1 = NULL, *line2 = NULL;
    size_t linecap1 = 0;
    ssize_t read1;

    // line count
    int nl = 1;
    while ((read1 = getline(&line1, &linecap1, file1)) != -1) {
        size_t linecap2 = 0;
        ssize_t read2;

        // exit loop if there are no more lines in file2
        if ((read2 = getline(&line2, &linecap2, file2)) == -1) {
            break;
        }

        // set length of shorter line
        ssize_t *linecap = read1 <= read2 ? &read1 : &read2;

        // compare lines and decrement linecap due to last char in line is '\n'
        int result = compare(line1, line2, (size_t) (*linecap - 1));

        // filter lines with differing chars
        if (result != 0) {
            // compare lines char by char to get number of differing chars
            int diff_chars = compareCharacters(line1, line2, (size_t) (*linecap - 1), compare);

            // write either to file or stdout, specified by output
            fprintf(output, "Line: %d, characters: %d\n", nl, diff_chars);
        }

        // increment line count
        nl++;
    }

    // free up dynamically allocated memory
    free(line1);
    free(line2);

    // close files
    closeFile(file1);
    closeFile(file2);
    closeFile(output);

    exit(EXIT_SUCCESS);
}
