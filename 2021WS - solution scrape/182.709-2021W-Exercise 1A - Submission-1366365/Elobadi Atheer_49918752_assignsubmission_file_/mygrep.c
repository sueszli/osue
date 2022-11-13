
/**
 * @file mygrep.c
 * @author Atheer Elobadi <e01049225@student.tuwien.ac.at>
 * @date 30.10.2021
 *
 * @brief This module contains util functions which are used for finding a keyword in lines either given in stdin or in files 
 */


#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <assert.h>

#define USAGE "Usage: %s [-i] [-o outfile] keyword [file...]\n"

/**
 * @brief prints the usage and exists with a failure message.
 * @param[in] programName   the name of the programm
 */
void usage(char *programName)
{
    fprintf(stderr, USAGE, programName);
}

/**
 * @brief handels the options.
 * @param[out]  opt_i            1 if the i option was given, 0 otherwise.
 * @param[out]  opt_o            1 if the o option was given, 0 otherwise.
 * @param[out]  outputFileName   sets the name of the output file if the o option was given.
 * @param[in]   argc             the number of arguments that was given when running the program.
 * @param[in]   argv             the list of arguments.
 * @return      0 on success and -1 on failure.
 */
int handleOptions(int *opt_i, int *opt_o, char **outputFileName, int argc, char *argv[])
{
    *opt_i = 0;
    *opt_o = 0;
    char c;
    while ((c = getopt(argc, argv, "io:")) != -1)
    {
        switch (c)
        {
        case 'i':
            (*opt_i)++;
            if (*opt_i > 1)
            {
                return -1;
            }
            break;
        case 'o':
            (*opt_o)++;
            *outputFileName = optarg;
            if (*opt_o > 1)
            {
                return -1;
            }
            break;
        case '?':
            return -1;
        default:
            assert(0);
        }
    }
    return 0;
}


/**
 * @brief converts a given char string to lower case
 * @par[out]    line    the char string which should be converted.
 */
void toLowerCase(char *line)
{
    char *letter = line;
    for (; *letter; ++letter)
        *letter = tolower((unsigned char)*letter);
}


/**
 * @brief puts all lines from input if they contain the given keyword to the specified output stream.
 * @param[in]   input   the input stream/file that should be read.
 * @param[in]   opt_i   >0 if the search should be case insensitive.
 * @param[in]   keyword the keyword which should be looked for in each line.
 * @param[in]   output  the output stream to which the results should be written.
 * @return 
 */
int putLinesIfContainString(FILE *input, int opt_i, char *keyword, FILE *output)
{
    size_t lineSize = 0;
    char *line;
    while (getline(&line, &lineSize, input) >= 0 && *line != '\n')
    {
        if (opt_i > 0)
        {
            toLowerCase(line);
            toLowerCase(keyword);
        }
        if (strstr(line, keyword) != NULL)
        {
            if (output != NULL)
            {
                if (fputs(line, output) == EOF)
                {
                    return -2;
                }
            }
            else
            {
                fprintf(stderr, "%s: Null output is given. %s\n", __func__, strerror(errno));
                return -1;
            }
        }
    }
    if (*line == '\n')
    {
        return 1;
    }
    return 0;
}

