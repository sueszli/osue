/**
 * @file main.c
 * @author Andreas Hoessl <e11910612@student.tuwien.ac.at>
 * @date 02.11.2021
 *
 * @brief Main program module.
 *
 * This is the main program module for the 'myexpand' command. The command 'myexpand' is a variation of the Unix-command 'expand'.
 * It replaces tab characters (\t) with a number of spaces. The default input is taken from stdin, however one or more input files
 * may be provided through positional arguments. The default tabstop distance is 8. It can be overridden with an arbitrary positive
 * integer using the option '-t'. The output is written to stdout unless option '-o' is given, in which case it is written to the
 * specified file.  
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

static char *myprog; /** < The Program name. */

static char *arg_o = NULL; /** < Argument of option '-o'. */
static char *arg_t = NULL; /** < Argument of option '-t'. */
static int tabstop = 8; /** < Default tabstop distance. */
static int position = 0; /** < Starting position of the first character in each line. */

static char **input = NULL; /** < Names of the input files. */
static char *output = NULL; /** < Name of the output file. */

static FILE *infile; /** < Input file. */
static FILE *outfile; /** < Output file. */

/**
 * Usage function.
 * @brief This function writes helpful usage information about the program to stderr.
 * @details global variables: myprog
 */
static void usage(void)
{
	fprintf(stderr, "usage: %s [-t tabstop] [-o outfile] [file...]\n" 
	"\t -t tabstop: tabstop distance (> 0, default is 8)\n"
	"\t -o outfile: the output-file (default output is stdout)\n"
	"\t file...: one or more input-files (default input is stdin)\n", myprog);
    exit(EXIT_FAILURE);
}

/**
 * Procedure.
 * @brief This procedure changes tab characters to a specified number of space characters.
 * @details The in- and output files are provided via the global variables. Tab characters in 'infile' are changed into a corresponding number of
 * space characters in 'outfile'. The position of the tab character and the tabstop distance determine the new position in the output file.
 * global variables: tabstop, position, infile, outfile
 */
static void expansion(void)
{
	char c;
	while ((c = fgetc(infile)) != EOF) { /** < Terminates when the end of the file is reached. */
		if ( c == '\t') {
			int p = tabstop * ((position / tabstop) + 1); /** < Calculating the position in the output file. */
			while (position < p) {
				fprintf(outfile, " ");
				position++;
			}
		} else {
			fprintf(outfile, "%c", c);
			if (c == '\n') {
				position = 0; /** < The position is reset to 0 when a new line begins. */
			} else {
				position++;
			}
		}
	}
}

/**
 * Procedure.
 * @brief This procedure provides the input files for 'expansion'.
 * @details The names of the input files are stored in the global variable 'input'. This procedure opens one file at a time and stores it in the
 * global variable 'infile'. 'expansion' will then process the file and ultimately the file will be closed. Each file will be handled until there are no
 * more left.
 * global variables: input, infile
 */
static void exp_files(void)
{
	while (*input != NULL) { /** < Terminates when no input files are left. */
		infile = fopen(*input++, "r"); /** < Opens one of the input files and sets the pointer to the next. */
		if (infile == NULL) { /** < Error handling. */
			fprintf(stderr, "fopen failed: %s \n", strerror(errno));
			exit(EXIT_FAILURE);
		} else {
			expansion(); /** < Proceeds to expanding the input file. */
		}
		fclose(infile);
	}
}

/**
 * Program entry point.
 * @brief The main function that handles options and arguments.
 * @details The options and arguments of this command are handled in this function. It makes sure that the desired tabstop distance, input and
 * output are being used. If the output is in a file, it will be opened and closed here.
 * global variables : myprog, arg_o, arg_t, tabstop, input, output, outfile
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS
 */
int main(int argc, char **argv)
{
	myprog = argv[0];
	int c;
	
	while ((c = getopt(argc, argv, "t:o:")) != -1) {
		switch (c) {
			case 't':
				arg_t = optarg;
				break;
			case 'o':
				arg_o = optarg;
				break;
			case '?':
				usage();
				break;
			default:
				assert(0);
		}
	}
	
	if (arg_t != NULL) {
		tabstop = (int) strtol(arg_t, NULL, 10); /** < arg_t is cast to an integer. */
		if (tabstop <= 0) {
			usage();
		}
	}
	
	if (arg_o != NULL) { /** < An output file is provided and will be opened. */
		output = arg_o;
		outfile = fopen(output, "w");
	} else { /** < Option '-o' is not in use and the output is on stdout. */
		outfile = stdout;
	}
	
	if ((argc - optind) > 0) { /** < The positional arguments correspond to the name of the input files. */
		input = &argv[optind];
		exp_files();
	} else { /** < No positional arguments. The input is taken from stdin. */
		infile = stdin;
		expansion();
	}
	
	if (output != NULL) { /** < If ther is an output file, it will be closed. */
		fclose(outfile);
	}
	
	return EXIT_SUCCESS;
}
