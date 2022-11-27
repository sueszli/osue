/**
 * @file mycompress.c
 * @author Raphael Pruckner <e11806918@student.tuwien.ac.at>
 * @date 11.11.2021
 * 
 * @brief Main program module
 * 
 * This program implements the functionality specified in Exercise 1A - MyCompress of the Betriebssysteme UE 2021.
 **/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <string.h>

char *prog_name;

/**
 * Mandatory usage function.
 * @brief This function writes helpful usage information about the program to stderr.
 * @details global variables: prog_name
 */
static void usage(void)
{
    fprintf(stderr, "Usage: %s [-o outfile] [file] \n Only one output option is allowed.", prog_name);
    exit(EXIT_FAILURE);
}

/**
 * Compression function.
 * @brief This function compresses a given input text by substituting subsequent identical
 * characters by only one occurence of the character followed by the number of occurences.
 * @details The function assumes all pointers for input and output files are valid.
 * @param in The pointer to the file which contains the input data (e.g. txt-file, stdin).
 * @param out The pointer to the file to which the output shall be written (e.g. txt-file, stdout).
 * @param read_count Counter which counts all read characters (for all input files) - needed for statistics.
 * @param written Counter which counts all written characters - needed for statistics.
 * @return EXIT_SUCCESS upon successfull compression or EXIT_FAILURE if an error occurs during writing 
 * (-1 being returned from fprintf()).
 */
int compress(FILE *in, FILE *out, int *read_count, int *write_count)
{
    int count = 0;
    int last = -1;
    int current;
    while ((current = fgetc(in)) != EOF)
    {
        *read_count = *read_count + 1;
        //if first character is read
        if (count == 0)
        {
            last = current;
            count++;
        }
        else
        {
            //print last character and set new character as current
            if (last != current)
            {
                int written = fprintf(out, "%c%d", last, count);
                if (written < 0)
                {
                    return EXIT_FAILURE;
                }
                *write_count = *write_count + written;
                last = current;
                count = 1;
            }
            else
            {
                count++;
            }
        }
    }
    //print last character to output as long as there is a character
    if (last != -1)
    {
        int written = fprintf(out, "%c%d", last, count);
        if (written < 0)
        {
            return EXIT_FAILURE;
        }
        *write_count = *write_count + written;
    }

    //stdout is buffered so stats could be shown before compressed file has completed output
    fflush(stdout);
    return EXIT_SUCCESS;
}

/**
 * Program entry point.
 * @brief This is the entry point for the mycompress program.
 * This function takes care of argument parsing as well as opening and closing files
 * @details After argument parsing, if an output file is specified, the file will be opened,
 * if not, stdout will be used to print the output.
 * If multiple output files are specified, the program shows proper usage and terminates.
 * A loop iterates over all input files and compresses each individual one with the compress()-function.
 * Finally, the statistics are calculated and printed at the end of the output file.
 * global variables: prog_name
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS.
 */
int main(int argc, char *argv[])
{
    prog_name = argv[0];
    int opt_o = 0;
    char *o_arg = NULL;
    int c;
    while ((c = getopt(argc, argv, "o:")) != -1)
    {
        if (c == 'o')
        {
            o_arg = optarg;
            opt_o++;
        }
        //if options other than 'o' are specified
        else
        {
            usage();
        }
    }

    //if multiple output files are specified
    if (opt_o > 1) {
        usage();
    }

    int input_file_count = argc - optind;
    char const *input_file_names[input_file_count];
    for (int i = 0; i < input_file_count; i++)
    {
        input_file_names[i] = argv[optind + i];
    }

    FILE *input_file;
    FILE *output_file;

    //open output file, if specified, otherwise open stdout as output file
    if (opt_o)
    {
        assert(o_arg != NULL);
        if ((output_file = fopen(o_arg, "w")) == NULL)
        {
            fprintf(stderr, "[%s] ERROR: fopen failed: %s\n", prog_name, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        output_file = stdout;
    }

    int read_count = 0;
    int write_count = 0;
    //iterate over array of input file names and compress each using the compress()-function
    for (int i = 0; i < input_file_count; i++)
    {
        //open input file
        if ((input_file = fopen(input_file_names[i], "r")) == NULL)
        {
            fprintf(stderr, "[%s] ERROR: fopen failed: %s\n", prog_name, strerror(errno));
            fclose(output_file);
            exit(EXIT_FAILURE);
        }

        //compress file
        if (compress(input_file, output_file, &read_count, &write_count) == -1)
        {
            fprintf(stderr, "[%s] ERROR: compression failed: %s\n", prog_name, strerror(errno));
            fclose(output_file);
            fclose(input_file);
            exit(EXIT_FAILURE);
        }
        fclose(input_file);
    }

    //if no input file is specified, input is read from stdin
    if (input_file_count == 0)
    {
        if (compress(stdin, output_file, &read_count, &write_count) == -1)
        {
            fprintf(stderr, "[%s] ERROR: compression failed: %s\n", prog_name, strerror(errno));
            fclose(output_file);
            exit(EXIT_FAILURE);
        }
    }

    if (opt_o)
    {
        fclose(output_file);
    }

    //calculate statistics
    float ratio = 0;
    if (read_count != 0)
    {
        ratio = ((float)write_count / read_count) * 100.0;
    }
    fprintf(stderr, "\nRead: %d characters\nWritten: %d characters\nCompression ratio: %f", read_count, write_count, ratio);
    return EXIT_SUCCESS;
}
