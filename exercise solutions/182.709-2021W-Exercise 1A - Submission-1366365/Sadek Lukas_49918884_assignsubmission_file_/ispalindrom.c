/**
 * @brief excercise 1a for OSUE 2021W
 * @author Lukas Sadek (11813847)
 * @file ispalindrom.c
 * @date 14.11.2021
 * @details The program ispalindrom shall read files line by line
 * and for each line check whether it is a palindrom,
 * i.e. whether the text read backwards is identical to itself.
 * Each line shall be printed followed by the text
 * "is a palindrom" if the line is a palindrom or "is not a palindrom" if the line is a not palindrom.
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>

// stores program name
char *myprog;

/**
 * @brief prints out a usage message to stderr and exits with EXIT_FAILURE
 * 
 */
static void usage(void) {
    fprintf(stderr, "Usage: %s [-s] [-i] [-o outfile] [file...]\n", myprog);
    exit(EXIT_FAILURE);
}

/**
 * @brief prints out given string to stderr and prints usage message and exits with EXIT_FAILURE
 * 
 * @param error_msg the string to print
 */
static void print_error(char* error_msg)
{
    fprintf(stderr, "%s\n", error_msg);
    usage();
}

/**
 * @brief takes the given string and makes it lowercase
 * 
 * @param line the string to format
 */
static void to_lower_case(char* line)
{
    while (*line != '\0')
    {
        *line = tolower(*line);
        line++;
    }
}

/**
 * @brief removes all whitespaces in given string
 * 
 * @param line the string to remove all whitespaces
 */
static void remove_whitespaces(char* line)
{
    int i = 0, j = 0;
	while (line[i] != '\0')
	{
		if (line[i] != ' ')
        {
            line[j++] = line[i];
        }
		i++;
	}
	line[j] = '\0';
}

/**
 * @brief checks if given string is a palindrome
 * 
 * @param first points to the first char of the string
 * @return 0 if given string is no palindrome, otherwise 1
 */
static int is_palindrom(char* first)
{
    // pointer to last index ('\0' excluded)
    char* last = first + strlen(first) - 1;

    while (first < last)
    {
        if (*first != *last)
        {
            return 0;
        }
        first++; last--;
    }
    return 1;
}

/**
 * @brief checks file line by line if the line is a palindrome
 * 
 * @param input file to read from
 * @param output file to write to
 * @param s if s is 1 then whitespaces should be ignored
 * @param i if i is 1 then the palindrome check should be case insensitive
 */
static void write_file(FILE* input, FILE* output, int s, int i)
{
    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;
    while ((linelen = getline(&line, &linecap, input)) > 0)
    {
        // if last character is '\n' then change it to '\0'
        if (line[strlen(line) - 1] == '\n')
        {
            line[strlen(line) - 1] = '\0';
        }

        if (strlen(line) != 0)
        {
            // save unmodified string
            char* og_str = malloc(sizeof(*og_str));
            strncpy(og_str, line, strlen(line));

            // remove whitespaces
            if (s == 1)
            {
                remove_whitespaces(line);
            }

            // lower case everything
            if (i == 1)
            {
                to_lower_case(line);
            }

            if (is_palindrom(line) == 1)
            {
                fprintf(output, "%s is a palindrom\n", og_str);
            } else
            {
                fprintf(output, "%s is not a palindrom\n", og_str);
            }
            free(og_str);
        }
    }
}

/**
 * @brief parses the arguments iterates over every given input file 
 */
int main (int argc, char *argv[]) {
    myprog = argv[0];

    extern char *optarg;
    extern int optind;

    FILE* output = stdout;

    int opt_s = 0;
    int opt_i = 0;
    int opt_o = 0;
    char *o_arg = NULL;

    int c;

    while ((c = getopt(argc, argv, "sio:")) != -1 )
    {
        switch (c)
        {
            case 's':
                    opt_s++;
                    break;
            case 'i':
                    opt_i++;
                    break;
            case 'o':
                    opt_o++;
                    o_arg = optarg;
                    break;
            case '?':
            default:
                    usage();
        }
    }

    // remaining arguments
    argc -= optind;
    // points to first positional argument
    argv += optind;

    if (opt_s >= 2)
    {
        print_error("option 's' occurs more than once");
    }

    if (opt_i >= 2)
    {
        print_error("option 'i' occurs more than once");
    }

    if (opt_o >= 2)
    {
        print_error("option 'o' occurs more than once");
    }

    if (opt_o == 1 && o_arg == NULL)
    {
        print_error("option 'o' did not occur");
    }

    // handle output file. if no file is specified use stdout
    if (opt_o == 1)
    {
        if ((output = fopen(o_arg, "w")) == NULL )
        {
            print_error("error opening output file");
        }
    } else {
        output = stdout;
    }

    // handle input file(s). if no file is specified use stdin

    FILE* input_files[argc == 0 ? 1 : argc];

    if (argc == 0) {
        write_file(stdin, output, opt_s, opt_i);
    } else {
        int i;
        for (i = 0; i < argc; i++)
        {
            if ((input_files[i] = fopen(argv[i], "r")) == NULL)
            {
                print_error("error opening input file(s)");
            }
            write_file(input_files[i], output, opt_s, opt_i);

            if (fclose(input_files[i]) < 0) {
                print_error("closing file failed");
            }
        }
    }   

    if (opt_o == 1) {
        if (fclose(output) < 0) {
            print_error("closing file failed");
        }
    }    

    return EXIT_SUCCESS;
}