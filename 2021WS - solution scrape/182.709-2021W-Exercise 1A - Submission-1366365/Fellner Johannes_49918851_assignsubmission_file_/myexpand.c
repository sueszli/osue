/**
 * author: 	Johannes Fellner
 * date: 	20180412
 * mat.nr.: 	01325043
 * program: 	myexpand
 */

/***********************************
*	     Includes              *
***********************************/

#include <stdio.h>
#include <stdlib.h>		/* EXIT_SUCCESS, EXIT_FAILURE */
#include <unistd.h>
#include <getopt.h>		/* getopt */
#include <assert.h>		/* assert() */
#include <string.h>		/* strcmp() */
#include <limits.h>		/* INT_MAX */

/***********************************
*	   Constants               *
***********************************/

#define MAX_BUFFER_SIZE (255)

static char *progname;
char *bufferTabs = NULL;
static FILE* infile;
static FILE* outfile;

/***********************************
*	   Prototypes              *
***********************************/

/**
 * @brief prints message of correct synopsis
 */
static void usage(void);

/*
 * @brief replaces tabulator with blanks and creates new char[] for output
 * @param buffer inputstream to be edited
 * @param bufferTabs edited output with blanks
 * @param tab number of blanks to replace the tabulator
 */
static void insertTab(char buffer[], char *bufferTabs, int tab);


/***********************************
*	   Implementation          *
***********************************/

static void usage(void)
{
	(void)fprintf(stderr, "Synopsis: %s [-t tabstop] [-o outfile] [file ...]\n", progname);
	exit(EXIT_FAILURE);
}

static void insertTab(char buffer[], char *bufferTabs, int tab)
{
	int position = 0;	// position of the tabulator in char[]
	int k = 0;
	int bufferSize = 0;	// size of the buffer to be reallocated

	for(int i = 0; i < MAX_BUFFER_SIZE; i++)
	{
		if(buffer[i] == '\t') // char is a tab -> new position needs to be calculated
		{
			if(tab > 0)	// tab must be greater zero to avoid DIVIDED_BY_ZERO_ERROR
			{		// if tab is set to zero do nothing to cut the tabs out of the buffer
				bufferSize += tab * sizeof(char);
				bufferTabs = (char *) realloc(bufferTabs, bufferSize);
				position = tab * ((i / tab) + 1);  // calculate position of the next tabulator

				for(int j = i; j < position; j++)  // insert blanks
				{
					bufferTabs[k] = ' ';
					k++;
				}
			}
		}
		else if(buffer[i] == '\n')	// buffer contains end of line -> stop processing buffer
		{
		    bufferSize += sizeof(char);
		    bufferTabs = (char *) realloc(bufferTabs, bufferSize);
		    bufferTabs[k] = buffer[i];
		    break;
		}
		else	// normal char -> copy to the resulting buffer
		{
			bufferSize += sizeof(char);
			bufferTabs = (char *) realloc(bufferTabs, bufferSize + sizeof(char));
			bufferTabs[k] = buffer[i];
			k++;
		}
	}
}

int main (int argc, char **argv)
{
	int tflag = 0;		// 0 for not set and 1 for set
	int oflag = 0;		// 0 for not set and 1 for set
	int option;
	int tab = 8;		// position of the tabulator (e.g.: 8, 16, 24, ...) default value 8
	char *endptr = "";	// pointer needed for function strtol
	char buffer[MAX_BUFFER_SIZE] = "";	// char[] to safe the input stream (file or stdin)
		// char[] to safe manipulated input stream to get to output (stdout)
	char *outfilename = "";

	progname = argv[0];	// argv[0] always contains the programname

	while ((option = getopt(argc, argv, "t:o:")) != -1)	// get opt returns all options comitted on execution of the program in command line
	{
		switch(option)
		{
			case 't':	/* option with argument */
				if(tflag == 0)	// option -t only one time allowed -> otherwise give out usage
				{
					tflag = 1;
					tab = strtol(optarg, &endptr, 10);	// convert optparam to long
										// if endptr is "" -> optparam is long
					if(tab > INT_MAX)
					{
						(void)fprintf(stderr, "Error value of tabstop exceeding INT_MAX\n");
						(void)usage();
					}
					if((tab < 0) || (strcmp("", endptr) != 0))
					{
						(void)fprintf(stderr, "Error value of tabstop must be Integer \n");
						(void)usage();
					}
				}
				else
				{
					(void)fprintf(stderr, "Error too many arguments of [-t tabstop]\n");
					(void)usage();
				}
				break;
			case 'o':
				fprintf(stderr, "\nTest\n");
				if(oflag == 0)
				{
					oflag = 1;
					outfilename = optarg;
				}
				else
				{
					(void)fprintf(stderr, "Error too many arguments of [-o outfile]\n");
					(void)usage();
				}
				break;
			case '?':	/* invalid option */
				(void)fprintf(stderr, "Error unknown option\n");
				(void)usage();
				break;
			default:	/* not reachable */
				(void)assert(0);
		}
	}

	// ./myexpand
	// ./myexpand -t tab
	// ./myexpand -o outfile
	// ./myexpand -t tab -o outfile
	// --> read input from stdin
	if(argc == 1 || (argc == 3 && tflag == 1) || (argc == 3 && oflag == 1) || (argc == 5 && tflag == 1 && oflag == 1))
	{
		while(fgets(buffer, MAX_BUFFER_SIZE, stdin))
		{
		  bufferTabs = (char *) malloc(0);	// initialize memory allocation
		  insertTab(buffer, bufferTabs, tab);
		  if(oflag == 0)
			  (void)fprintf(stdout, "%s", bufferTabs);
		  else
		  {
			  outfile = fopen(outfilename, "a");
			  if(outfile == NULL)
			  {
				  (void)fprintf(stderr, "Error opening output file\n");
				  exit(EXIT_FAILURE);
			  }
			  else
				  (void)fputs(bufferTabs, outfile);
			  (void)fclose(outfile);
		  }
		  free(bufferTabs);
		}
	}
	else	// read input from file(s)
	{
		for(int i = 1; i < argc; i++)
		{
			if(strcmp(argv[i], "-t") != 0 && strcmp(argv[i], "-o") != 0)
			{
				if((infile = fopen(argv[i], "r")) == NULL)	//check if file exists
				{
					(void)fprintf(stderr, "Error failed to open/find file: %s\n", argv[i]);
					exit(EXIT_FAILURE);
				}

				while(fgets(buffer, MAX_BUFFER_SIZE, infile))
				{
					bufferTabs = (char *) malloc(0);	// initialize memory allocation
					insertTab(buffer, bufferTabs, tab);	// otherwise print buffer
					if(oflag == 0)
						(void)fprintf(stdout, "%s", bufferTabs);
					else
					{
						outfile = fopen(outfilename, "a");
						if(outfile == NULL)
						{
						    (void)fprintf(stderr, "Error opening output file\n");
						    exit(EXIT_FAILURE);
						}
						else
							(void)fputs(bufferTabs, outfile);
						if(fclose(outfile) == EOF)
						{
						    (void)fprintf(stderr, "Error closing output file\n");
						    exit(EXIT_FAILURE);
						};
					}
					free(bufferTabs);
				}
				(void)fclose(infile);		// closes file
			}
			else
			{
				i++;
			}
		}
	}

	exit(EXIT_SUCCESS);
}
