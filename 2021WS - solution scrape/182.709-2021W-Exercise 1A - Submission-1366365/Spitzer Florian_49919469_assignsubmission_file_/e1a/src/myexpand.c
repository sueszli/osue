/**
 * @file
 * @author Florian Spitzer, Student ID 00507913
 * @date 2021-10-27
 *
 * @brief Main program module.
 * 
 * This program replaces tabs by spaces. The input is either read
 * from files specified at the command line or from stdin.
 * The output is either written to a file specified at the command
 * line or to stdout.
 * See the function **usage** for details.
 **/

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include <unistd.h>
#include <getopt.h>

/**
 * The default tab stop size.
 */
#define DEFAULT_TABSTOP (8)

/**
 * The name of this program. Will be set at the beginning of main() and shall
 * stay constant then.
 */
char *program_name = 0;

/**
 * @brief Function that does the actual work.
 * It copies the stream **in** into the stream **out**,
 * while replacing spaces by tabs.
 * This is done such that the next character is placed at the next multiple of 
 * the tabstop distance within the line.
 * The argument **tabstop** determines how big a tabstop shall be in the output.
 * @details This function works by reading the input stream character by
 * character, while maintaining a counter that indicates the position in the
 * current line. If a tab character is encountered, this counter is used
 * to determine the number of spaces to write into the output.
 * @param in The input file.
 * @param out The output file.
 * @param tabstop An integer describing the tabstop size.
 */
static void expand(FILE *in, FILE *out, int tabstop)
{
    int c;
    int count = 0;
    while ((c = fgetc(in)) != EOF) {
        switch (c) {
        case '\n':
            count = 0;
            fputc('\n', out);
            break;
        case '\t':
            do {
                fputc(' ', out);
                count++;
            } while (count % tabstop != 0);
            break;
        default:
            fputc(c, out);
            count++;
            break;
        }
    }
}

/**
 * @brief This function writes helpful usage information about the program to
 * stderr.
 * @details Uses the global variable **program_name** to give the name of
 * this program.
 * @param program_name The program name (for display).
 */
static void usage(void)
{
    fprintf(stderr,
            "SYNOPSIS:\n"
            "    %s [-t tabstop] [-o outfile] [file...]\n"
            "\n"
            "tabstop ... integer between 0 and %d (both exclusive)\n"
            "outfile ... path of writeable file\n",
            program_name, INT_MAX);
}

/**
 * @brief This function tries to close a file stream, and writes an error
 * message to stderr on failure.
 * @details Uses the global variable **program_name** to give the name of
 * this program.
 * @param fp The file stream to close
 */
static void close_file(FILE *fp)
{
    if (fclose(fp) != 0)
        fprintf(stderr, "%s: could not close file\n", program_name);
}

/**
 * @brief The program starts here. This function parses the command line
 * arguments and then uses the function **expand** to do the actual work.
 * @details Parsing of the command line is done by getopt(3). 
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS or EXIT_FAILURE.
 */
int main (int argc, char **argv)
{
    FILE *outfile = stdout;
    int tabstop = DEFAULT_TABSTOP;
    int c;
    program_name = argv[0];
    
    opterr = 0;
    while ((c = getopt(argc, argv, "t:o:")) != -1)
        switch (c) {
        case 't':
            {
                char *end;
                long t = strtol(optarg, &end, 10);
                if (t <= 0 || t >= INT_MAX || end == optarg || *end != 0) {
                    usage();
                    exit(EXIT_FAILURE);
                }
                tabstop = t;
            }
            break;
        case 'o':
            if ((outfile = fopen(optarg, "w")) == NULL) {
                fprintf(stderr, "%s: could not open file %s for writing\n",
                        argv[0], optarg);
                exit(EXIT_FAILURE);
            }
            break;
        default:
            usage();
            exit(EXIT_FAILURE);
            break;
        }

    if (optind == argc)
        expand(stdin, outfile, tabstop);
    else for (int i = optind; i < argc; i++) {
        FILE *fp = fopen(argv[i], "r"); 
        if (fp == NULL) {
            fprintf(stderr, "%s: could not open file %s for reading\n",
                    argv[0], argv[i]);
            if (outfile != stdout) {
                close_file(outfile);
                exit(EXIT_FAILURE);
            }
        }
        expand(fp, outfile, tabstop);
        close_file(fp);
    }

    if (outfile != stdout)
        close_file(outfile);
    return EXIT_SUCCESS;
}
