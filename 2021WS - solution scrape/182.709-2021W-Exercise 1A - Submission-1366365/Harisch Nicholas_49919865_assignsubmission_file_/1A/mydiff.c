/**
 * @file mydiff.c
 * @author Nicholas Harisch 11705361<harischnicholas@gmail.com>
 * @date 10.11.2021
 *
 * @brief Programm to count differences between text lines
 * 
 * This program  accepts two text files as inputs and reads them line by line, counting the differences in each line and printing
 * the result to stdout, or if the -o parameter is set, to an output file. With -i the program ignores letter casing.
 **/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>


/**
 * Program entry point.
 * @brief The program starts here. The function takes care of the whole functionality of the program.
 * @details Handles the parameter inputs. Then goes through the given input file line by line, comparing the characters. Counts the differences in
 * the lines and outputs the result to stdout. If an ouput file is given, it writes the result line by line.
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS.
 */
int main(int argc, char *argv[])
{

    //Handling and couting arguments
    int c;
    int opt_o = 0;
    int opt_i = 0;
    char *o_arg = NULL;

    while ((c = getopt(argc, argv, "o:i")) != -1)
    {
        switch (c)
        {
        case 'o':
            o_arg = optarg;
            opt_o++;
            break;
        case 'i':
            opt_i++;
            break;
        default:
            break;
        }
    }

    /* number of positional arguments is not 2 */
    if ((argc - optind) != 2)
    {
        printf("Pos. arguments missing\n");
        exit(EXIT_FAILURE);
    }

    //File names
    char *f1 = argv[optind];
    char *f2 = argv[optind + 1];

    //File pointers
    FILE *fp1 = fopen(f1, "r");
    FILE *fp2 = fopen(f2, "r");
    FILE *fpOut;

    if (o_arg != NULL)
    {
        fpOut = fopen(o_arg, "w");
    }

    char buffer1[1024];
    char buffer2[1024];

    //Open both input files read only
    if ((fp1 = fopen(f1, "r")) == NULL)
    {
        fprintf(stderr, "fopen failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if ((fp2 = fopen(f2, "r")) == NULL)
    {
        fprintf(stderr, "fopen failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    int lineNr = 0;

    //Read line by line
    while (fgets(buffer1, sizeof(buffer1), fp1) != NULL)
    {
        int maxlength = strlen(buffer1) - 1;
        lineNr++;
        if (fgets(buffer2, sizeof(buffer2), fp2) != NULL)
        {

            //Get smaller value of lengths
            if (maxlength > strlen(buffer2) - 1)
            {
                maxlength = strlen(buffer2) - 1;
            }

            //If two lines are not the same (up to shorter length)
            if (strncmp(buffer1, buffer2, maxlength - 1) != 0)
            {
                int errCount = 0;
                //Count the number of differences
                for (int i = 0; i < maxlength; i++)
                {
                    //If not case sensitive
                    if (opt_i > 0)
                    {
                        if (toupper(buffer1[i]) != toupper(buffer2[i]))
                        {
                            errCount++;
                        }
                    }
                    else
                    {
                        if (buffer1[i] != buffer2[i])
                        {
                            errCount++;
                        }
                    }
                }
                //Print/Write to file
                if (errCount > 0)
                {
                    if (o_arg != NULL)
                    {
                        char outBuffer[1024];
                        snprintf(outBuffer, sizeof(outBuffer), "Line: %d, characters: %d\n", lineNr, errCount);
                        if (fputs(outBuffer, fpOut) == EOF)
                        {
                            fprintf(stderr, "fputs failed: %s\n", strerror(errno));
                            exit(EXIT_FAILURE);
                        }
                    } else {
                        printf("Line: %d, characters: %d\n", lineNr, errCount);
                    }
                }
            }
        }
        else
        {
            exit(EXIT_SUCCESS);
        }
    }

    if (ferror(fp1) || ferror(fp2))
    {
        fprintf(stderr, "fgets failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}