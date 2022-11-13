#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define BUFFER_SIZE 1024
#define STANDARD_TABSTOP 8

/** 
 * @author Berndt Uhlig, 11937833
 * @brief Module which replaces all tabulator ("\t") with a given number of empty spaces " ". Synopsis for the program is:
   myexpand [-t tabstop] [-o outfile] [file...]
 * @details The module extracts the necessary arguments from the command line and utilizes a custom function "myexpand" to create the desired solution.
 * @date 23.10.2021
 */

static char *pgm_name; /**< The program name. This comment just shows you how to comment (member) variables/constants or defines on the same line */

/**
 * Mandatory usage function.
 * @brief This function writes helpful usage information about the program to stderr.
 * @details global variables: pgm_name
 */
static void usage(void)
{
    (void)fprintf(stderr, "USAGE: %s [-t tabstop] [-o outfile] [file...]\n", pgm_name);
    exit(EXIT_FAILURE);
}

/** 
 * @brief Custom function in order to replace tabulators from an input file with empty spaces
 * @details Utilizing fgets, a BUFFER_SIZED chunk is taken from the FILE designated in "in". The buffer is the traversed, noting both new lines and tabulators. 
 * In the case of a tabulator, a "tabbuffer" is created, which designates the needed amount of empty spaces per the given calculation and then pastes these into the "out" file. 
 * Should at any point "fputc" return EOF, meaning that the output file is at its end, the loop will break off. Should no tabulator be detected, the function simply pastes the contents into "out", char by char.
 * @param in : Input file
 * @param out : Output file
 * @param tabstop: designated "tabstop" distance. 
 * @return Function returns EXIT_SUCCESS Upon success, EXIT_FAILURE Upon failure.
*/
static void myexp(FILE *in, FILE *out, long tabstop)
{
    char buffer[BUFFER_SIZE];
    int tabindex = 0;
    int linelen = 0;

    while (fgets(buffer, sizeof(buffer), in) != NULL) //buffer sized chunks are taken out of the input file
    {
        for (int i = 0; i < strlen(buffer); i++)
        {
            if (buffer[i] == '\n') //should a new line be detected linelen, meaning the line length is set to -1
            {
                linelen = -1;
            }
            if (buffer[i] == '\t') //detection of a tabulator
            {
                if (tabstop != 0)
                {
                    tabindex = tabstop * ((linelen / tabstop) + 1); //Calculation according to the task
                }
                int len = tabstop == 0 ? 0 : tabindex - linelen;
                char tabbuffer[len];
                for (int k = 0; k < len; k++)
                {
                    tabbuffer[k] = ' '; //Tabbuffer is filled with the correct amount of spaces
                }
                for (int j = 0; j < len; j++)
                {
                    if (fputc(tabbuffer[j], out) == EOF) //fputc is used to place the tabbufferinto output
                    {
                        break;
                    }
                }
                linelen = tabindex - 1; //Needs to be subtracted by 1 => Its incremented again later
            }
            else
            {
                if (fputc(buffer[i], out) == EOF) //if end of output file is reached
                {
                    break;
                }
            }
            linelen++; //incrementation of line length
            if (buffer[i] == '\n')
            {
                linelen = 0;
            }
        }
    }
    fflush(out);
}

/**
 * @brief program execution/main function
 * @details Arguments and options are extracted using getopt(). Should no tabstop value be provided, it is automatically set to 8.
 * Should no output file be designated, the output is put into stdout. Should no input file be designated, the input is std in.
 * The program will then open the input file(s), and use myexp() to alter the contents and paste them into the output.
 * @param argc: length of the option/argument array
 * @param argv: array of options/arguments
 * @return Function returns EXIT_SUCCESS Upon success, EXIT_FAILURE Upon failure.
 **/
int main(int argc, char *argv[])
{
    pgm_name = argv[0];
    char *t_arg = NULL;
    char *o_arg = NULL;
    int c;
    while ((c = getopt(argc, argv, "t:o:")) != -1)
    {
        switch (c)
        {
        case 't':
            t_arg = optarg;
            break;
        case 'o':
            o_arg = optarg;
            break;
        default:
            usage();
        }
    }
    long tabstop;
    char *errptr;

    if (t_arg)
    {
        tabstop = strtol(t_arg, &errptr, 10);
        if (*errptr)
        {
            fprintf(stderr, "%s : Your tabstop value was invalid, please enter a Number, nothing else. Error Code: %s \n", argv[0], strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        tabstop = STANDARD_TABSTOP;
    }

    if (optind == argc)
    {
        FILE *c_output;
        if (o_arg)
        {
            if ((c_output = fopen(o_arg, "w")) == NULL)
            {
                fprintf(stderr, "%s : Failed to Open File to write to. Error Code: %s \n", argv[0], strerror(errno));
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            c_output = stdout;
        }
        myexp(stdin, c_output, tabstop);
        fclose(c_output);
    }
    else
    {
        FILE *b_output;
        if (o_arg)
        {
            if ((b_output = fopen(o_arg, "w")) == NULL)
            {
                fprintf(stderr, "%s : Failed to Open File to write to. Error Code: %s \n", argv[0], strerror(errno));
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            b_output = stdout;
        }

        for (int i = optind; i < argc; i++)
        {

            FILE *a_input;

            if ((a_input = fopen(argv[i], "r")) == NULL)
            {
                fprintf(stderr, "%s : Failed to Open File to read from.  Error Code: %s \n", argv[0], strerror(errno));
                fclose(b_output);
                exit(EXIT_FAILURE);
            }

            myexp(a_input, b_output, tabstop);

            fclose(a_input);
        }
        fclose(b_output);
    }

    return EXIT_SUCCESS;
}