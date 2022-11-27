/**
 * @file mycompress.c
 * @author MAYERL Thomas, 52004289 <thomas.mayerl@tuwien.ac.at>
 * @date 20.10.2021
 * 
 * @brief Program mycompress: This program compresses an input of files with run-length encoding
 * 
 * @details This program reads from either input files or the stdin and writes to either an output file or the stdout. 
 * The output will contain the data of the input compressed with run-length encoding.
 * 
 **/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/errno.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>

char *prog_name; /**< The program name */

/**
 * Print error function
 * @brief This function takes an error message as a parameter and writes it to stderr. Additionally, it takes a detail message.
 * @details global variables: prog_name: The program name
 * @param error_msg The error message
 * @param detail_msg More details of the error - often obtained with strerror and errno
 **/
static void print_error(char *error_msg, char *detail_msg) 
{
    fprintf(stderr, "[%s] %s: %s\n", prog_name, error_msg, detail_msg);
}

/**
 * Usage function
 * @brief This function prints information on how to use the program.
 * @details global variables: prog_name: The program name
 **/
static void usage(void)
{
    fprintf(stderr,"Usage: %s [-o outfile] [file...]\n", prog_name);
    exit(EXIT_FAILURE);
}

/**
 * Compress function
 * @brief This function compresses the data of an input file (or stdin) to the output file (or stdout).
 * @details The number of characters read and written will be counted.
 * @param input The input file
 * @param output The output file
 * @param read_sum The pointer to the number of characters read
 * @param write_sum The pointer to the number of characters written
 **/
static void compress_file(FILE *input, FILE *output, int *read_sum, int *write_sum)
{
    char current_char = -1; // No character was read yet
    int char_cnt = 0;

    // Read until EOF is reached. Then check if EOF was given because the end of the file was reached or an error occurred.
    // The break condition can't be checked here because we have to differentiate between those two cases.
    while(1)
    {
        char c = fgetc(input);
        if (c == EOF)
        {
            if (ferror(input))
            {
                // Error
                if (errno == EINTR) {
                    continue; //Interrupt occurred - try to read character again
                }
                // Another error occurred - Terminate program but close files before - if possible
                fclose(input);
                fclose(output);
                print_error("Error: Could not read file", strerror(errno));
                exit(EXIT_FAILURE);
            }
            // EOF occurred - Break out of loop
            break;
        }
        (*read_sum)++;
        if (current_char == c)
        {
            char_cnt++;
        } else 
        {
            if (current_char != -1)
            {
                int written_chars = 0;
                if((written_chars = fprintf(output, "%c%d", current_char, char_cnt)) < 0)
                {
                    //Terminate program but close files before - if possible
                    fclose(input);
                    fclose(output);
                    print_error("Error: Could not write to file", strerror(errno));
                    exit(EXIT_FAILURE);
                }
                (*write_sum) += written_chars;
            }
            // Prepare variables for next character
            current_char = c;
            char_cnt = 1;
        }
    }
    // If EOF was reached, the last found character still has to be written - if there is any
    if (char_cnt > 0 && current_char != -1)
    {
        (*write_sum)+=2; // We write two characters - the char and the number of replaced chars
        if(fprintf(output, "%c%d", current_char, char_cnt) < 0)
        {
            // Try to close files
            fclose(input);
            fclose(output);
            print_error("Error: Could not write to file", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
}

/**
 * Program entry point.
 * @brief The program starts here and will take the options and arguments from the console.
 * @details Option -o OUTPUT will set the output location to the file OUTPUT. If no output
 * is given, stdout will be used instead. The additional arguments describe the input files.
 * If no input file is fiven, stdin will be used instead.
 * global variables: prog_name: The program name - set by this function
 * @param argc The number of arguments
 * @param argv The arguments
 * @return EXIT value
 **/
int main(int argc, char *argv[])
{
    prog_name = argv[0];
    // opt_o variable in order to count whether the user doesn't pass too many o-options
    int opt_o = 0;
    char *arg_o = NULL;
    int c;
    while((c = getopt(argc, argv, "o:")) != -1)
    {
        switch (c)
        {
            case 'o':
                arg_o = optarg;
                opt_o++;
                break;
            case '?':
                // Unknown option found
                usage();
                break;
            default:
                assert(0); //Should not happen

        }
    }
    if (opt_o > 1)
    {
        usage();
    }
    FILE *output;
    // Open output file if one is given
    if (arg_o != NULL)
    {
        output = fopen(arg_o, "w");
        if (output == NULL)
        {
            print_error("Error: Could not open file", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    // Use stdout for output if no output file was given
    else
    {
        output = stdout;
    }

    int read_sum = 0;
    int write_sum = 0;
    // Compress stdin if no input files were given
    if (optind >= argc)
    {
        compress_file(stdin, output, &read_sum, &write_sum);
    }
    // Set argv to first non-option argument
    argv += optind;
    // Read every input file and compress it
    while(optind++ < argc)
    {
        FILE *input = fopen(*argv, "r");
        if (input == NULL)
        {
            print_error("Error: Could not open file", strerror(errno));
            exit(EXIT_FAILURE);
        }
        compress_file(input, output, &read_sum, &write_sum);
        if (fclose(input) != 0)
        {
            print_error("Error: Could not close file", strerror(errno));
            exit(EXIT_FAILURE);
        }
        argv++;
    }
    
    // Write read and written characters to stderr
    fprintf(stderr, "%-10s%d characters\n%-10s%d characters\nCompression ratio: %.2f%%\n", "Read:", read_sum, "Written:", write_sum, (read_sum == 0) ? 100.0 : (float)write_sum/read_sum*100);
    
    // Close the output file if one was given
    if (arg_o != NULL) 
    {
        fclose(output);
    }
    return EXIT_SUCCESS;
    
}
