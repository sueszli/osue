#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <errno.h>

/**
 * @name mydiff
 * @author Kurdo-Jaroslav Asinger, 01300351
 * @brief compares two files line-by-line and prints how many characters deviate each line
 * @details after argument parsing, 
 * @date Nov/10/2021
 */

/**
 * @brief compares the first character of two given strings.
 * 
 * @param line1 
 * @param line2 
 * @param insens if 1, the comparison is case-insensitive, otherwise it is case-sensitive
 * @return int the "difference" of both characters
 */
static int compare(const char *line1, const char *line2, int insens);

/**
 * @brief shows how the program is called corectly
 * 
 */
static void usage(char *progname);

int main(int argc, char *argv[])
{

    char *out = NULL;
    int c = 0;
    int oNum = 0;
    int iNum = 0;

    while ((c = getopt(argc, argv, "io:")) != -1)
    {
        switch (c)
        {
        case 'o':
            out = optarg;
            oNum++;
            break;
        case 'i':
            iNum++;
            break;
        default:
            usage(argv[0]);
            break;
        }
    }

    // check if there are exactly 2 arguments for input-files
    if ((argc - optind) != 2)
    {
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    // check if an option was given multiple times
    if ((oNum > 1) || (iNum > 1))
    {
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    FILE *file1, *file2, *fileOut;

    // open the two input files
    if ((file1 = fopen(argv[optind], "r")) == NULL)
    {
        fprintf(stderr, "ERROR at %s: %s\n", argv[0], strerror(errno));
        exit(EXIT_FAILURE);
    }

    if ((file2 = fopen(argv[optind + 1], "r")) == NULL)
    {
        fprintf(stderr, "ERROR at %s: %s\n", argv[0], strerror(errno));
        fclose(file1);
        exit(EXIT_FAILURE);
    }

    // open output file if option was given, else refer to stdout
    if (out != NULL)
    {
        if ((fileOut = fopen(out, "w")) == NULL)
        {
            fprintf(stderr, "ERROR at %s: %s\n", argv[0], strerror(errno));
            fclose(file1);
            fclose(file2);
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        fileOut = stdout;
    }

    char *line1, *line2;
    // first line is not referred as 0, but 1
    int lineCount = 1;
    size_t size1, size2;
    ssize_t lengthComp, length1, length2;

    /**
     * @brief keep iterations while neither of the files has ended
     * 
     */
    while (((length1 = getline(&line1, &size1, file1)) != -1) && ((length2 = getline(&line2, &size2, file2)) != -1))
    {

        int diffCount = 0;

        // find out which line has fewer characters
        if (length1 > length2)
        {
            lengthComp = length2;
        }
        else
        {
            lengthComp = length1;
        }

        // compare the lines char-by-char
        int i;
        for (i = 0; i < lengthComp; i++)
        {
            char *char1, *char2;
            char1 = line1 + i;
            char2 = line2 + i;

            // pass if option -i was given which indicates if comarison should be case-sensitive or not
            if (compare(char1, char2, iNum) != 0)
            {
                diffCount++;
            }
        }

        // print line number and number of differences (only if least one difference is found)
        if (diffCount != 0)
        {
            fprintf(fileOut, "Line: %d, characters: %d\n", lineCount, diffCount);
        }

        lineCount++;
    }
    free(line1);
    free(line2);
    fclose(fileOut);
    fclose(file1);
    fclose(file2);
    EXIT_SUCCESS;
}

static int compare(const char *line1, const char *line2, int insens)
{
    if (insens == 1)
    {
        return strncasecmp(line1, line2, 1);
    }
    else
    {
        return strncmp(line1, line2, 1);
    }
}

static void usage(char *progname)
{
    fprintf(stderr, "Usage: %s [-i] [-o file] file file\n", progname);
    exit(EXIT_FAILURE);
}
