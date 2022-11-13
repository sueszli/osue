/**
 * @file mycompress.c
 * @author Lulu Wang 01633998
 * @date 12.11.2021
 *
 * @brief Main program module.
 * 
 * This program compress the input and write it to the output.
 **/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief short usage description 
 * @param prog_name of program
 **/
void usage(const char prog_name[])
{
    fprintf(stderr, "Usage: %s [-o outfile] [file...]\n", prog_name);
    exit(EXIT_FAILURE);
}

/**
 * The compress function
 * @brief Read and compress the file(s)/stdin input and counts the written and read character.
 * @param in is either file with lines to compress or stdin
 * @param out is the output file with the results or stdout
 * @param read Pointer to counter for reading characters
 * @param written Pointer counter for writting characters
 * @return 0 if success, or -1 failed.
 */
int compress(FILE *in, FILE *out, uint64_t *read, uint64_t *written)
{
    // counts chars in line (til eof) for compression
    int char_counter = 0; 
    int last_char;
    while (true)
    {
        int current_char = fgetc(in);
        if (feof(in))
        {
            break;
        }
        *read = *read + 1;
        // The first character
        if (char_counter == 0)
        {
            char_counter = 1;
            last_char = current_char;
            continue;
        }
        if (current_char == last_char)
        {
            char_counter++;
            continue;
        }
        // new character in line, print out the result count for last_char char
        int tmp = fprintf(out, "%c%d", last_char, char_counter);
        if (tmp == -1)
        {
            return -1;
        }
        *written += tmp;
        // Reset for the next char
        last_char = current_char;
        char_counter = 1;
    }

    // Write the last_char character to the file
    int tmp = fprintf(out, "%c%d", last_char, char_counter);
    if (tmp == -1)
    {
        return -1;
    }
    *written += tmp;
    return 0; //success
}

/**
 * @brief This is the start of the program. It reads the command line, open/close 
 * the files and compress the input. Lastly it prints out the results.
 * @param argc counts the arguments
 * @param argv holds programm's arguments and options
 * @return EXIT_SUCCESS or EXIT_FAILURE.
 **/
int main(int argc, char *argv[])
{
    const char *out_filename = NULL;
    int option_val = 0;
    const char *const progname = argv[0];
    //getopt reads the flags
    while ((option_val = getopt(argc, argv, "o:")) != -1)
    {
        //save the output file name if available 
        switch (option_val)
        {
        case 'o':
            if (out_filename != NULL)
            {
                fprintf(stderr, "Error: -o can be only used once!\n");
                usage(progname);
            }
            out_filename = optarg;
            break;
        default:
            // invalid option
            usage(progname);
            break;
        }
    }
    // optind is the index of the next element to be processed in argv.
    // gets the input file(s)
    int input_len = argc - optind;
    char const *input_filenames[input_len];
    for (int i = 0; i < input_len; i++)
    {
        input_filenames[i] = argv[optind + i];
    }
    // Open the output file
    FILE *out_file = stdout;
    if (out_filename != NULL)
    {
        out_file = fopen(out_filename, "w");
        if (out_file == NULL)
        {
            fprintf(stderr, "Error: couldnt open output file %s: %s\n", out_filename, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    // Existing input file(s), read them and compress all
    uint64_t read = 0;
    uint64_t written = 0;
    for (int i = 0; i < input_len; i++)
    {
        FILE *in_file = fopen(input_filenames[i], "r");
        if (in_file == NULL)
        {
            fprintf(stderr, "Error: couldnt open input file %s: %s\n", input_filenames[i], strerror(errno));
            fclose(out_file);
            exit(EXIT_FAILURE);
        }
        if (compress(in_file, out_file, &read, &written) == -1)
        {
            fprintf(stderr, "Error: couldnt compress file %s: %s\n", input_filenames[i], strerror(errno));
            fclose(in_file);
            fclose(out_file);
            exit(EXIT_FAILURE);
        }
        fclose(in_file);
    }

    // stdin
    if (input_len == 0)
    {
        if (compress(stdin, out_file, &read, &written) == -1)
        {
            fprintf(stderr, "Error: An error occurred while compressing file stdin: %s\n", strerror(errno));
            fclose(out_file);
            exit(EXIT_FAILURE);
        }
    }

    // Close the output file
    if (out_file != stdout)
    {
        fclose(out_file);
    }

    // Print the statistics
    fprintf(stderr, "Read: %7lu characters\n", read);
    fprintf(stderr, "Written: %4lu chararcets\n", written);
    fprintf(stderr, "Compression ratio: %4.1f%%\n", ((double)(written) / read) * 100.0);
    return EXIT_SUCCESS;
}
