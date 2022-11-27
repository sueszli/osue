/**
 * @file ispalindrom.c
 * @author Lorenz Hoerburger <e12024737@student.tuwien.ac.at>
 * @date 04.11.2021 
 *
 * @brief Main program module.
 * This program validates if a input is a palindrom. 
 * Inputs can be stdin or 0..* FILES. 
 * White spaces and case can be ignored. Writes output to stdout or a file ([-o FILE])   
 * USAGE: %s [-s] [-i] [-o outfile] [file...]
 **/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <getopt.h>

static char *pgm_name;

struct options
{
    int opt_i;
    int opt_s;
    FILE *output;
};

//Prototypes
static int is_palindrom(char *str, struct options *opt);
static void usage(void);
static char *str_toupper(char *str);
static void hande_input_options(int argc, char **argv, struct options *opt);
static void write_input(char *input_line, struct options *opt);
static void handle_file(char *file_name, struct options *opt);
static void handle_file_v(FILE *file, struct options *opt);
static void remove_space(char *str);

/**
 * Program entry point.
 * @brief The program starts here. This function takes care about parameters, and if the
 * overall program is reasonable trivial it could implement the whole program functionality -
 * like here. 
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS.
 */
int main(int argc, char **argv)
{
    pgm_name = argv[0];

    struct options opt = {0, 0, stdout};
    hande_input_options(argc, argv, &opt);

    // input
    if (argc - optind > 0)
    {
        int i;
        for (i = optind; i < argc; i++)
        {
            handle_file(argv[i], &opt);
        }
    }
    else
    {
        handle_file_v(stdin, &opt);
    }

    fclose(opt.output);
    exit(EXIT_SUCCESS);
    return 0;
}

/**
 * Hanles user input and sets options 
 * @brief This function handles the user input and checks the options s, i ,o
 * i ... ignore case 
 * s ... ignore whitespaces
 * o ... outputfile
 * 
 * @param argc argument count from main
 * @param argv argument vector from main 
 * @param opt  options
 */
static void hande_input_options(int argc, char **argv, struct options *opt)
{
    int c;

    while ((c = getopt(argc, argv, "sio:")) != -1)
    {
        switch (c)
        {
        case ('s'):
            opt->opt_s = 1;
            break;
        case ('i'):
            opt->opt_i = 1;
            break;
        case ('o'):
            if (optarg == NULL)
                usage();
            if ((opt->output = fopen(optarg, "w")) == NULL)
            {
                fprintf(stderr, "fopen failed: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
            break;
        default:
            usage();
        }
    }
}

/**
 * 
 * @brief Reads a file line by line and and validates every line if it's a palindrom. 
 * Writes validated string to output opion
 * 
 * @param file_name file name of file to read
 * @param opt options
 */
static void handle_file(char *file_name, struct options *opt)
{
    FILE *file;

    if ((file = fopen(file_name, "r")) == NULL)
    {
        fprintf(stderr, "fopen failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    handle_file_v(file, opt);

    fclose(file);
}

/**
 * 
 * @brief Reads a file line by line and and validates every line if it's a palindrom. 
 * Writes validated string to output opion
 * 
 * @param file_name file name of file to read
 * @param opt options
 */
static void handle_file_v(FILE *file, struct options *opt)
{
    size_t size = 0;
    char *line = NULL;

    while (getline(&line, &size, file) > 0)
    {
        if (line[strlen(line) == '\n'])
            line[strlen(line) - 1] = '\0';
        write_input(line, opt);
    }
}

/**
 * Writes input_line to output option
 * @brief This function takes the input_line, validates if it's a palindrom and writes it to output option.
 * 
 * @param input_line string to be validated as palindrom and to be written to output
 * @param opt options
 */
static void write_input(char *input_line, struct options *opt)
{
    if (is_palindrom(input_line, opt) != 0)
        fprintf(opt->output, "%s is a palindrom\n", input_line);
    else
        fprintf(opt->output, "%s is not a palindrom\n", input_line);
}

/**
 * Validates if a string is a palindrom
 * @brief This function checks if the string given in the parameter is a palindrom.
 * @param str to be validated as palindrom
 * @param opt options
 * @return returns true if the string is a palindrome else returns false
 */
static int is_palindrom(char *str, struct options *opt)
{
    char *d = malloc(strlen(str));
    strcpy(d, str);

    // ignore case
    if (opt->opt_i != 0)
        str_toupper(d);

    // ignore space
    if (opt->opt_s != 0)
        remove_space(d);

    int start = 0;
    int end = strlen(d) - 1;

    while (start <= end)
    {
        if (d[start] != d[end])
            return 0;
        start++;
        end--;
    }

    free(d);

    return 1;
}

/**
 * @brief removes all space from input string
 * 
 * @param str input string
 */
static void remove_space(char *str)
{
    for (char *reader = str; *reader != '\0'; ++reader)
    {
        if (*reader != ' ')
        {
            *str = *reader;
            ++str;
        }
    }
    *str = '\0';
}

/**
* transforms all chars in string to uppercase
* @brief Takes a string and returns the same string transfomed in uppercase
* @param *str: input string to be transformed to uppercase
* @return param: *str in uppercase
*/
static char *str_toupper(char *str)
{
    for (; *str != '\0'; str++)
    {
        *str = toupper(*str);
    }
    return "d";
}

/**
 * @brief This function writes helpful usage information about the program to stderr.
 * @details global variables: pgm_name
 */
static void usage(void)
{
    (void)fprintf(stderr, "USAGE: %s [-s] [-i] [-o outfile] [file...]\n", pgm_name);

    exit(EXIT_FAILURE);
}
