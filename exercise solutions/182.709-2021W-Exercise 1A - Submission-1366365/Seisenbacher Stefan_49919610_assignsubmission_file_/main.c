/**
 * @file main.c
 * @author Stefan Seisenbacher
 * @date 09.11.2021
 *
 * @brief Main program module.
 * 
 * This program compresses a string input. output to stdout or a outputfile
 * 
 **/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "compressor.h"
#include "filehandler.h"
#include "string.h"

//program name is stored in a global variable
char *PROGRAM_NAME;

/**
 * Mandatory usage function.
 * @brief This function writes helpful usage information about the program to stderr.
 * @details global variables: pgm_name
 */
static void usage(void) {
	(void) fprintf(stderr, "USAGE: %s [-o outfile] [file...] [\n", PROGRAM_NAME);
	exit(EXIT_FAILURE);
}


/**
 * Program entry point.
 * @brief bla 
 * @details bla
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS.
 */
int main(int argc, char *argv[])
{
    char *outputFile;
    char optionsCharacter;
    int charactersRead = 0;
    int charactersWritten = 0;
   
    PROGRAM_NAME = argv[0];
    while ((optionsCharacter = getopt(argc, argv, "o:")) != -1)
    {
        switch (optionsCharacter)
        {
        case 'o':
            outputFile = optarg;
            break;
        case '?':
           // fprintf(stderr,"Invalid option\n");
           usage();
            return EXIT_FAILURE;

        default:
            usage();
            return EXIT_FAILURE;
        }
    }   

    if (argc == 1 || (outputFile != NULL && argc == 3))
    {
        char *line = NULL;
        size_t len = 0;
        getline(&line, &len, stdin);
        char *compressed = compress(line);
        charactersRead += strlen(line);
        charactersWritten += strlen(compressed);
        free(line);
        if (outputFile == NULL)
        {
            printf("%s", compressed);
        }
        else
        {
            writeToFile(compressed, outputFile);
        }        
    }
    else
    {
        int i = 1;
        if (outputFile != NULL)
        {
            i = 3;
        }
        while (i < argc)
        {
            char *input = readFile(argv[i]);
            charactersRead += strlen(input);
            char *compressed = compress(input);
            charactersWritten += strlen(compressed);
            if (outputFile == NULL)
            {
                printf("%s", compressed);
            }
            else
            {
                writeToFile(compressed, outputFile);
            }
            i++;
        }       
    }   

        fprintf(stderr,"Read: %8d characters\n", charactersRead);
        fprintf(stderr,"Written: %5d characters\n", charactersWritten);        
        fprintf(stderr,"Compression ratio: %.2f%%", 100 * ((float)charactersWritten / (float)charactersRead));

    return EXIT_SUCCESS;
}