/**
 * @file mydiff.c
 * @author Filip Lovre <e1526630@student.tuwien.ac.at>
 * @date 12.11.2021
 *
 * @brief Main program module.
 * 
 * This program implements a variation of the UNIX-command "diff".
 **/

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>

static char *prog_name; /**< The program name. */

/**
 * Mandatory usage function.
 * @brief This function writes helpful usage information about the program to stderr.
 * @details global variables: prog_name
 */
static void usage()
{
    fprintf(stderr, "Usage: %s [-i] [-o outfile] file1 file2\n", prog_name);
    exit(EXIT_FAILURE);
}

/**
 * Program entry point.
 * @brief Takes care of argument handling. Reads in two files, compares them line-by-line and prints the difference into an outputstream.
 * @details If argument -i is given, the comparison is case-insensitive. If argument -o [outputfile] is given, the program writes the result to an outputfile, rather than to stdout.
 * global variables: prog_name
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS.
 */
int main(int argc, char *argv[])
{
    prog_name = argv[0];

    int opt;
    int opt_i = 0;
    int opt_o = 0;

    FILE *file1;
    FILE *file2;
    FILE *output_file;
    char *o_arg = NULL;

    while ((opt = getopt(argc, argv, "io:")) != -1)
    {
        switch (opt)
        {
        case 'i':
            opt_i++;
            break;
        case 'o':
            o_arg = optarg;
            opt_o++;
            break;
        default:
            usage(prog_name);
        }
    }

    if (argc - optind != 2)
    {
        usage(prog_name);
    }
    if ((file1 = fopen(argv[optind], "r")) == NULL || (file2 = fopen(argv[optind + 1], "r")) == NULL)
    {
        fprintf(stderr, "%s: fopen failed: %s\n", prog_name, strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (opt_o > 1 || opt_i > 1)
    {
        usage(prog_name);
    }
    else if (opt_o && o_arg == NULL)
    {
        usage(prog_name);
    }
    else if (opt_o)
    {
        output_file = fopen(o_arg, "w");
    }

    char *line1;
    char *line2;
    ssize_t nread1;
    ssize_t nread2;
    size_t len1 = 0;
    size_t len2 = 0;
    size_t line_length;
    int line_number = 1;
    int diffchar_counter = 0;

    while (((nread1 = getline(&line1, &len1, file1)) != -1) && ((nread2 = getline(&line2, &len2, file2)) != -1))
    {
        line1[strcspn(line1, "\n")] = 0;
        line2[strcspn(line2, "\n")] = 0;
        line_length = strlen(line1) < strlen(line2) ? strlen(line1) : strlen(line2);

        int diff = opt_i ? strncasecmp(line1, line2, line_length) : strncmp(line1, line2, line_length);
        if (diff != 0)
        {
            int i;
            for (i = 0; i < line_length; i++)
            {
                if (line1[i] != line2[i])
                {
                    diffchar_counter++;
                }
            }
            fprintf(opt_o ? output_file : stdout, "Line: %i, characters: %i\n", line_number, diffchar_counter);
            diffchar_counter = 0;
        }
        line_number++;
    }

    free(line1);
    free(line2);
    fclose(file1);
    fclose(file2);
    if (opt_o)
    {
        fclose(output_file);
    }

    return EXIT_SUCCESS;
}
