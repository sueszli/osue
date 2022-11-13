/**
 * @file ispalindrom.c
 * @author Marcin Boniecki (11904666)
 * @date 08.10.2021
 *
 * @brief A program that checks whether strings are palindroms.
 * @details The program ispalindrom shall read files line by line and for each line check whether it is a palindrom, i.e. whether the text read backwards is identical to itself. Each line shall be printed followed by the text “is a palindrom” if the line is a palindrom or “is not a palindrom” if the line is a not palindrom. Your program must accept lines of any length. If one or multiple input files are specified (given as positional arguments), then ispalindrom shall read each of them in the order they are given. If no input file is specified, the program reads from stdin. If the option -o is given, the output is written to the specified file (outfile). Otherwise, the output is written to stdout. The option -s causes the program to ignore whitespaces when checking whether a line is a palindrom. If the option -i is given, the program shall not differentiate between lower and upper case letters, i.e. the check for a palindrom shall be case insensitive.
 **/
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include "ispalindrom.h"

static unsigned char check_if_palindrom (char s[]);
static char *to_lower_case (char s[]);
static char *cut_spaces (char s[]);
static unsigned char check_if_palindrom (char s[]);
static void check_palindron_output (FILE * input, FILE * output,
				    unsigned char opt);
static void check_palindron (FILE * input, unsigned char opt);

/**
 * @brief Program name as a static string variable
 * 
 */
static char *program_name;

/**
 * @brief the main function
 * 
 * @param argc the argument counter
 * @param argv the arguments
 * @return int exit code (EXIT_SUCCESS if successfull, EXIT_FAILURE if failed)
 */
int
main (int argc, char *argv[])
{
  /*
     Options are added with the following values that can be joined by adding them:
     1: cut spaces
     2: case insensitivity
     4: if stdin should be used
     8: if output is defined (is not necessary)
   */
  program_name = argv[0];
  unsigned char opt = 0;
  FILE *output = NULL;

  int c;
  while ((c = getopt (argc, argv, "sio:")) != -1)
    {
      switch (c)
	{
	case 's':
	  opt += 1;
	  break;
	case 'i':
	  opt += 2;
	  break;
	case 'o':
	  opt += 8;
	  if ((output = fopen (optarg, "w")) == NULL)
	    {
	      fprintf (stderr, "%s: fopen in output failed: %s\n",
		       program_name, strerror (errno));
	      exit (EXIT_FAILURE);
	    }
	  break;
	case '?':		/* invalid option */
	  break;
	default:
	  break;
	}
    }

  if (argc != optind)
    {

      FILE *input;

      for (int i = 0; i < (argc - optind); i++)
	{
	  if ((input = fopen (argv[optind + i], "r")) == NULL)
	    {
	      fprintf (stderr, "%s - fopen in input failed: %s\n",
		       program_name, strerror (errno));
	      exit (EXIT_FAILURE);
	    }
	  else
	    {
	      if (output == NULL)
		{
		  check_palindron (input, opt);
		}
	      else
		{
		  check_palindron_output (input, output, opt);
		}
	      fclose (input);
	    }
	}

    }
  /* if no input files are specified */
  else
    {
      opt += 4;
    }

  if ((opt & 4) == 4)
    {
      if (output == NULL)
	{
	  check_palindron (stdin, opt);
	}
      else
	{
	  check_palindron_output (stdin, output, opt);
	}
    }

  if (output != NULL)
    {
      fclose (output);
    }

  exit (EXIT_SUCCESS);
}

/**
 * @brief checks if a file is a palindrom line by line and writes that into the output file
 * 
 * @param input the input that is beeing checked line by line
 * @param output the output where solution is written into
 * @param opt options the oprions that can be given to check if a line is a palindrom with the following parameters: (1: cut spaces, 2: case insensitivity, 3: cut spaces and case insensitivity)
 */
static void
check_palindron_output (FILE * input, FILE * output, unsigned char opt)
{
  char *buffer = NULL;
  size_t buffet_size = 0;
  ssize_t read_file;

  while ((read_file = getline (&buffer, &buffet_size, input)) != -1)
    {
      unsigned char palindrom_check;
      if ((opt & 3) == 3)
	{
	  palindrom_check =
	    check_if_palindrom (cut_spaces (to_lower_case (buffer)));
	}
      else if ((opt & 2) == 2)
	{
	  palindrom_check = check_if_palindrom (to_lower_case (buffer));
	}
      else if ((opt & 1) == 1)
	{
	  palindrom_check = check_if_palindrom (cut_spaces (buffer));
	}
      else
	{
	  palindrom_check = check_if_palindrom (buffer);
	}

      if (palindrom_check)
	{
	  if (output == NULL)
	    printf ("%.*s is a palindrom\n", (int) (strlen (buffer) - 1),
		    buffer);
	  else
	    fprintf (output, "%.*s is a palindrom\n",
		     (int) (strlen (buffer) - 1), buffer);
	}
      else
	{
	  if (output == NULL)
	    printf ("%.*s is not a palindrom\n", (int) (strlen (buffer) - 1),
		    buffer);
	  else
	    fprintf (output, "%.*s is not a palindrom\n",
		     (int) (strlen (buffer) - 1), buffer);
	}
    }
  if (ferror (input))
    {
      free (buffer);
      fprintf (stderr, "%s: getline failed: %s\n", program_name,
	       strerror (errno));
      exit (EXIT_FAILURE);
    }
  free (buffer);
}

/**
 * @brief checks if a file is a palindrom line by line and writes that as an output in the console
 * 
 * @param input the input that is beeing checked line by line
 * @param output the output where solution is written into
 * @param opt options the oprions that can be given to check if a line is a palindrom with the following parameters: (1: cut spaces, 2: case insensitivity, 3: cut spaces and case insensitivity)
 */
static void
check_palindron (FILE * input, unsigned char opt)
{
  check_palindron_output (input, NULL, opt);
}

/**
 * 
 * @brief Checks if the given string is a palindrom. The last given character must be a \n, otherwise the program doesn't work properly
 * 
 * @param s the string that should be checked if it is a palindrom or not with \n as the last character
 * @return unsigned char return 1 if the string is a palindrom and 0 if not
*/
static unsigned char
check_if_palindrom (char s[])
{
  for (int i = 0; i < strlen (s) / 2; i++)
    {
      if (s[i] != s[strlen (s) - 2 - i])
	{
	  return 0;
	}
    }
  return 1;
}

/**
 * @brief Turns a string into a lower case string
 * 
 * @param s character will be turned into a lower case string
 * @return string s in lower case letters
*/
static char *
to_lower_case (char s[])
{
  char *s_return = malloc (sizeof (char) * strlen (s));

  for (int i = 0; i < strlen (s); i++)
    {
      s_return[i] = tolower (s[i]);
    }

  return s_return;
}

/**
 * @brief deletes white spaces from a string
 * 
 * @param s string from which strings will be removed
 * @return string s with removed spaces
*/
static char *
cut_spaces (char s[])
{
  char *s_return = malloc (sizeof (char) * strlen (s));
  int s_i = 0;

  for (int i = 0; i < strlen (s); i++)
    {
      if (s[i] != ' ')
	{
	  s_return[s_i] = s[i];
	  s_i++;
	}
    }

  return s_return;
}
