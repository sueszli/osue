/**
 * @file main.c
 * @author Michael Gahleitner, 01633034
 * @brief 
 * @details 
 * @date 07.11.2021
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <getopt.h>

char *program_name;

typedef struct output_option_t
{
    char *result;
    char *filename;
    FILE *outfile;
} output_option_t;

typedef struct options
{
    int is_case_insensitive;
    char *comapare_string;
    output_option_t output;
} options_t;

/**
 * @brief Prints usage
 * @details Prints usage which displays the usage of options and arguments and exits with EXIT_FAILURE status.
 * @param void No parameters
 * @return No return value
 */
void usage(void)
{
    fprintf(stderr, "Usage: %s [-i] [-o outfile] keyword [file...]\n", program_name);
    exit(EXIT_FAILURE);
}

/**
 * @brief Handles malloc error.
 * @details Prints the error caused by malloc to stderr.
 * @param void No parameters
 * @return No return value
 */
void show_error_malloc(void)
{
    fprintf(stderr, "%s ERROR: malloc failed: %s\n", program_name, strerror(errno));
    exit(EXIT_FAILURE);
}

/**
 * @brief Closes file
 * @details Closes a file stream.
 * @param stream Name of the file stream to be closed.
 */
void close_file(FILE *stream)
{
    if (fclose(stream) == EOF)
    {
        fprintf(stderr, "%s ERROR: fclose failed: %s\n", program_name, strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void read_input(FILE *stream, options_t options)
{
    char *line = NULL;
    size_t len = 0;
    ssize_t nread = 0;

    while ((nread = getline(&line, &len, stream)) != -1)
    {
        for (int i = 0; i < nread - 1; i++)
        {
            if (line[i] == '\n')
            {
                line[i] = '\0';
            }
        }
        char *initial_line = malloc(sizeof(char) * nread);
        if (nread > 0 && initial_line == NULL)
        {
            show_error_malloc();
        }
        strcpy(initial_line, line);
        if (options.is_case_insensitive == 1)
        {
            for (int i = 0; i < nread; i++)
            {
                line[i] = tolower(line[i]);
            }
        }
        char *string_found = strstr(line, options.comapare_string);

        if (string_found != NULL)
        {
            fwrite(initial_line, nread, 1, options.output.outfile);
        }

        free(initial_line);
    }

    free(line);
}

/**
 * @brief Opens file
 * @details Opens a specified file in a specified mode and returns it's pointer.
 * @param filename Name of the file to be opened.
 * @param mode the mode the file should be opened in
 * @return pointer of the file
 */
FILE *open_file(char *filename, char *mode)
{
    FILE *file = NULL;

    if (filename != NULL)
    {
        file = fopen(filename, mode);
    }

    if (file == NULL)
    {
        fprintf(stderr, "%s ERROR: fopen failed: %s\n", program_name, strerror(errno));
        exit(EXIT_FAILURE);
    }

    return file;
}

/**
 * @brief Main function
 * @details Main function which parses options and positional arguments and initiates 
 * checkinput based on the optional arguments.
 * @param void No parameters
 * @return No return value
 */
int main(int argc, char *argv[])
{
    program_name = argv[0];

    int c;
    options_t options = {
        .is_case_insensitive = 0,
        .output = {
            .result = "",
            .filename = NULL,
            .outfile = stdout}};

    while ((c = getopt(argc, argv, "o:is")) != -1)
    {
        switch (c)
        {
        case 'o':
            options.output.filename = optarg;
            break;
        case 'i':
            options.is_case_insensitive = 1;
            break;
        case '?':
        default:
            usage();
            break;
        }
    }

    int arg_count = argc - optind;
    if (arg_count == 0)
    {
        usage();
    }
    else if (arg_count == 1)
    {
        options.comapare_string = argv[optind];
        if (options.is_case_insensitive == 1)
        {
            int cnt = 0;
            while (options.comapare_string[cnt] != '\0')
            {
                options.comapare_string[cnt] = tolower(options.comapare_string[cnt]);
                cnt++;
            }
        }
        if (options.output.filename != NULL)
        {
            options.output.outfile = open_file(options.output.filename, "w");
        }
        read_input(stdin, options);
    }
    else
    {
        if (options.output.filename != NULL)
        {
            options.output.outfile = open_file(options.output.filename, "w");
        }

        options.comapare_string = argv[optind];
        if (options.is_case_insensitive == 1)
        {
            int cnt = 0;
            while (options.comapare_string[cnt] != '\0')
            {
                options.comapare_string[cnt] = tolower(options.comapare_string[cnt]);
                cnt++;
            }
        }
        for (int arg_idx = (optind + 1); arg_idx < argc; arg_idx++)
        {
            FILE *input_file = open_file(argv[arg_idx], "r");
            read_input(input_file, options);
            close_file(input_file);
        }
    }
    if (options.output.outfile != stdout)
    {
        close_file(options.output.outfile);
    }

    exit(EXIT_SUCCESS);
}
