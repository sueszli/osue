/**
 * @file mycompress.c
 * @author Manuel Hochreiter e0671428@student.tuwien.ac.at
 * @brief Main program module.
 * 
 * This program reads an input text, compresses it and provides the new text as an output. Further information like the compression rate is also given.
 * 
 * @version 0.1
 * @date 2021-11-10
 * 
 * @copyright Copyright (c) 2021
 * 
 **/

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

/* Those two variables are needed for the information regarding the compression ratio. 
newSize counts the amount of letters after compression. originalSize takes are of counting the amount of letters before compression.*/
static int newSize = 0;
static int originalSize = 0;
//program_name contains the name of the program
static char *program_name = "mycompress";

/**
 * @brief This function takes the input from the file *file is pointing to. Then it compresses the content the following way: 
 * "The input is compressed by substituting subsequent identical characters by only one occurence of the character followed by the number of characters."
 * @details global variables:
 * currentChar: contains the current char read from the function.
 * count: counts how many occurences of a letter there are in a row.
 * lineLength: is given its value by the function getline which defines it as the length of the read line.
 * buffer: contains the line which is currently read.
 * ret: is the return value of the reading function.
 * 
 * @param file a pointer to file we want to compress
 * @param output a pointer to the file where the compressed input should get written into
 * @return void: this function only executes and does not return anything other than void
 **/
static void compressing(FILE *file, FILE *output)
{
    char currentChar = 'c';
    int count = 0;
    size_t lineLength = 0;
    char *buffer = NULL;
    int ret;

    //We start by reading every line of the file line by line
    while ((ret = getline(&buffer, &lineLength, file)) != -1)
    {
        //if ret = 1 we have an empty line, so a '\n', hence we increase the count and the count of characters read
        if (ret == 1)
        {
            count++;
            originalSize++;
        }
        //else we have a line with at least one character other than '\n'
        else
        {
            //here we look at each character of the line individually
            for (int i = 0; i <= lineLength; i++)
            {
                //if the character we look at is a letter or the '\n' it is seen as valid input
                if ((buffer[i] > 64 && buffer[i] < 123) || buffer[i] == '\n')
                {
                    //if the current character is identical to the one before we increase the count by one
                    if (buffer[i] == currentChar)
                    {
                        count++;
                    }
                    else
                    //if it is a different character we print the char from before with its count and refill currentChar with the current letter and reset the count to one
                    //also we increase the count of new letters by 2
                    {
                        if (count > 0)
                        {
                            fprintf(output, "%c", currentChar);
                            fprintf(output, "%d", count);
                        }
                        newSize = newSize + 2;
                        currentChar = buffer[i];
                        count = 1;
                    }
                    originalSize++;
                }
            }

            //to not forget about the end of each line we repeat the "else" block from before
            if (currentChar > 64 && currentChar < 123)
            {
                if (count > 0)
                {
                    fprintf(output, "%c", currentChar);
                    fprintf(output, "%d", count);
                }
                newSize = newSize + 2;
                currentChar = buffer[lineLength - 1];
                count = 1;
            }
        }
        //to make sure everything compressed is written in the output as soon as possible we flush the output after every line
        if (fflush(output) != 0)
        {
            fprintf(stderr, "fflush failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    fprintf(output, "%c", currentChar);
    fprintf(output, "%d", count);

    free(buffer);
}

/**
 * @brief This function provides information about calling it correctly to stderr
 * @details global variables: program_name
 */
void usage(void)
{
    fprintf(stderr, "Usage: %s [-o outfile] [infiles]\n", program_name);
    exit(EXIT_FAILURE);
}

/**
 * @brief The main function is the program entry point. It considers the optional arguments if provided, opens an closes the streams and calls the compressing algorithm.
 * @details global variables: 
 * c: provides the return value of getopt
 * outputName: contains the name of the output file if given
 * inputFilename: an array containing the names of the input files if given
 * in, output: pointers pointing to the input and output file
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char *argv[])
{
    int c;
    char *outputName;
    char *inputFileNames[argc - optind];

    while ((c = getopt(argc, argv, "o:")) != -1)
    {
        switch (c)
        {
        case 'o':
            outputName = optarg;
            break;
        case '?':
            usage();
            break;
        default:
            usage();
            break;
        }
    }

    //if input file names are given we fill the inputFilenames array with them
    for (int i = optind; i < argc; i++)
    {
        inputFileNames[i - optind] = argv[i];
    }

    FILE *in, *output;

    //next we try to open the possible input and output files and associate them with the returned stream
    if (optind == 3)
    {
        output = fopen(outputName, "w");
        if (output == NULL)
        {
            fprintf(stderr, "output fopen failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        output = fdopen(1, "w");
        if (output == NULL)
        {
            fprintf(stderr, "output fdopen failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    if (optind == argc)
    {
        in = fdopen(0, "r");
        if (in == NULL)
        {
            fprintf(stderr, "input fdopen failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        compressing(in, output);
    }
    else
    {
        for (int i = 0; i < argc - optind; i++)
        {
            in = fopen(inputFileNames[i], "r");
            if (in == NULL)
            {
                fprintf(stderr, "input fopen failed: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
            compressing(in, output);
        }
    }

    //here we print several information about the compressing process:
    fprintf(output, "\n");
    fprintf(output, "Read: %d characters\n", originalSize);
    fprintf(output, "Written: %d characters\n", newSize);
    int ratio = newSize * 100 / originalSize;
    fprintf(output, "Compression ratio: %d percent \n", ratio);
    fprintf(output, "\n");

    //last we close the streams again
    if (fclose(in) != 0)
    {
        fprintf(stderr, "fclose failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (fclose(output) != 0)
    {
        fprintf(stderr, "fclose failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}