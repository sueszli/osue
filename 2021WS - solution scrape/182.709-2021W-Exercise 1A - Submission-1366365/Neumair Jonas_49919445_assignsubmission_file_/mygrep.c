/**
 * @file mygrep.c
 * @author Jonas Neumair <e11911064@student.tuwien.ac.at>
 * @date 01.11.2021
 *
 * @brief Main program module.
 * 
 * This program reads an input stream (stdin or given files) line by line and checks if
 * a line contains a given keyword. If so, the line is written to
 * the given output stream, either stdout or the given output file.
 **/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

static char *pgm_name; /* The program name. */

/**
 * Mandatory usage function.
 * @brief This function writes helpful usage information about the program to stderr.
 * @details global variables: pgm_name
 */
static void usage(void) {
	(void) fprintf(stderr, "USAGE: %s [-i] [-o outfile] keyword [file...]\n", pgm_name);
	exit(EXIT_FAILURE);
}

/**
 * Error function.
 * @brief This function writes error messages to stderr.
 * @details Function prints message and exits program.
 */
static void error(char *msg) {
	(void) fprintf(stderr, "Error %s: %s\n", pgm_name, msg);
	exit(EXIT_FAILURE);
}

/**
 * Line reading function.
 * @brief This function reads every line of the provided input stream.
 * @details Function reads line by line, checking for keyword.
 * If case_sensitive is false, then the input line is transformed to
 * lower case. A copy of the input line is made to be printed after
 * reading. At last we close the provided filestream, if not stdin.
 * @param keyword The keyword to be searched.
 * @param file The provided input stream.
 * @param case_sensitive States, if the search is case sensiive.
 */
void read_lines(char *keyword, FILE *file, bool case_sensitive, FILE *out) {
    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;
    while ((linelen = getline(&line, &linecap, file)) > 0)
    {
        if (linelen < 0) error("error while reading stdin.");

        if (case_sensitive) {
            if (strstr(line, keyword) != NULL)
                fwrite(line, linelen, 1, out);
        } else {
            char copy[linelen];
            for (size_t i = 0; i < linelen; i++)
            {
                copy[i] = line[i];
                line[i] = tolower(line[i]);
            }
            if (strstr(line, keyword) != NULL)
                fwrite(copy, linelen, 1, out);
        }
    }
    if (file != stdin) {
        if (fclose(file) == EOF) error("Couldn't close file.");
    }
}

/**
 * Program entry point.
 * @brief The program starts here.
 * @details
 * The program starts to take care about parameters.
 * -i if case doesn't matter
 * -o to provide an output file
 * and after the parameters, you can add any number of files.
 * After that, we check if we provide an outputfile to start opening the filestream.
 * Then we iterate over the provided files and read them line by line to print lines
 * containing the key, done by calling read_lines() function above.
 * If no file is provided, we just read the stdin.
 * At last we close the output stream, if opened before.
 * global variables: pgm_name
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS.
 */
int main(int argc, char **argv)
{
    pgm_name = argv[0];
    bool case_sensitive = true;
    char *outputfilename = NULL;
    int opt;
    while ((opt = getopt(argc, argv, "io:")) != -1)
    {
        switch (opt)
        {
            case 'i':
                if (!case_sensitive) usage();
                case_sensitive = false;
                break;
            
            case 'o':
                if (outputfilename != NULL) usage();
                outputfilename = optarg;
                break;
            
            default:
                break;
        }
    }

    int file_count = (argc-optind)-1;
    if (file_count < 0)
    {
        usage();
    }

    char *keyword = argv[optind];
    FILE *file = stdin;
    FILE *output = stdout;

    if (outputfilename != NULL) {
        output = fopen(outputfilename, "w+");
        if (output == NULL) error("Couldn't open outputfile.");
    }

    char *filename;
    if (file_count > 0)
    {
        for (size_t i = 1; i <= file_count; i++)
        {
            filename = argv[optind+i];
            file = fopen(filename, "r");
            if (file == NULL) error("Couldn't open file.");
            read_lines(keyword, file, case_sensitive, output);
        }
    } else {
        read_lines(keyword, file, case_sensitive, output);
    }
    if (output != stdout) {
        if (fclose(output) == EOF) error("Couldn't close file.");
    }

    return EXIT_SUCCESS;
}
