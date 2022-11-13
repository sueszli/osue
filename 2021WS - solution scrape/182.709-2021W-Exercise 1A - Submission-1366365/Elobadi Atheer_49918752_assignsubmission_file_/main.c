
/**
 * @file main.c
 * @author Atheer Elobadi <e01049225@student.tuwien.ac.at>
 * @date 30.10.2021
 *
 * @brief this program goes line by line over a specified files and looks for a given
 * keyword within them.
 */

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include "mygrep.h"

void usage();
char *programName;

int main(int argc, char *argv[])
{
    programName = argv[0];
    int opt_i;
    int opt_o;
    char *outputFileName = NULL;
    FILE *outputFile = NULL;
    char *keyword;

    if (handleOptions(&opt_i, &opt_o, &outputFileName, argc, argv) == -1)
    {
        usage(programName);
        exit(EXIT_FAILURE);
    }

    keyword = argv[optind];

    // Open the output file if given
    if (opt_o > 0)
    {
        if ((outputFile = fopen(outputFileName, "w")) == NULL)
        {
            fprintf(stderr, "%s: can´t open file %s\n", programName, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        outputFile = stdout;
    }

    // Check if there is a keyword
    if (keyword == NULL)
    {
        usage(programName);
        exit(EXIT_FAILURE);
    }

    FILE *input;

    bool readingFromFiles = false;
    if (argv[optind + 1] != NULL)
    {
        readingFromFiles = true;
    }
    else
    {
        input = stdin;
    }

    while (1)
    {
        if (readingFromFiles)
        {
            if (argv[++optind] != NULL)
            {
                if ((input = fopen(argv[optind], "r")) == NULL)
                {
                    fprintf(stderr, "%s: can´t open file %s\n", programName, strerror(errno));
                    exit(EXIT_FAILURE);
                }
            }
            else
            {
                return EXIT_SUCCESS;
            }
        }

        int resultOfFindString = putLinesIfContainString(input, opt_i, keyword, outputFile);
        if (resultOfFindString == -2)
        {
            fprintf(stderr, "%s: can´t write to file %s\n", programName, strerror(errno));
            exit(EXIT_FAILURE);
        }
        else if (resultOfFindString == -1)
        {
            exit(EXIT_FAILURE);
        }
        else if (resultOfFindString == 1)
        {
            return EXIT_SUCCESS;
        }

        if (readingFromFiles)
            fclose(input);
    }

    if (outputFile != NULL)
        fclose(outputFile);

    return EXIT_SUCCESS;
}
