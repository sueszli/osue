/**
* @file mycompress.c
* @author Andreas Leeb 12019848
* @date 09.11.2021
*
* @brief Main program module which compresses input
*
* @details This program takes input from files or stdin and compresses it.
* The output will be written to a given file or stdout.
**/

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include "mycompress.h"

/**
 * @brief Count read and written chars for statistics
 * 
 */
struct stats
{
    unsigned int readChars, writtenChars;
};

/**
 * @brief algorithm to compress input and write it to output
 * 
 * @param input the name of the input file
 * @param out the FILE where the output should be written to
 * @param stat pointer to statistic struct to track compress efficency
 * @return char* the returned message in case of an error
 */
static char *compress(char *input, FILE *out, struct stats *stat)
{
    char buffer[1024];

    FILE *in;

    if (input != NULL)
    {
        if ((in = fopen(input, "r")) == NULL)
        {
            return "open of input failed: ";
        }
    }
    else
    {
        in = stdin;
    }

    int i = 0, newLineCount = 0;
    char stored;

    while (fgets(buffer, sizeof(buffer), in) != NULL)
    {
        stored = buffer[0];
        int count = 0;

        // empty line
        if (stored == '\n')
        {
            stat->readChars++;
            newLineCount++;
        }
        else
        {
            if (newLineCount > 0)
            {
                fprintf(out, "\n%d", newLineCount);
                int numberLength = floor(log10(abs(newLineCount))) + 1;

                stat->writtenChars += numberLength + 1;

                newLineCount = 0;
            }

            for (i = 0; i < sizeof(buffer); i++)
            {
                stat->readChars++;
                char current = buffer[i];

                if (stored == current)
                {
                    count++;
                }
                else
                {
                    fprintf(out, "%c%d", stored, count);
                    int numberLength = floor(log10(abs(count))) + 1;

                    stat->writtenChars += 1 + numberLength;

                    stored = current;
                    count = 1;
                }

                if (current == '\n')
                {
                    // end of line, no need to search through whole buffer
                    newLineCount++;
                    break;
                }
            }
        }
    }

    if (newLineCount > 0)
    {
        fprintf(out, "\n%d", newLineCount);
        int numberLength = floor(log10(abs(newLineCount))) + 1;

        stat->writtenChars += numberLength + 1;

        fputc('\n', out);
    }

    if (fclose(in) < 0)
    {
        return "close failed: ";
    }

    return NULL;
}

/**
 * @brief main entrypoint of the program taking arguments and calls compress
 * 
 * @param argc argument count
 * @param argv argument values
 * @return int value to determine successfull procedure
 */
int main(int argc, char *argv[])
{
    char *input = NULL;
    char *output = NULL;

    int c;

    while ((c = getopt(argc, argv, "o:")) != -1)
    {
        switch (c)
        {
        case 'o':
            output = optarg;
            break;
        default:
            break;
        }
    }

    FILE *out = NULL;
    char *message = NULL;

    if (output != NULL)
    {
        if ((out = fopen(output, "w")) == NULL)
        {
            fprintf(stderr, "%s: open of output failed: %s\n", argv[0], strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        out = stdout;
    }

    struct stats stat = {
        .readChars = 0,
        .writtenChars = 0,
    };

    if ((argc - optind) > 0)
    {
        int i;

        for (i = optind; i < argc; i++)
        {
            input = argv[i];

            if (input != NULL)
            {
                message = compress(input, out, &stat);

                if (message != NULL)
                {
                    fprintf(stderr, "%s %s %s\n", argv[0], message, strerror(errno));
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
    else
    {
        message = compress(NULL, out, &stat);

        if (message != NULL)
        {
            fprintf(stderr, "%s %s %s\n", argv[0], message, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    if (fclose(out) < 0)
    {
        fprintf(stderr, "%s: close failed: %s\n", argv[0], strerror(errno));
        exit(EXIT_FAILURE);
    }

    fprintf(stderr, "\nRead: %d characters\nWritten: %d characters\nCompression ratio: %.1f%%\n", stat.readChars, stat.writtenChars, (stat.writtenChars * 100.0 / stat.readChars));

    return 0;
}
