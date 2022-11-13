/**
 * @file utility.c
 * @author Simon Josef Kreuzpointner 12021358
 * @date 27 October 2021
 *
 * @brief implementation of the utility module
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "utility.h"
#include "debugUtility.h"

int numberOfDigits(int n)
{
    n = abs(n);

    if (n < 10)
        return 1;
    if (n < 100)
        return 2;
    if (n < 1000)
        return 3;
    if (n < 10000)
        return 4;
    if (n < 100000)
        return 5;
    if (n < 1000000)
        return 6;
    if (n < 10000000)
        return 7;
    if (n < 100000000)
        return 8;
    if (n < 1000000000)
        return 9;

    // the maximum value of an integer has 10 places
    return 10;
}

int appendToString(char **s, size_t size, int offset, char *content)
{
    if (*s == NULL)
    {
        *s = malloc(size);
        if (*s == NULL)
        {
            printError("malloc", "An error ocurred while trying to allocate memory.");
            return -1;
        }

        strcpy(*s, content);
    }
    else
    {
        *s = realloc(*s, size);
        if (*s == NULL)
        {
            printError("realloc", "An error ocurred while trying to reallocate memory.");
            return -1;
        }

        strcpy(*s + offset, content);
    }

    return 0;
}

int printVerticalSpacing(int n, FILE *stream)
{
    if (stream == NULL)
    {
        return -1;
    }

    for (int i = 0; i < n; i++)
    {
        (void)fprintf(stream, "\n");
    }

    return 0;
}

void printError(char *functionName, char *msg)
{
    extern char *programName; ///< the global variable programName

    (void)fprintf(
        stderr,
        "[%s]\t[ ERROR ] %s: %s\n%s\n", programName, functionName, msg, strerror(errno));
}