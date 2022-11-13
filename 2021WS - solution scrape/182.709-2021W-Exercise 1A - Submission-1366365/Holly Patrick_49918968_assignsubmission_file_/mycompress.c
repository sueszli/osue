#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>

typedef int bool;		//my solution to use boolean
#define true 1
#define false 0





//reads every char until eof, and writes the solution down.
int
readAndCompress (char *inputfile, char *outputfile, bool hasOutputFile,
		 char *myprog)
{

  FILE *file = fopen (inputfile, "r");
  if (file == NULL)
    {
      fprintf (stderr, " [%s] ERROR: Couldn't open file \n", myprog);
      return EXIT_FAILURE;
    }

  char c = fgetc (file);	//reads the first character
  if (c == EOF)
    {
      fprintf (stderr, " [%s] ERROR: File is empty \n", myprog);
      return EXIT_FAILURE;
    };


  char last = c;
  int counter = 1;
  FILE *fp = stdout;


  int readwords = 1;
  int writtenwords = 0;
  FILE *out = NULL;


  if (hasOutputFile == true)
    {
      out = fopen (outputfile, "a+");	//a for append, when there are 3 inputs i don't want to overwrite the first 2 results
      fp = out;
    }

  fprintf (fp, "%c", c);	//Prints the first letter into the output
  //reads each char until eof
  c = fgetc (file);
  while (c != EOF)
    {
      readwords = readwords + 1;

      if (c == last)
	{
	  counter = counter + 1;
	}
      else
	{
	  writtenwords = writtenwords + 2;
	  fprintf (fp, "%i%c", counter, c);
	  last = c;
	  counter = 1;
	}
      c = fgetc (file);
    };

  writtenwords = writtenwords + 2;
  fprintf (fp, "%i", counter);

  if (hasOutputFile == true)
    {
      fclose (out);
    }

  fclose (file);

  fprintf (stderr, "Read:    %i characters\n", readwords);
  fprintf (stderr, "Written: %i characters\n", writtenwords);
  fprintf (stderr, "Compression ratio: %.1f %c\n",
	   100 * ((float) writtenwords) / (float) readwords, '%');

  return EXIT_SUCCESS;
}


//@details reads the inputs and starts readAndCompress for each input
int
main (int argc, char *argv[])
{
  char *outputfile = NULL;
  bool hasOutputFile = false;
  char *myprog;			//programm name for fancy error messages

  myprog = argv[0];
  int option;

  while ((option = getopt (argc, argv, "o:")) != -1)
    {				//o: because o has a value afterwards that i need to catch with optarg
      switch (option)
	{
	case 'o':
	  outputfile = optarg;
	  hasOutputFile = true;
	  break;
	default:
	  break;
	}
    }
  if (hasOutputFile == true)
    {
      FILE *out = fopen (outputfile, "w");	//delete the values from the file. w opens a the file for writing
      fclose (out);
    }
  if (argc == 1)
    {
      fprintf (stderr, " [%s] ERROR: No input file given \n", myprog);
    }

  if (hasOutputFile == true && argc == 3)
    {
      fprintf (stderr, " [%s] ERROR: No input file given \n", myprog);
    }

  for (; optind < argc; optind++)
    {
      readAndCompress (argv[optind], outputfile, hasOutputFile, myprog);
    }

  return EXIT_SUCCESS;
}
