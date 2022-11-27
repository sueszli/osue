/**
 * @file main.c
 * @author Noah Reinalter 11908085
 * @brief This module implements the functionality of exercise 1A.
 * @version 1
 * @date 2021-11-13
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

bool searchInString();
void printArgumentError();

/**
 * Programm entry point
 * @brief Main funktion for the exercise 1A.
 * @details This funktion parses the given arguments and manages the output location.
 * 
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return EXIT_SUCCESS if the programm worked without error.
 * @return EXIT_FAILURE if there was an error parsing the function arguments.
 */
int main(int argc, char *argv[])
{
    char *o_arg = NULL;
    int opt_o = 0;
    int opt_i = 0;
    int c;
    FILE *outputFile;

    while ((c = getopt(argc, argv, "io:")) != -1)
    {
        switch (c)
        {
        case 'o':
            o_arg = optarg;
            opt_o = 1;
            break;
        case 'i':
            opt_i = 1;
            break;
        case '?':
            break;
        }
    }

    if (opt_o == 1 && o_arg == NULL) // Exits the programm when the o flag is present but no value for it is provieded.
    {
        printArgumentError();
        return EXIT_FAILURE;
    }
    else if (o_arg != NULL) // If there is a outputfile location provieded this openes the file so the programm can later write to it.
    {
        char *filename = malloc(3 + strlen(o_arg));

        if (filename == NULL)
        {
            fprintf(stderr, "Error: Could not allocate memory\n");
            return EXIT_FAILURE;
        }

        filename[0] = '.';
        filename[1] = '/';
        strcat(filename, o_arg);
        outputFile = fopen(filename, "w");

        if (outputFile == NULL)
        {
            fprintf(stderr, "Error: Could not open output file\n");
            return EXIT_FAILURE;
        }
    }

    if ((argc - optind) < 1) // At least one positional argument must be present.
    {
        printArgumentError();
        return EXIT_FAILURE;
    }
    else if ((argc - optind) == 1) // If only on positional argument the lines to compare are read from stdin.
    {
        char *partialString = argv[optind];
        while (1) // Reads for ever from stdin
        {
            char *input = NULL;
            size_t inputSize = 0;
            if (getline(&input, &inputSize, stdin)) // Only proceed if a line can be read from stdin.
            {
                input[strlen(input) - 1] = '\0';                          // Makes the input string null terminated.
                if (searchInString(&input, partialString, opt_i) == true) // True if partialString is in input.
                {
                    if (o_arg != NULL)
                    {
                        fprintf(outputFile, "%s\n", input);
                    }
                    else
                    {
                        printf("%s\n", input);
                    }
                }
            }
            free(input);
        }
    }
    else
    {
        char *partialString = argv[optind];
        for (size_t i = optind + 1; i < argc; i++) // Goes over all the input file names.
        {
            char *filename = malloc(3 + strlen(argv[i]));
            if (filename == NULL)
            {
                break;
            }
            filename[0] = '.';
            filename[1] = '/';
            strcat(filename, argv[i]);
            FILE *file = fopen(filename, "r");
            if (file == NULL) // If the file can not be opened the file will be ignored.
            {
                free(filename);
                break;
            }

            char *input = NULL;
            size_t inputSize = 0;
            ssize_t read;
            while ((read = getline(&input, &inputSize, file)) != -1) // Reads ever line of the file into input.
            {
                input[strlen(input) - 1] = '\0';
                if (searchInString(&input, partialString, opt_i) == true)
                {
                    if (o_arg != NULL)
                    {
                        fprintf(outputFile, "%s\n", input);
                    }
                    else
                    {
                        printf("%s\n", input);
                    }
                }
            }
            free(filename);
            fclose(file);
            free(input);
        }
    }

    if (o_arg != NULL)
    {
        fclose(outputFile);
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Checks if a given partialString is in a given inputString
 * @details Very similar to strstr(3) with the only difference that, 
 * there is an easy way to make a check when the check needs to be caseinsensitive.
 * 
 * @param inputString The haystack string in the search.
 * @param partialString The needle string in the search.
 * @param caseinsensitive Value if the search should be caseinsensitive.
 * @return true If the partialString was found in the inputString.
 * @return false If the partialString was not found in the inputString.
 */
bool searchInString(char **inputString, char *partialString, int caseinsensitive)
{
    for (size_t i = 0; i < strlen(*inputString); i++) // Loops over the inputString.
    {
        if (strlen(*inputString) < (strlen(partialString) + i)) // Therminat if there are less chars in inputString left then chars in partialString.
        {
            return false;
        }

        // Checks if inputString at the index i is the same as the first char of partialString and if the search is caseinsensitve also check if the upper case chars are the same.
        if ((*inputString)[i] == partialString[0] || (caseinsensitive && ((char)toupper((*inputString)[i]) == (char)toupper(partialString[0]))))
        {
            for (size_t j = 0; j < strlen(partialString); j++) // Loop over partialString to check every index of partialString.
            {
                if (caseinsensitive)
                {
                    if ((char)toupper((*inputString)[i + j]) != (char)toupper(partialString[j])) // If the chars after toupper are not the same we know that the start of the substring can not be i and we break.
                    {
                        break;
                    }
                    // If it is the last char from partialString and it is the same as the coresponding char from inputString after toupper we know we found a substring and return true.
                    else if ((j == (strlen(partialString) - 1)) && ((char)toupper((*inputString)[i + j]) == (char)toupper(partialString[j])))
                    {
                        return true;
                    }
                }
                else
                {
                    if ((*inputString)[i + j] != partialString[j]) // If the chars are not the same we know the substring has a different start and break.
                    {
                        break;
                    }
                    else if ((j == (strlen(partialString) - 1)) && ((*inputString)[i + j] == partialString[j])) // If it is the last char from partialString and the chars are the same return true.
                    {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

/**
 * @brief A function to print a standardized error for an argument error.
 * 
 */
void printArgumentError()
{
    fprintf(stderr, "Error parsing arguments for mygrep [-i] [-o outfile] keyword [file...]\n");
}