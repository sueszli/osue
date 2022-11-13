/**
 * @file myexpand.c
 * @author Michael Meiseneder, 51907333, <michael.meiseneder@student.tuwien.ac.at>
 * @date 23.10.2021
 * @brief myexpand is a variation of the UNIX command expand. myexpand replaces tabstops with spaces. The next character after the tabstop is placed at the next column which is a multiple of the tabstop lenght.
 * @details Synopsis: myexpand [-t tabstop] [-o outfile] [file...]
 * @details If no outfile is specified the output is written to stdout. The default tabstop lenght is 8. If no input files are passed the input is read from stdin. 
 * @details Buffered I/O from stdio.h is used.
 **/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "myexpand.h"

static void error(const char *reason);
static void usage(void);
static void close_files_and_print_error(const char *reason, FILE * f1, FILE * f2);
static void expand_file(FILE * infile, FILE * outfile, int tabstop_lenght);

const char *program_name; /**< Pointer to argv[0] which stores the name of the called executable as a char String.*/
/**
 * @brief Program entry point.
 * @brief Start of the program. Parses passed arguments and implements the functionality of myexpand.
 * @details The actual file modification is carried out by the function expand_file().
 * @details Sets global variable program_name to argv[0].
 * @param argc Argument counter.
 * @param argv Argument vector.
 * @return EXIT_SUCCESS if no I/O errors occured EXIT_FAILURE otherwise.
 * */
int main (int argc, char **argv)
{
    program_name = argv[0];
    
    char opt;   //Stores option character returned by getopt()
    int option_o_parsed = 0, option_t_parsed = 0;     //Stores a non zero value if option -o or option -t has already been parsed (to prevent unexpected behaviour if -t or -o is contained multiple times in the options).
    long int tabstop_lenght = DEFAULT_TABSTOP_LENGHT;   //Stores the lenght of a tabstop character.
    FILE *outfile = stdout;     //File pointer to the output file. If no output file is specified vie -o option stdout is used as the output file.
    char *outfile_name = NULL; //Stores the name of the outfile if specified via -o option. If no output file is specified via -o option stdout is used as the output file.
    char *strtol_endptr = NULL;     //Is passed to strtol as the endptr argument.
    
    while ((opt = getopt (argc, argv, "t:o:")) != -1)
    {
        switch (opt)
        {
            case 't':
                
                if(option_t_parsed != 0)
                {
                    usage();
                }
                option_t_parsed = 1;
                
                tabstop_lenght = strtol (optarg, &strtol_endptr, 10);
                
                /*
                 * tabstop_lenght <= 0: strtol() returns a value smaller / equal than 0 if a number smaller than 1 was parsed or if an invalid String was parsed.
                 * strtol_endptr != '\0': *strtol_endptr contains the address of the first character in the parsed String which is not a digit. '\0' if all parsed characters were digits.
                 * errno == ERANGE: errno is set to ERANGE if the converted number would overflow / underflow.
                 */
                if (tabstop_lenght <= 0 || *strtol_endptr != '\0' || errno == ERANGE)
                {
                    usage();
                }
                break;
                
            case 'o':
                
                if(option_o_parsed != 0)
                {
                    usage();
                }
                option_o_parsed = 1;
                
                outfile_name = optarg;
                
                break;
                
            default:
                usage();
                break;
        }
    }
    
    //If an output file is specified via -o it gets opened here. If no output file was specified no file will be opened and the default value of outfile (=stdout) will be used.
    if(outfile_name != NULL)
    {
        outfile = fopen (outfile_name, "w");
        
        if (outfile == NULL)
        {
            error("opening output file failed");
            exit(EXIT_FAILURE);
        }    
    }
    
    if (optind == argc) //No positional arguments (input files) passed. Therefore use stdin as the input.
    {
        expand_file (stdin, outfile, tabstop_lenght);
    }
    
    else //If positional arguments were passed
    {
        while (argv[optind] != NULL)
        {
            FILE *infile = fopen (argv[optind], "r");   //File pointer to the input file.
            
            if (infile == NULL)
            {
                close_files_and_print_error ("opening input file failed", outfile, NULL);
            }
            
            expand_file (infile, outfile, tabstop_lenght);
            
            if(fclose(infile) != 0)
            {
                close_files_and_print_error("closing input file failed", outfile, NULL);
            }
            
            ++optind;
        }
    }
    
    if(fclose (outfile) != 0)
    {
        error("closing output file failed");
        exit(EXIT_FAILURE);
    }
    
    exit(EXIT_SUCCESS);
}

/**
 * @brief Error function.
 * @brief Prints error message to stderr.
 * @details Error message contains the program name which is stored in the global variable program_name.
 * @param reason String which describes the error; is printed to stderr.
 * @return void
 */
static void error(const char *reason)
{
    fprintf (stderr, "Error: %s: %s. %s\n", program_name, reason, strerror (errno));
}

/**
 * @brief Usage function which terminates the program.
 * @brief Prints usage message to stderr and terminates program
 * @details Program is terminated with status EXIT_FAILURE
 * @details Usage message contains the program name which is stored in the global variable program_name.
 * @return void
 */
static void usage(void)
{
    fprintf (stderr, "Usage: %s: [-t tabstop] [-o outfile] [file...]\n", program_name);
    exit (EXIT_FAILURE);
}

/**
 * @brief Error function which closes passed files and exits program.
 * @brief Closes passed files f1 and f2, prints error message to stderr and terminates program
 * @details Program is terminated with status EXIT_FAILURE
 * @details fclose() is only called if the passed file is not NULL
 * @details Calls function error() which prints the error message
 * @param reason String which describes the error; is printed to stderr
 * @param f1 File 1
 * @param f2 File 2
 * @return void
 */
static void close_files_and_print_error (const char *reason, FILE *f1, FILE *f2)
{
    error(reason);
    
    if (f1 != NULL)
    {
        if(fclose (f1) != 0)
        {
            error("fclose failed");
        }
    }
    
    if (f2 != NULL)
    {
        if(fclose (f2) != 0)
        {
            error("fclose failed");
        }
    }
    
    exit(EXIT_FAILURE);
}

/**
 * @brief Reads content from the input file and writes it to the output file. Tabs are replaced with space characters according to the specification of myexpand.
 * @details The next character after a tab character is placed at a column which multiply of the tabstop lenght. The first column in a line has position 0.
 * @details If I/O operations fail the function close_files_and_print_error() is called.
 * @param infile Input file.
 * @param outfile Output file.
 * @param tabstop_lenght Lenght of a tabstop.
 * @return void
 * */
static void expand_file (FILE * infile, FILE * outfile, int tabstop_lenght)
{
    int c;      //Holds character which was read from the input file.
    int line_position = 0;      //Position of the read character in the line. A line starts with position 0.
    int next_char_position;     //If a tabstop was read from the file the position of the next character is stored in this variable.
    
    while ((c = fgetc(infile)) != EOF)
    {
        if (c == TABSTOP_CHARACTER)
        {
            next_char_position = tabstop_lenght * ((line_position / tabstop_lenght) + 1);   //Calculte position of the next character.
            
            /*
             *             Inserts space characters until line_position equals next_char_position.
             *             A minimum of one space character is always inserted.
             */
            do
            {
                if (fprintf (outfile, " ") < 0) //Write a blank symbol to the output file.
                {
                    close_files_and_print_error("writing to output file failed", infile, outfile);
                }
            }
            while ((++line_position) < next_char_position);
        }
        
        else    //If the read character was not a tabstop character.
        {
            if (fprintf (outfile, "%c", c) < 0) //Write the read character to the output file.
            {
                close_files_and_print_error ("writing to output file failed", infile, outfile);
            }
            
            line_position = ((c == '\n') ? 0 : (line_position + 1));
        }
    }
    
    if(ferror(infile) != 0)     //Check if an error occured while calling fgetc(). This cannot be done by only checking the return value of fgetc().
    {
        close_files_and_print_error("reading input file failed", infile, outfile);
    }
    
}
