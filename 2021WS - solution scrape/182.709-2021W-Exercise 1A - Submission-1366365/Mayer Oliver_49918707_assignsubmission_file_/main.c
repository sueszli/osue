/**
 * @file main.c
 * @author Oliver Mayer, MatNr: 12023147
 * @brief  Program which accepts different inputs and checks for every line if it is a palindrom.
 * 
 * @details The standard input is stdin but many input files can be specified. 
 *          If atleast one inputfile is specified it's no longer possible to input via stdin.
 *          The standard output is stdout but a output file can be specified. The ouput to stdout is only if no output file is specified
 *          Two options are accepted to define how whitespaces or lower/upper case letters are treated. 
 * @version 0.1
 * @date 2021-11-09
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include "isPalindrom.h"

/**
 * @brief Prints the synopsis for the command and exits
 * 
 */
void usage(void);

/**
 * @brief Holds the name of this program
 * 
 */
char *myprog;

void usage(void)
{
    fprintf(stderr, "Usage: %s [-s] [-i] [-o outfile] [file...]\n", myprog);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    myprog = argv[0];
    int c; //Varialbe for getopt
    int ignoreWhitespaces = 0;
    int caseSensitive = 1;
    char *outputFilename = NULL;
    char **inputFilenames = NULL;
    FILE *outputFile = stdout;

    //Optional argument parsing
    while ((c = getopt(argc, argv, "sio:")) != -1)
    {
        switch (c)
        {
        case 's':
            if (ignoreWhitespaces == 1)
            {
                usage();
            }
            ignoreWhitespaces = 1;
            break;
        case 'i':
            if (caseSensitive == 0)
            {
                usage();
            }
            caseSensitive = 0;
            break;
        case 'o':
            if (outputFilename != NULL)
            {
                usage();
            }
            outputFilename = optarg;
            break;
        case '?':
        case ':':
            usage();
            break;
        default:
            assert(0);
            usage();
            break;
        }
    }

    // Get input files
    int numberOfNonOptionArg = argc - optind;
    if (numberOfNonOptionArg > 0)
    {
        inputFilenames = malloc(sizeof(char) * numberOfNonOptionArg);
        char **tmp = inputFilenames;
        if (inputFilenames == NULL)
        {
            fprintf(stderr, "[%s] Failed to allocate memory: %s\n", argv[0], strerror(errno));
            exit(EXIT_FAILURE);
        }
        while (optind < argc)
        {
            *inputFilenames = argv[optind];
            inputFilenames++;
            optind++;
        }
        inputFilenames = tmp;
    }

    // Change outputFile if one was specified
    if (outputFilename != NULL)
    {
        outputFile = fopen(outputFilename, "w");
        if (outputFile == NULL)
        {
            free(inputFilenames);
            fprintf(stderr, "[%s] Failed to open \"%s\": %s\n", argv[0], outputFilename, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    size_t BUFSIZE = 32;
    char *inputLine = NULL; //Must be freed due to use in getline()
    int size = 0;

    if (inputFilenames == NULL)
    {
        while ((size = getline(&inputLine, &BUFSIZE, stdin)) != -1)
        {
            inputLine[size - 1] = '\0';

            if (size > 1) //Ignore input only containing a '\n'
            {
                char *help = isPalindrom(inputLine, ignoreWhitespaces, caseSensitive);
                if (help == NULL)
                {
                    free(inputLine);
                    fprintf(stderr, "[%s] Failed to check line: %s\n", argv[0], strerror(errno));
                    exit(EXIT_FAILURE);
                }
                fprintf(outputFile, "%s %s\n", inputLine, help);
            }
        }
        // If getline() returned -1 due to an error
        if ((errno == EINVAL) || (errno == ENOMEM))
        {
            free(inputLine);
            fprintf(stderr, "[%s] Failed to read from \"stdin\": %s\n", argv[0], strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        // Read file after file
        for (int i = 0; i < numberOfNonOptionArg; i++)
        {
            FILE *inputFile = fopen(*(inputFilenames + i), "r");
            if (inputFile == NULL)
            {
                free(inputFilenames);
                free(inputLine);
                fprintf(stderr, "[%s] Failed to open \"%s\": %s\n", argv[0], *(inputFilenames + 1), strerror(errno));
                exit(EXIT_FAILURE);
            }

            //Check and output line per line
            while ((size = getline(&inputLine, &BUFSIZE, inputFile)) != -1)
            {
                if (inputLine[size - 1] == '\n') //Remove '\n' for better output-format
                {
                    inputLine[size - 1] = '\0';
                }
                if (size > 1) //Ignore lines only containing a '\n'
                {
                    char *help = isPalindrom(inputLine, ignoreWhitespaces, caseSensitive);
                    if (help == NULL)
                    {
                        free(inputFilenames);
                        free(inputLine);
                        fprintf(stderr, "[%s] Failed to check line: %s\n", argv[0], strerror(errno));
                        exit(EXIT_FAILURE);
                    }
                    fprintf(outputFile, "%s %s\n", inputLine, help);
                }
                if (feof(inputFile) != 0)
                {
                    break;
                }
            }

            // If getline() returned -1 due to an error
            if ((errno == EINVAL) || (errno == ENOMEM))
            {
                free(inputFilenames);
                free(inputLine);
                fprintf(stderr, "[%s] Failed to read from \"%s\": %s\n", argv[0], *(inputFilenames + 1), strerror(errno));
                exit(EXIT_FAILURE);
            }

            if (fclose(inputFile) < 0)
            {
                free(inputFilenames);
                free(inputLine);
                fprintf(stderr, "[%s] Failed to close \"%s\": %s\n", argv[0], *(inputFilenames + 1), strerror(errno));
                exit(EXIT_FAILURE);
            }
        }
        free(inputFilenames);
    }
    free(inputLine);

    if (fclose(outputFile) < 0)
    {
        fprintf(stderr, "[%s] Failed to close \"%s\": %s\n", argv[0], outputFilename, strerror(errno));
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}
