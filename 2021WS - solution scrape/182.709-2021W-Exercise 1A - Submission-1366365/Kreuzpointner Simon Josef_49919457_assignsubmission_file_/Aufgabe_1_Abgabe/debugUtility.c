/**
 * @file debugUtility.c
 * @author Simon Josef Kreuzpointner 12021358
 * @date 27 October 2021
 * 
 * @brief implementation of the debug utility module
 */

#include <stdio.h>
#include <stdlib.h>

#include "debugUtility.h"

void usage(int exitAfterwards)
{
    extern char *programName; ///< the global varable programName

    (void)fprintf(stderr, "Usage: %s [-o outfile] [file...]\n", programName);

    if (exitAfterwards == 1)
    {
        exit(EXIT_FAILURE);
    }
}