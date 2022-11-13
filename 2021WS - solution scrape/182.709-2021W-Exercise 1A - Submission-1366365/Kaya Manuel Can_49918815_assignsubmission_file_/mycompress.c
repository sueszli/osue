/**
 * @file mycompress.c
 * @author Manuel Can Kaya 12020629
 * @brief mycompress program for assignment 1A mycompress
 * @details Program reads input provided either from files or stdin, compresses that
 * and prints it out either to a provided file or stdout.
 * @version 0.1
 * @date 2021-11-14
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>

/**
 * @brief Compresses input and writes to output.
 * @details Reads input from 'in', compresses that and writes the output to 'out'. Keeps track
 * of total read and written characters in 'num_of_read_chars' and 'num_of_written_chars'.
 * 
 * @param in Input file stream.
 * @param out Output file stream.
 * @param num_of_read_chars Writes number of read characters here (increments it).
 * @param num_of_written_chars Writes number of written characters here (increments it).
 * @return int Returns -1, if an error occurrs and 1 otherwise.
 */
static int compress(FILE *in, FILE *out, int *num_of_read_chars, int *num_of_written_chars);

/**
 * @brief Calculates number of characters in a number.
 * @details Calculates number of characters in a number 'num' and returns it.
 * 
 * @param num Number, whose characters are determined.
 * @return int Returns number of characters in 'num'.
 */
static int num_of_chars_in_number(int num);

/**
 * @brief Checks if a file exists.
 * @details Checks if a file 'file_path' exists or not.
 * 
 * @param file_path File to be checked.
 * @return int Returns -1, if file does not exist, -2, if some other error occurrs and 1 otherwise.
 */
static int file_exists(const char *file_path);

/**
 * @brief Prints an error to stderr.
 * @details Prints a formated string to stderr for error messages. The error message
 * includes the program name 'program_name', a brief message 'brief_msg' and a more detailed
 * message 'detail_msg'.
 * 
 * @param program_name Name of the program.
 * @param brief_msg Brief error message.
 * @param detail_msg Detailed error message.
 */
static void error_msg(const char *program_name, const char *msg, const char *detail_msg);

/**
 * @brief Prints the usage message to stderr.
 * @details Prints the usage message 'Usage: mycompress [-o outfile] [file...]' to stderr.
 * 
 */
static void usage(void);


/** Global variable for the name of the program. */
static char *program_name = NULL;


int main(int argc, char *argv[])
{
    program_name = argv[0];

    // Parse command line arguments.
    char c;
    char *outfile = NULL;
    while ((c = getopt(argc, argv, "o:")) != -1)
    {
        switch (c)
        {
            case 'o':
                if (outfile != NULL)
                {
                    // The o-option has been given multiple times.
                    usage();
                    exit(EXIT_FAILURE);
                }
                else
                {
                    outfile = optarg;
                }
                break;
            case '?':
                // Invalid option has been given.
                usage();
                exit(EXIT_FAILURE);
            default:
                assert(0);
                break;
        }
    }

    FILE *in;
    FILE *out;

    // Open outfile.
    if (outfile == NULL)
    {
        // No output file was provided, therefore stdout is the output.
        out = stdout;
    }
    else
    {
        out = fopen(outfile, "w");
        if (out == NULL)
        {
            // Error occured while opening the file.
            error_msg(program_name, "fopen() failed", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    bool success = true;
    int num_of_read_chars = 0;
    int num_of_written_chars = 0;

    // Compress.
    if (optind == argc)
    {
        // No input files were provided, therefore stdin is the input.
        in = stdin;
        if (compress(in, out, &num_of_read_chars, &num_of_written_chars) < 0)
        {
            success = false;
        }
    }
    else
    {
        bool files_exist = true;
        for (int i = optind; i < argc; i++)
        {
            if (file_exists(argv[i]) < 0)
            {
                files_exist = false;
                success = false;
            }
        }

        if (files_exist == true)
        {
            for (int i = optind; i < argc && success == true; ++i)
            {
                in = fopen(argv[i], "r");
                if (in == NULL)
                {
                    // Error occured while opening the file.
                    error_msg(program_name, "fopen() failed", strerror(errno));
                    success = false;
                }
                else
                {
                    if (compress(in, out, &num_of_read_chars, &num_of_written_chars) < 0)
                    {
                        success = false;
                    }

                    if (fclose(in) == EOF)
                    {
                        error_msg(program_name, "fclose() failed", strerror(errno));
                        success = false;
                    }
                }
            }
        }
    }

    if (fclose(out) == EOF)
    {
        error_msg(program_name, "fclose() failed", strerror(errno));
        success = false;
    }

    if (success == true)
    {
        fprintf(
            stderr,
            "Read: %d characters\nWritten: %d characters\nCompression ratio: %.1f%%\n",
            num_of_read_chars,
            num_of_written_chars,
            num_of_written_chars / (float)num_of_read_chars * 100
        );
        exit(EXIT_SUCCESS);
    }
    else
    {
        exit(EXIT_FAILURE);
    }
}

static int compress(FILE *in, FILE *out, int *num_of_read_chars, int *num_of_written_chars)
{
    char c;
    char prev_c;
    int c_counter = 0;

    while ((c = fgetc(in)) != EOF)
    {
        (*num_of_read_chars) = (*num_of_read_chars) + 1;

        if (c_counter == 0)
        {
            // First character read.
            prev_c = c;
            c_counter++;
        }
        else if (c == prev_c)
        {
            c_counter++;
        }
        else
        {
            if (fprintf(out, "%c%d", prev_c, c_counter) < 0)
            {
                // An error occurred.
                error_msg(program_name, "fprintf() failed", strerror(errno));
                return -1;
            }
            (*num_of_written_chars) = (*num_of_written_chars) + 1 + num_of_chars_in_number(c_counter);

            prev_c = c;
            c_counter = 1;
        }
    }

    if (ferror(in) != 0)
    {
        // Loop was broken out of because of error and not EOF.
        error_msg(program_name, "fgetc() failed", strerror(errno));
        return -1;
    }

    // Last character needs to be printed too.
    if (c_counter != 0)
    {
        if (fprintf(out, "%c%d", prev_c, c_counter) < 0)
        {
            // An error occurred.
            error_msg(program_name, "fprintf() failed", strerror(errno));
            return -1;
        }
        (*num_of_written_chars) = (*num_of_written_chars) + 1 + num_of_chars_in_number(c_counter);
    } 

    return 1;
}

static int num_of_chars_in_number(int num)
{
    int counter = 1;
    while (num >= 10)
    {
        counter++;
        num = num % 10;
    }
    return counter;
}

static int file_exists(const char *file_path)
{
    int result = 1;
    FILE *file = fopen(file_path, "r");
    if (file == NULL)
    {
        if (errno == ENOENT)
        {
            // File does not exist.
            result = -1;
        }
        else
        {
            // Other error.
            result = -2;
        }
        error_msg(program_name, "fopen() failed", strerror(errno));
    }
    else
    {
        if (fclose(file) == EOF)
        {
            error_msg(program_name, "fclose() failed", strerror(errno));
            result = -2;
        }
    }

    return result;
}

static void error_msg(const char *program_name, const char *brief_msg, const char *detail_msg)
{
    fprintf(stderr, "[%s]: %s: %s\n", program_name, brief_msg, detail_msg);
}

static void usage(void)
{
    fprintf(stderr, "Usage: %s [-o outfile] [file...]\n", program_name);
}
