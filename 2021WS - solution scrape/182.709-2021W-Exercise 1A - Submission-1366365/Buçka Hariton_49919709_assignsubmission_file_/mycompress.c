/**
 * @file mycompress.c
 * @author Hariton Bucka (01529018)
 * @brief  mycompress takes input data and compresses it.
 * @version 1.0
 * @date 2021-11-12
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <errno.h>

char const *program_name;

/**
 * @brief boolean implementation with values false and true as enum
 */
typedef enum
{
    false,
    true
} boolean;

/**
 * @brief mycompress reads from the input, compresses the input and writes it out to the output.
 * @details this function reads from the input source which may be a file or the stdin. The input
 *          is processed and compresssed by replacing repeating characters with numbers. The compressed
 *          input is then written to the output file or stdout, depending on the provided output.
 * @param input contains the file to read from.
 * @param output contains the file to write to.
 * @return an integer array containin two values: nr. of characters that were read and nr. of characters that were written.
 */
int *mycompress(FILE *input, FILE *output)
{
    int readChars = 0;
    int writtenChars = 0;
    int *result;
    result = malloc(2 * sizeof *result);
    int count = 0; // counts how often a char is repeated
    int prev;

    while (1)
    {

        int current = fgetc(input); // get current character

        //terminate loop when EOF is reached
        if (feof(input))
        {
            break;
        }

        readChars++; //count nr. of read charachters

        //set the the prev value for comparison
        if (count == 0)
        {
            prev = current;
            count++;
        }
        else
        {
            //count character occurences
            if (current == prev)
            {
                count++;
            }
            //write out when character stops repeating
            else
            {
                int chars_compressed = fprintf(output, "%c%d", prev, count);
                writtenChars = writtenChars + chars_compressed;

                //reset values for next iteration
                prev = current;
                count = 1;
            }
        }
    }

    // Write the last character to the file
    int chars_compressed = fprintf(output, "%c%d", prev, count);
    writtenChars = writtenChars + chars_compressed;
    printf("\n");

    result[0] = readChars;
    result[1] = writtenChars;

    return result;
}

/**
 * @brief The main function of the program, which is also the starting point.
 * 
 * @param argc contains number of arguments.
 * @param argv contains array of all arguments.
 * @return EXIT_SUCCESS if everything is correct, otherwise EXIT_FAILURE
 */
int main(int argc, char *argv[])
{
    program_name = argv[0]; //set program name
    char *outputName;
    boolean outFile = false;
    int opt_o = 0;

    int c;

    while ((c = getopt(argc, argv, "o:")) != -1)
    {
        switch (c)
        {
        case 'o':
            outFile = true;
            outputName = optarg;
            opt_o++;
            break;
        case '?':
            fprintf(stderr, "Usage: %s [-o outfile] [file...]\n", program_name);
            exit(EXIT_FAILURE);
            break;
        default:
            assert(0);
        }
    }

    if (opt_o > 1)
    {
        fprintf(stderr, "Usage: %s [-o outfile] [file...]\n", program_name);
        exit(EXIT_FAILURE);
    }

    int input_files_nr = argc - optind;
    char *file_names[input_files_nr];

    if (input_files_nr > 0)
    {
        for (int i = 0; i < input_files_nr; i++)
        {
            file_names[i] = argv[optind + i];
        }
    }

    //set output stdout/File with outputname
    FILE *output = stdout;

    if (outFile)
    {
        output = fopen(outputName, "w");
        if (output == NULL)
        {
            fprintf(stderr, "%s: Could not open file with name %s. %s\n", program_name, outputName, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    int read_total = 0;
    int written_total = 0;

    //read from stdin if no files are given and compress
    if (input_files_nr == 0)
    {
        int *compress = mycompress(stdin, output);
        read_total = read_total + compress[0];
        written_total = written_total + compress[1];
    }

    //read input from files and compress
    for (int i = 0; i < input_files_nr; i++)
    {
        FILE *input = fopen(file_names[i], "r");
        if (input == NULL)
        {
            fprintf(stderr, "%s: Could not open file with name %s. %s\n", program_name, file_names[i], strerror(errno));
            exit(EXIT_FAILURE);
        }

        int *compress = mycompress(input, output);
        read_total = read_total + compress[0];
        written_total = written_total + compress[1];

        fclose(input);
    }

    // Close the output file if it wasn't stdout
    if (outFile)
    {
        fclose(output);
    }

    fprintf(stderr, "Read: \t\t%d characters\nWritten: \t%d characters\nCompression ratio: \t%.1f%% \n", read_total, written_total, ((double)(written_total) / read_total) * 100.0);
    return EXIT_SUCCESS;
}
