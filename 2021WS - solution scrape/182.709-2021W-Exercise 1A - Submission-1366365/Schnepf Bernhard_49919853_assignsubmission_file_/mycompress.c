#include <stdio.h>
#include <string.h>
#include <stdlib.h> //For EXIT_SUCCESS / EXIT_FAILURE
#include <unistd.h> //getopt

/**
 * @author Bernhard Schnepf, 12022508
 * @brief Compresses input files into one output file
 * @details
 * @date 03.11.2021
 */

/**
 * @brief Function that prints out the USAGE and exits
 * @details The function prints out the USAGE and a detailed EXAMPLE for it. Then it exits with EXIT_FAILURE. 
 */

char *program_name;
FILE *input, *output;

void input_error()
{
    fprintf(stderr, "USAGE: %s  [-o outfile] [file...]\n", program_name);
    fprintf(stderr, "EXAMPLE: %s -o output.txt input.txt\n", program_name);
    exit(EXIT_FAILURE); //exit() terminates the calling process immedeately, return only the function
}

/**
 * @brief This function closes the opened files at exit.
 * 
 */
void closeFiles()
{
    if (input != NULL && input != stdin)
    {
        fclose(input);
    }
    if (input != NULL && output != stdout)
    {
        fclose(output);
    }
}

/**
 * @brief Compresses the data given
 * @details The function first checks the arguments and handles accordingly.
 * no input defined = stdin
 * no output defined = stdout
 * oterwise set it to the defined input or output
 * Then it starts reading in the data that shall be compressed. (Had a few issues with windows / linux + a vowi answer was a little wrong after all).
 * Fgetc gets every single character and if the following character is the same, then count is increased.
 * If the next character is not the same as the last character, then print the last character with its count.
 * Do this until EOF (End of file) is reached.
 * At the end print out a summary of the compressed data. (Then it goes to atexit(closeFiles) and the input and output if still open.
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
//argc refers to the number of command line arguments passed, which includes the actual name of the program
// argv contains the arguments [0] program name, [1] rest of the arguments
int main(int argc, char *argv[])
{
    program_name = argv[0];
    char ch, lastChar;
    int read = 0, written = 0, count = 1, fileStart = 1, c = 0;
    FILE *input = stdin, *output = stdout;

    //Close files after exit
    if (atexit(closeFiles) < 0)
    {
        fprintf(stderr, "ERROR: %s Couldn't set up shut_down function!\n", program_name);
        exit(EXIT_FAILURE);
    }

    //: for entries after it
    //Checks input arguments
    while ((c = getopt(argc, argv, "o:")) != -1)
    {
        switch (c)
        {
        case 'o':
            if (output != stdout)
            {
                fprintf(stderr, "ERROR: %s there can only be 1 output file defined!\n", program_name);
                input_error();
            }
            else
            {
                //Open file and handle error if not a valid file
                output = fopen(optarg, "w");
                if (output == NULL)
                {
                    fprintf(stderr,"ERROR: %s opening file %s failed!\n", program_name, optarg);
                    exit(EXIT_FAILURE);
                }
                fileStart = 3;
            }
            break;
        case '?':
            //another option was used, that should not be there!
            input_error();
            break;
        default:
            //Won't go in here.. but because of guidelines
            break;
        }
    }

    if ((fileStart + 1) > argc)
    {
        fileStart = 0;
    }

    //Open input files until
    while (argc > fileStart)
    {
        if (fileStart != 0)
        {
            if (input != NULL && input != stdin)
            {
                fclose(input);
            }
            //Open file and handle error if not a valid file
            input = fopen(argv[fileStart], "r");
            if (input == NULL)
            {
                fprintf(stderr,"ERROR: %s opening file %s failed!\n", program_name, argv[fileStart]);
                exit(EXIT_FAILURE);
            }
        }
        lastChar = ch = fgetc(input);
        if (ch != EOF)
        {
            read += 1;
        }
        while ((ch = fgetc(input)) != EOF && lastChar != EOF)
        {
            read += 1;
            //Increase char if same char as before
            if (ch == lastChar)
            {
                count += 1;
            }
            //Print if the last char and the char after that are not the same and start from new
            else
            {
                fprintf(output, "%c%d", lastChar, count);
                count = 1;
                written += (2 + count / 10);
            }
            lastChar = ch;
        }
        //Print last character
        if (lastChar != EOF)
        {
            written += 2 + count / 10;
            fprintf(output, "%c%d", lastChar, count);
        }
        if (fileStart == 0)
        {
            break;
        }
        fileStart += 1;
        count = 1;
    }

    float ratio = (float)written / read * 100;
    //This is to prevent nan
    if (read == 0)
    {
        ratio = 0;
    }
    fprintf(stderr, "Read:      %d characters\n", read);
    fprintf(stderr, "Written:   %d characters\n", written);
    fprintf(stderr, "Compression ratio: %.1f%%\n", ratio);

    exit(EXIT_SUCCESS);
}