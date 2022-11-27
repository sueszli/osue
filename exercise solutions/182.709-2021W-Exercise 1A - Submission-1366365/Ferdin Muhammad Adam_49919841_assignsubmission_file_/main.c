/** 
 * @name main.c
 * @author Muhammad Adam Ferdin
 * @details 11910541
 * @brief main program file for mydiff
 * @date 14.11.2021
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h> 

/** 
 * compares two lines of characters, they are only compared until one the lines ends
 * returns the number of differing characters
 */

int compare(char *line1, char *line2, ssize_t linesize1, ssize_t linesize2, int caseSensitive)
{
    ssize_t size = 0;
    int diffChar = 0;

    // determening which char array is shorter
    if (linesize1 <= linesize2)
    {
        size = linesize1 - 1;
    }
    else
    {
        size = linesize2 - 1;
    }

    // if the option -i was chosen the two strings will be compared without casesensitivity
    if (caseSensitive == 0)
    {
        for (ssize_t i = 0; i < size; i++)
        {
            if ((tolower(line1[i])) != (tolower(line2[i])))
                diffChar++;
        }
    }
    else
    {
        for (ssize_t i = 0; i < size; i++)
        {
            if (line1[i] != line2[i])
                diffChar++;
        }  
    }
    return diffChar;
}

/**
 * if the program has not been called properly it is the terminated with the following message
 */

void usage()
{
    fprintf(stderr, "Usage: mydiff [-i] [-o outfile] file1 file2");
    exit(EXIT_FAILURE);
}

int main(int argc, char const *argv[])
{
    int c;
    char *filePathOut = NULL;
    char *filePath1 = NULL;
    char *filePath2 = NULL;
    int caseSensitive = 1;

    // checking for options
    while ((c = getopt(argc, argv, "io:")) != -1)
    {
        switch (c)
        {
        case 'i':
            caseSensitive = 0;
            break;
        case 'o':
            filePathOut = optarg;
            break;
        case '?':
            usage();
            break;
        }
    }

    // checking if there ar no more options
    if (argc - optind != 2)
    {
        usage();
    }

    // initialising variables for positional arguments
    filePath1 = argv[optind];
    filePath2 = argv[optind + 1];

    if (filePath1 == " " || filePath2 == " ")
    {
        fprintf(stderr, "No file path given: %d\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // opening and checking filestreams
    FILE *file1 = fopen(filePath1, "r");
    FILE *file2 = fopen(filePath2, "r");

    if (file1 == NULL || file2 == NULL)
    {
        fprintf(stderr, "fopen failed: %d\n", strerror(errno));
        fclose(file1);
        fclose(file2);
        exit(EXIT_FAILURE);
    }

    // initializing all variables for the getline() function
    size_t buffsize1 = sizeof(char) * 2;
    size_t buffsize2 = sizeof(char) * 2;
    char *buffer1 = (char *)malloc(buffsize1);
    char *buffer2 = (char *)malloc(buffsize1);
    ssize_t linesize1;
    ssize_t linesize2;

    // dynamically allocated integer array for number of different characters in each row
    size_t arraySize = 8;
    int *mistakes = (int *)malloc(sizeof(int) * arraySize);
    if (mistakes == NULL)
    {
        fprintf(stderr, "malloc for mistakes failed: %d\n", strerror(errno));
        free(mistakes);
        fclose(file1);
        fclose(file2);
        exit(EXIT_FAILURE);
    }

    int linecounter = 0;

    
    // while loop for getting the lines from both inputfiles and comparing them to each other
    while ((linesize1 = getline(&buffer1, &buffsize1, file1)) >= 0 && (linesize2 = getline(&buffer2, &buffsize2, file2)) >= 0)
    {
        // adjusting the arraysize of mistakes if necessary
        if (linecounter >= arraySize)
        {
            arraySize = arraySize * 2;
            mistakes = realloc(mistakes, sizeof(int) * arraySize);
            if (mistakes == NULL)
            {
                fprintf(stderr, "realloc for array failed: %d\n", strerror(errno));
                free(mistakes);
                fclose(file1);
                fclose(file2);
                free(buffer1);
                free(buffer2);
                exit(EXIT_FAILURE);
            }
        }

        // comparing the two strings and writing the number of differences into the mistakes array
        if ((mistakes[linecounter] = compare(buffer1, buffer2, linesize1, linesize2, caseSensitive)) == -1)
        {
            fprintf(stderr, "string comparison failed: %d\n", strerror(errno));
            free(mistakes);
            fclose(file1);
            fclose(file2);
            free(buffer1);
            free(buffer2);
            exit(EXIT_FAILURE);
        }
        linecounter++;
    }

    // close all open files/streams
    free(buffer1);
    free(buffer2);
    fclose(file1);
    fclose(file2);

    
    /** 
     * programm will put out the number of differences per line
     * if an outputfile has been specified it wil write it into that instead
     */

    if (filePathOut != NULL)
    {
        FILE *outfile = fopen(filePathOut, "w");

        for (int i = 0; i < linecounter; i++)
        {
            if (mistakes[i] != 0)
                fprintf(outfile, "Line: %i, characters: %i\n", (i + 1), mistakes[i]);
        }

        fclose(outfile);
    }
    else
    {
        for (int i = 0; i < linecounter; i++)
        {
            if (mistakes[i] != 0)
                fprintf(stdout, "Line: %i, characters: %i\n", (i + 1), mistakes[i]);
        }
    }

    // close the dynamically allocated mistakes array
    free(mistakes);

    return 0;
}
