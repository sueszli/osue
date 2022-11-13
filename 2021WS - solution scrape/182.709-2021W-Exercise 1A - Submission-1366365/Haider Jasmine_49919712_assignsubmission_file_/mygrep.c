/**
*@file mygrep.c
*@author Jasmine Haider <e11943664@student.tuwien.ac.at>
*@date 14.11.2021
*
*@brief implements a reduced version of grep for a file.
*
* The mygrep module. Checks if a keyword is contained in lines of a file or stdin.
**/

#include "mygrep.h"

/**
 * function to convert string to lower case.
 * @brief This function will replace all characters in the input string with their lower case version.
 **/
static void
lowercase (char *inputstring)
{
  //convert inputstring to lower case
  int length = strlen (inputstring);
  for (int i = 0; i < length; i++)
    {
      inputstring[i] = tolower (inputstring[i]);
    }
}

/**
 * @details the line that is read by getline needs to be copied for printing, as lowercase will replace the string
 * with a lower case version, and otherwise the printed line will not have the same case as the line of the input file.
 **/
void
mygrep (FILE * in, FILE * out, int insensitive, char *keyword)
{

  char *origline = NULL;
  char *line = NULL;
  size_t len = 0;
  ssize_t buffer;

  //read lines from the file
  while ((buffer = getline (&origline, &len, in)) != -1)
    {
      line = malloc (strlen (origline) + 1);
      if (line == NULL)
	{
	  fprintf (stderr, "%s: malloc failed: %s \n", prog_name,
		   strerror (errno));
	  fclose (in);
	  free (origline);
	  exit (EXIT_FAILURE);
	}
      else
	{

	  strcpy (line, origline);
	  if (insensitive == TRUE)
	    {
	      lowercase (line);
	      lowercase (keyword);
	    }

	  //strstr returns NULL if keyword is not found in buffer
	  if (((insensitive == TRUE) && (strstr (line, keyword) != NULL))
	      || (strstr (origline, keyword) != NULL))
	    {
	      if (fputs (origline, out) == EOF)
		{
		  fprintf (stderr, "%s: fputs failed: %s \n", prog_name,
			   strerror (errno));
		  fclose (in);
		  free (line);
		  free (origline);
		  exit (EXIT_FAILURE);
		}
	    }
	}
      free (line);
    }
  free (origline);
  fclose (in);
}
