/**
 * @file myExpand.c
 * @author Yuto Sato, 11908092
 * @date 08.11.2021
 *
 * @brief Main prorgram module.
 * 
 * This program is used to replace any tab characters in a given .txt file with spaces.
 * 
 **/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <ctype.h>
#include <string.h>

static unsigned int tabstop = 8;  /** < default distance of tabstop, which can be overriden with any positive integer. > **/
static char *outputFile;          /** < This variable is used to store a pointer to the output file if option -o is given. > **/
static unsigned int position = 0; /** < Marks the position in the line of either stdout or in the output file. >  **/
static unsigned int argCounter;

FILE *fp1, *fp2; /** < Filepointers used to open files given in arguments > **/

/**
 * myExpand without outputFile
 * @brief This method uses the files given and prints every character, while replacing every tab with tabstop amount of spaces.
 * @details global variables: fp1
 */
int myExpand(int counter, char *argv[])
{
    char *inputFile;
    while ((argv[counter]) != NULL)
    {
        char *pointer = argv[counter];
        inputFile = strdup(pointer);

        fp1 = fopen(inputFile, "r");
        if (fp1 != NULL)
        {
            char currentPos = fgetc(fp1);
            while (currentPos != EOF)
            {
                if (currentPos == '\t')
                {
                    int p = tabstop * ((position / tabstop) + 1);
                    while (position < p)
                    {
                        char space = ' ';
                        printf("%c", space);
                        position++;
                    }
                }
                else
                {
                    printf("%c", currentPos);
                    if (currentPos == '\n')
                    {
                        position = 0;
                    }
                    else
                    {
                        position++;
                    }
                }
                currentPos = fgetc(fp1);
            }
        }
        fclose(fp1);
        counter++;
        char newRow = '\n';
        printf("%c", newRow);
    }
    free(inputFile);
    return 0;
}
/**
 * myExpand with outputFile
 * @brief This method uses the files given and puts every character into an outputFile given with option, while replacing every tab with tabstop amount of spaces.
 * @details global variables: fp1, fp2
 */
int myExpandOutFile(int counter, char *argv[])
{
    char *inputFile;
    while ((argv[counter]) != NULL)
    {
        inputFile = strdup(argv[counter]);
        fp1 = fopen(inputFile, "r");
        fp2 = fopen(outputFile, "a");
        if (fp1 != NULL)
        {
            char currentPos = fgetc(fp1);
            while (currentPos != EOF)
            {

                if (currentPos == '\t')
                {

                    int p = tabstop * ((position / tabstop) + 1);
                    while (position < p)
                    {
                        currentPos = ' ';
                        fputc(currentPos, fp2);
                        position++;
                    }
                }
                else
                {
                    fputc(currentPos, fp2);
                    if (currentPos == '\n')
                    {
                        position = 0;
                    }
                    else
                    {
                        position++;
                    }
                }
                currentPos = fgetc(fp1);
            }
        }
        fputc('\n', fp2);
        fclose(fp1);
        fclose(fp2);
        counter++;
    }
    free(inputFile);

    return 0;
}
/**
 * Main function
 * @brief This is the main method of the program. It deals accordingly with the options given, and calls the necessary method afterwards. 
 * @details Checks if the options are all aligned with the synopsis, and returns EXIT_FAILURE if not. 
 * global variables: tabstop, outputFile
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS on successful execution, else EXIT_FAILURE
 */
int main(int argc, char *argv[])
{
    argCounter = argc;
    char *path = argv[0];
    int hasOutputFile = 0;
    int optcount = getopt(argc, argv, "t:o");
    while (optcount != -1)
    {
        switch (optcount)
        {
        case 't':
            if (isdigit(*optarg))
            {
                tabstop = strtol(optarg, NULL, 0);
                if (tabstop != 0)
                {
                    printf("New tabstop: %i \n", tabstop);
                    optcount = getopt(argc, argv, "t:o");
                    argCounter = argCounter - 2;
                }
                else
                {
                    fprintf(stderr, "Usage: %s myexpand [-t tabstop] [-o outputFile] [file...]", path);
                    exit(EXIT_FAILURE);
                }
            }
            else
            {
                fprintf(stderr, "Usage: %s myexpand [-t tabstop] [-o outputFile] [file...]", path);
                exit(EXIT_FAILURE);
            }
            break;

        case 'o':
            if (argv[optind] != NULL)
            {
                outputFile = argv[optind];
                printf("Output to this file: %s\n", outputFile);
                optind++;
                hasOutputFile = 1;
                optcount = getopt(argc, argv, "t:o");
                argCounter = argCounter - 2;
                break;
            }
            else
            {
                fprintf(stderr, "Usage: %s myexpand [-t tabstop] [-o outputFile] [file...]", path);
                exit(EXIT_FAILURE);
            }

        default:
            fprintf(stderr, "Usage: %s myexpand [-t tabstop] [-o outputFile] [file...]", path);
            exit(EXIT_FAILURE);
        }
    }

    if (argCounter == 1)
    {
        fprintf(stderr, "Usage: %s myexpand [-t tabstop] [-o outputFile] [file...]", path);
        exit(EXIT_FAILURE);
    }
    if (hasOutputFile == 0)
    {
        myExpand(optind, argv);
    }
    else
    {
        myExpandOutFile(optind, argv);
    }

    exit(EXIT_SUCCESS);
}
