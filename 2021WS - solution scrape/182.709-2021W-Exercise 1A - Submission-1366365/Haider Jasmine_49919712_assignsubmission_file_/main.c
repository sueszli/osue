/**
*@file main.c
*@author Jasmine Haider <e11943664@student.tuwien.ac.at>
*@date 14.11.2021
*
*@brief Main program module
*
* This program reads lines from files or stdin and prints lines containing a keyword to an output file or stdout.
* Per default the comparison is case sensitive, but case insensitive can be chosen with -i.
**/

#include "mygrep.h"

/** program name.
 * @brief this name will be used for error messages.
 **/
char *prog_name;

/**
 *  usage function.
 * @brief In case the arguments when calling the program were invalid, this function prints what they should be.
 * @details global variables: prog_name
 **/
static void
usage (void)
{
  fprintf (stderr, "Usage %s [-i] [-o outfile] keyword [file...] file \n",
	   prog_name);
  exit (EXIT_FAILURE);
}


/**
 * Starting point of the program.
 * @brief The main function reads the input parameters, opens input and output files and calls mygrep.
 * @details To work with stdin and stdout if no input or output files are given, the file pointers are initialised
 * with those values. If the output file does not exist, it will be created, if it does exist the content will be cleared
 * before it is written.
 * global variables: prog_name
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS.
 **/
int
main (int argc, char *argv[])
{
  int c;
  char *outfile = NULL;
  char *keyword = NULL;
  FILE *in = stdin, *out = stdout;
  int insensitive = FALSE;

  prog_name = argv[0];

  while ((c = getopt (argc, argv, "io:")) != -1)
    {
      switch (c)
	{
	case 'i':
	  if (insensitive == TRUE)
	    {			//argument i has already been given
	      usage ();
	    }
	  insensitive = TRUE;
	  break;
	case 'o':
	  if (outfile != NULL)
	    {			//outputfile has already been set
	      usage ();
	    }
	  outfile = optarg;
	  break;
	case '?':
	  usage ();
	  break;
	default:
	  fprintf (stderr,
		   "%s: default statement reached in argument handling \n",
		   prog_name);
	  exit (EXIT_FAILURE);
	}
    }
  keyword = argv[optind];
  //exit on error if no keyword to be searched for was given
  if (keyword == NULL)
    {
      usage ();
    }
  //if there is an output file - open, exit on error, otherwise stdout is used
  if (outfile != NULL)
    {
      if ((out = fopen (outfile, "w")) == NULL)
	{
	  fprintf (stderr, "%s: fopen failed for %s - %s \n", prog_name,
		   outfile, strerror (errno));
	  exit (EXIT_FAILURE);
	}
    }

  int i = 1;
  // if there is no other positional argument, there are no files to be searched, read from stdin instead
  if (argv[optind + i] == NULL)
    {
      mygrep (in, out, insensitive, keyword);
    }
  else
    {
      //OPEN Input files and call mygrep!
      while (argv[optind + i] != NULL)
	{
	  //try to open the input files
	  if ((in = fopen (argv[optind + i], "r")) == NULL)
	    {
	      fprintf (stderr, "%s: fopen failed for %s - %s \n", prog_name,
		       argv[optind + i], strerror (errno));
	      if (outfile != NULL)
		{
		  fclose (out);
		}
	      exit (EXIT_FAILURE);
	    }
	  //read lines from the file
	  mygrep (in, out, insensitive, keyword);
	  i += 1;
	}
    }
  if (outfile != NULL)
    {
      fclose (out);
    }
  exit (EXIT_SUCCESS);
}
