/**
 * @file mygrep.c
 * @author Andreas Tober 11809604
 * @date 8.11.2021
 *
 * @brief mygrep reads in several files and prints all lines containing a keyword
 *
 * @details mygrep will go through all input files one after the other, line by line. If no input file is specified
 * it will read from stdin. If a line contains the given keyword it will be printed to stdout or an output
 * file if the option -o followed by the filename is passed. If the option -i is given the search will be case-insensitive.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#define PROGRAMM_NAME  "mygrep"

/**
 * @brief prints an error message to stderr.
 * @details a given error message will be printed containing the name of the programm.
 * @param message:  error message that will be printed.
 */
static void print_error (char *message);

/**
 * @brief prints an error message to stderr and exits.
 * @details a given error message will be printed containing the name of the programm,
 * afterwards the programm will exit with a failiure Status.
 * @param message:  error message that will be printed.
 */
static void print_error_and_exit (char *message);

/**
 * @brief prints an usage message to stderr.
 * @details will print information about the options and arguments of the programm.
 * afterwards the programm will exit with a failiure Status.
 */
static void print_usage (void);

/**
 * @brief converts a string to lowercase.
 * @details will go through each character of a given string and replaces 
 * every occurence of an uppercase character with the according lowercase character.
 * @param str: string to convert
 */

static void string_to_lower (char *str);

/**
 * @brief prints line if it contains keyword
 * @details checks if the line contains the keyword, this check can be case-insensitive.
 * If the check passes the line will be printed to the geiven output file.
 * @param line the string to search for the keyword.
 * @param output the output file to write too
 * @param keyword the word to search.
 * @param is_case_insensitive decides if the search is case insensitive
*/
static void handle_line (char *line, FILE * output, char *keyword,
			 int is_case_insensitive);

/**
 * @brief prints each line of the input file if it contains the keyword.
 * @details searches for the keyword line by line in the input file. This seach
 * can be case-insensitive. If a line contains the keyword it will be printed to the 
 * output file.
 * @param input file to search for the keyword.
 * @param output file to write too.
 * @param keyword the word to search.
 * @param is_case_insensitive decides if the search is case-insensitive
 */
static void handle_file (FILE * input, FILE * output, char *keyword,
			 int is_case_insensitive);

/**
 * @brief handles options and arguments of the programm.
 * @details will set the output file and the case-insensitivity according to the arguments and options of the programm.
 * The options are -i for case insensitivity and -o output_file to set the output file. Multiple or wrong 
 * arguments will lead to errors.
 * @param argc the argument counter o f the programm.
 * @param argv the arguments of the programm.
 * @param output this file should be set if option is given.
 * @param is_case_insensitive should be set to positive value if option is given.
 * @return returns the option index.
 */
static int handle_arguments (int argc, char **argv, FILE ** output,
			     int *is_case_insensitive);


/**
 * @brief entry point of the program. manages input files and delegates to other functions.
 * @details is responsible for managing variables and passing them to the right functions, also opens input files
 * and opens and closes output file. Problems with the files or wrong number of arguments will lead to errors and early termination.
 * calls functions to proceed them. Uses global variable final_stats to update the statistics.
 * @param argc counter of arguments
 * @param argv holding the program's arguments and options.
 */
int
main (int argc, char **argv)
{

  if (argc <= 1)
    {
      print_usage ();
    }

  FILE *output = stdout;
  FILE *input = stdin;
  int is_case_insensitive = 0;

  int i = handle_arguments (argc, argv, &output, &is_case_insensitive);

  char *keyword = argv[i++];

  if (i == argc)
    {
      handle_file (input, output, keyword, is_case_insensitive);
    }

  for (; i < argc; i++)
    {
      input = fopen (argv[i], "r");
      if (input == NULL)
	{
	  print_error (strcat ("could not open input file: ", argv[i]));
	  continue;
	}
      handle_file (input, output, keyword, is_case_insensitive);

      if (fclose (input) != 0)
	{
	  print_error (strcat ("could not close input file: ", argv[i]));
	}
    }

  if (output != stdout)
    {
      if (fclose (output) != 0)
	{
	  print_error ("could not close output file");
	}
    }

  exit (EXIT_SUCCESS);

}



static void
print_error (char *message)
{
  fprintf (stderr, "%s error: %s", PROGRAMM_NAME, message);
}

static void
print_error_and_exit (char *message)
{
  print_error (message);
  exit (EXIT_FAILURE);
}

static void
print_usage (void)
{
  fprintf (stderr, "Usage: %s [-i] [-o outfile] keyword [file...]\n",
	   PROGRAMM_NAME);
  exit (EXIT_FAILURE);
}

static void
string_to_lower (char *str)
{
  for (int i = 0; str[i]; i++)
    {
      str[i] = tolower (str[i]);
    }
}

static void
handle_line (char *line, FILE * output, char *keyword,
	     int is_case_insensitive)
{
  char *work_line = (char *) malloc (strlen (line));
  strcpy (work_line, line);

  char *work_keyword = (char *) malloc (strlen (keyword));
  strcpy (work_keyword, keyword);

  if (is_case_insensitive != 0)
    {
      string_to_lower (work_line);
      string_to_lower (work_keyword);
    }

  if (strstr (work_line, work_keyword) != NULL)
    {
      fprintf (output, "%s\n", line);
    }

  free (work_line);
  free (work_keyword);
}


static void
handle_file (FILE * input, FILE * output, char *keyword,
	     int is_case_insensitive)
{
  char *line = NULL;
  size_t size = 0;

  while (getline (&line, &size, input) != -1)
    {
      if (line[strlen (line) - 1] == '\n')
	{
	  line[strlen (line) - 1] = '\0';
	}

      if (strlen (line) == 0)
	{
	  continue;
	}

      handle_line (line, output, keyword, is_case_insensitive);
    }

  free (line);

  if (feof (input) == 0)
    {
      print_error ("could not read till end of input file");
    }
}

static int
handle_arguments (int argc, char **argv, FILE ** output,
		  int *is_case_insensitive)
{
  int c;

  while ((c = getopt (argc, argv, "io:")) != -1)
    {
      switch (c)
	{
	case 'i':
	  if (*is_case_insensitive == 0)
	    {
	      *is_case_insensitive = 1;
	    }
	  else
	    {
	      print_usage ();
	    }
	  break;
	case 'o':
	  if (*output == stdout)
	    {
	      *output = fopen (optarg, "w");
	      if (output == NULL)
		{
		  print_error_and_exit ("could not open output file");
		}
	    }
	  else
	    {
	      print_usage ();
	    }
	  break;
	default:
	  print_usage ();
	}
    }

  return optind;
}
