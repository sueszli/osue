/**
 * @file fileActions.c
 * @author Simon Josef Kreuzpointner 12021358
 * @date 28 October 2021
 *
 * @brief implementation of the file actions module
 */

#include <stdlib.h>
#include <string.h>

#include "fileActions.h"
#include "utility.h"
#include "debugUtility.h"

int closeFile(FILE *file)
{
    if (file != NULL)
    {
        if (fclose(file) != 0)
        {
            printError("fclose", "An error ocurred while trying to close a file.");
            return -1;
        }
    }

    return 0;
}

int closeFiles(FILE *files[], int fileCount)
{
    for (int i = 0; i < fileCount; i++)
    {
        if (closeFile(files[i]) < 0)
        {
            return -1;
        }
    }

    return 0;
}

int openFile(FILE **file, char *fileName, const char *mode)
{
    debug("Opening file %s in '%s' mode.", fileName, mode);

    *file = fopen(fileName, mode);
    if (*file == NULL)
    {
        printError("fopen", "n error ocurred while trying to open a file.");
        return -1;
    }

    return 0;
}

int openFiles(FILE **files[], char **fileNames, const char *mode, int fileCount)
{
    debug("Opening files.");

    for (int i = 0; i < fileCount; i++)
    {
        if (openFile(&(*files)[i], fileNames[i], mode) < 0)
        {
            return -1;
        }
    }

    return 0;
}

int getFilesContent(FILE *files[], int fileCount, char **dest)
{
    debug("Getting all file contents.");

    int bufferLength = 32;      ///< The length of the buffer without the end-of-string symbol.
    int actualBufferLength = 0; ///< This holds the actual buffer length after the usage of fgets(3).

    // add 1 for the end-of-string symbol
    char buffer[sizeof(char) * bufferLength + 1]; ///< String buffer where the results from fgets(3) are put into.

    int size = 0;          ///< Keeps track of the size of dest.
    int pointerOffset = 0; ///< Points to the first empty position of dest.

    for (int i = 0; i < fileCount; i++)
    {
        while (fgets(buffer, sizeof(buffer), files[i]) != NULL)
        {
            int l = strlen(buffer);

            actualBufferLength = l < bufferLength ? l : bufferLength;

            size += sizeof(char *) * actualBufferLength;

            if (appendToString(dest, size, pointerOffset, buffer) < 0)
            {
                printError("appendToString", "An error ocurred while appending a string.");
                return -1;
            }

            pointerOffset += actualBufferLength;
        }

        if (ferror(files[i]) != 0)
        {
            printError("ferror", "An error ocurred while trying to read from a file.");
            return -1;
        }
    }

    return 0;
}

int writeToFile(FILE *file, char *fileName, char *content)
{
    debug("Writing to file: '%s'", fileName);

    if (strcmp(fileName, "stdout") == 0)
    {
        if (printVerticalSpacing(3, stdout) < 0)
        {
            return -1;
        }
    }

    int written = fprintf(file, "%s", content);

    if (written != strlen(content))
    {
        debug("Written: %d", written);
        debug("Len: %ld", strlen(content));

        printError("fprintf", "An error ocurred while writing to a file.");
        return -1;
    }

    if (fflush(file) != 0)
    {
        printError("fflush", "An error ocurred while flushing a file.");
        return -1;
    }

    return 0;
}