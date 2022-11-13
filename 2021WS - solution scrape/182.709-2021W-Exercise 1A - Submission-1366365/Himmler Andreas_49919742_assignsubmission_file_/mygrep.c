/**
 * @file mygrep.c
 * @author Andreas Himmler 11901924
 * @date 9.11.2021
 *
 * @brief A reduced variation of the Unix-command grep, which reads in several
 *        files and prints all lines containing a keyword.
 */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

static char *myprog; /**< The name of the program. */

/**
 * @brief Swaps every upper case letter in the given string to the respective
 *        lower case letter and returns it.
 * @param string Mixed case string.
 * @return lower case string.
 */
static char *lower_string (char *string)
{
    char *result = malloc (strlen (string));
    strcpy (result, string);

    for (int i = 0; i < strlen (result); ++i)
    {
        result[i] = tolower (result[i]);
    }

    return &result[0];
}

/**
 * @brief Scans every line of the input-file and checks if the keyword is
 *        contained. If so the line is outputed to the output-file. If
 *        ignore_case is set, the search is case insensitivw, otherwise it is
 *        case sensitive.
 * @param input input file
 * @param output output file
 * @param keyword keyword to be searched
 * @param ignore_case flag if search is case sensitive
 */
static void print_lines_with_keyword (FILE *input, FILE *output,
                                      char *keyword, int ignore_case)
{
    char *line = NULL;
    size_t len = 0;
    ssize_t read = 0;

    while ((read = getline (&line, &len, input)) != -1)
    {
        if (ignore_case > 0)
        {
            if (strstr (lower_string (line), keyword) != NULL)
            {
                fputs (line, output);
            }
        }
        else if (strstr (line, keyword) != NULL)
        {
            fputs (line, output);
        }
    }
    free (line);
}

/**
 * @brief Outputs the usage of the program.
 */
static void usage (void)
{
    fprintf (stderr, "Usage: %s [-i] [-o outfile] keyword [file...]\n",
             myprog);
    exit (EXIT_FAILURE);
}

/**
 * @brief The main function. Processes all arguments. Opens files, if possible
 *        and calls print_lines_with_keyword in combination with the keyword on
 *        them.
 * @param argc Number of arguments
 * @param argv arguments
 * @return EXIT_SUCCESS if no error occurs
 * @return EXIT_FAILURE in case of an error
 */
int main (int argc, char *argv[])
{
    myprog = argv[0];

    int c;
    int ignore_case = 0;
    char *outfile = NULL;

    while ((c = getopt (argc, argv, "io:")) != -1)
    {
        switch (c)
        {
            case 'i':
                ignore_case++;
                break;
            case 'o':
                outfile = optarg;
                break;
            case '?':       // invalid option
                usage ();
                break;
            default:
                assert (0);
        }
    }

    if ((argc - optind) < 1)
        usage ();


    char *keyword = argv[optind];
    FILE *output = NULL;

    if (outfile)
    {
        if ((output = fopen (outfile, "wb")) == NULL)
        {
            fprintf (stderr, "[%s] Error: File %s could not be opened\n",
                     myprog, outfile);
            exit (EXIT_FAILURE);
        }
    }
    else
        output = stdout;

    if (ignore_case > 0)
        keyword = lower_string (keyword);

    if (argc - optind == 1)
        print_lines_with_keyword (stdin, output, keyword, ignore_case);
    else
    {
        FILE *file;

        for (int i = optind + 1; i < argc; ++i)
        {
            if ((file = fopen (argv[i], "r")) == NULL)
            {
                // error: file cannot be opened.
                fprintf (stderr,
                         "[%s] Error: File '%s' could not be opened!\n",
                         myprog, argv[i]);
                fclose (output);
                exit (EXIT_FAILURE);
            }
            print_lines_with_keyword (file, output, keyword, ignore_case);
            fclose (file);
        }
    }
    fclose (output);
    exit (EXIT_SUCCESS);
}

