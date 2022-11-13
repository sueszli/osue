/**
 * @file   mydiff.c
 * @author e11802361
 * @date   22.10.2021
 * 
 * @brief  mydiff compares two text files line by line.
 * 
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <ctype.h>

// this variable  needs to be used in different functions hence why it is global
char *programName;

/**
 * @brief compares two lines
 * 
 * @param buffer1 a line in the first text file
 * @param buffer2 a line in the second text file
 * @param caseSensitivity an int which is used to set the case sensitivity on and off
 * 
 * @return amount of differences after comparing two lines
*/
static int compareLines(char *buffer1, char *buffer2, int caseSensitivity) 
{
    int differences = 0;
    char ch;
    char ch2;
      
    if (caseSensitivity) 
    {
        // changes all characters to lower case
        int i = 0;
        while (i < sizeof(buffer1)) 
        {
            buffer1[i] = tolower(buffer1[i]);
            i++;
        }
        i = 0;
        while (i < sizeof(buffer2)) 
        {
            buffer2[i] = tolower(buffer2[i]);
            i++;
        }
    }

    for (size_t i = 0; i < sizeof(buffer1); i++) 
    {
        ch = buffer1[i];
        ch2 = buffer2[i];

        if (ch != '\n' && ch2 != '\n' && ch != '\0' && ch2 != '\0')
        {   

            if (ch != ch2)
            {
                differences++;
            }   
        } else 
        {
            break;
        }
    }

    return differences;
}
/**
 * @brief creates output file
 * 
 * @param filename name of the output file
 * @return empty write-only file
 */
static FILE* createOutputfile(const char *filename) 
{
    FILE *fp;
    fp = fopen(filename, "w");
    if (fp == NULL)
    {
        fprintf(stderr, "fopen failed: %s\n", strerror(errno), programName);
        exit(EXIT_FAILURE);
    }
    
    return fp;
}
/**
 * @brief usage of the program
 * 
 * Prints how the program is used properly, if the input was incorrect. 
 */
static void usage(void)
{
    fprintf(stderr, "Usage %s [-i] [-o outfile] file1 file2\n", programName);
    exit(EXIT_FAILURE);
}

/**
 * @brief main
 * 
 * @param argc the argument counter
 * @param argv the arguments
 * @return exit code success
*/
int main(int argc, char *argv[])
{
    programName = argv[0];
    
    char *filename1 = argv[argc - 2];
    char *filename2 = argv[argc - 1];

    int caseSensitive = 0;
    int outputFile = 0;
    
    int opt;
    char *outfilename;
    while ( (opt = getopt(argc, argv, "io:")) != -1)
    {
        switch ( opt ) 
        {
            case 'i':
                caseSensitive = 1;
                break;
            case 'o':
                outfilename = optarg;
                outputFile = 1;
                break;
            default: 
                usage();
                break;
        }
        
    }

    FILE *file1;
    FILE *file2;
    FILE *out;

    file1 = fopen(filename1, "r");
    file2 = fopen(filename2, "r");
    
    char buffer1[1024];
    char buffer2[1024];
    
    int currentLine = 1;
    int outputCreated = 0;
    
    while (fgets(buffer1, sizeof(buffer1), file1) && fgets(buffer2, sizeof(buffer2), file2))
    {
        int differences = compareLines(buffer1, buffer2, caseSensitive);
        if (differences > 0)
        {
            if (outputFile == 0) 
            {
                printf("Line: %i, characters: %i\n", currentLine, differences);
            } else if (outputFile == 1)
            {
                if (outputCreated == 0) {
                    out = createOutputfile(outfilename); 
                    outputCreated = 1;
                }
                fprintf(out, "Line: %i, characters: %i\n", currentLine, differences);           
            }
        }

        currentLine++;
    }

    fclose(file1);
    fclose(file2);
    if (outputFile) {
        fclose(out); 
    }
    return 0;
}
