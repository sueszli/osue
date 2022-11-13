
/**
 * @file main.c
 * @author Josef Taha <e11920555@student.tuwien.ac.at>
 * @date 03.11.2021
 *
 * @brief Main program module.
 *
 * This program reads every line of multipe optinal given files (when none given: stdin) and writes ervery line that contains keyword
 * in an optionally given output file (when none given: stdout). By adding flag -i spaces lower/uppersize differences can be ignored.
 **/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stddef.h>
#include "header.h"


/**
 * Mandatory usage function.
 * @brief This function prints helpful usage information about the program to stderr.
 * @param prog name of the program
 */
static void usage(char *prog)
{
    fprintf(stderr, "Usage: %s [-i] [-o outfile] keyword [file...]\n", prog);
    exit(EXIT_FAILURE);
}

/**
 * Program entry point.
 * @brief handles and process the parameter input to pass it to functions. Checks if the input parameter
 * are equivalent to the synopsis. Opens all necessary files and closes them by end of the program.
 * @param argc the argument counter.
 * @param argv the argument vector.
 * @return Returns 0 if success.
 */
int main(int argc, char **argv)
{

    int c;
    int iflag;
    int oflag;
    char *keyword;

    FILE **input;
    FILE *output = stdout;

    while ((c = getopt(argc, argv, "io:")) != -1)
    {
        switch (c)
        {
        case 'i':
            if (iflag == 1)
                usage(argv[0]);
            iflag = 1;
            break;
        case 'o':
            if (oflag == 1)
                usage(argv[0]);
            oflag++;
            output = fopen(optarg, "w");
            if (output == NULL)
            {
                fprintf(stderr, "[%s] fopen failed: %s\n", argv[0], strerror(errno));
                exit(EXIT_FAILURE);
            }
            break;
        default:
            usage(argv[0]);
        }
    }

    if (argc > optind)
    {

        keyword = argv[optind];

        if (argc > optind + 1)
        {
            input = malloc(sizeof(FILE *) * (argc - (optind + 1)));
            for (int i = 0; i < (argc - (optind+1)); i++)
            {
                input[i] = fopen(argv[optind + 1 + i], "r");
                if (input[i] == NULL)
                {
                    fprintf(stderr, "[%s] fopen famallociled: %s\n", argv[0], strerror(errno));
                    exit(EXIT_FAILURE);
                }
            }
        } else {
            input = malloc(sizeof(FILE *));
            input[0] = stdin;
        }
    }
    else
    {
        usage(argv[0]);
    }

    io(input, output, keyword, iflag, argv[0]);

    for (int i = 0; input[i]; i++)
    {
        fclose(input[i]);
    }
    free(input);
    fclose(output);


    return 0;
}