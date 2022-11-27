/**
 * @file ispalindrom.c
 * @author Kristiana Muco 01528577
 * @date 14.11.2021
 * @brief <b>ispalindrom</b> Main program module
 * @details
 * This program reads text from stdin or from files, line by line,
 * and checks if the text of a line is a palindrom.
 * Each line is written to output.
 * When the text is a palindrom, " is a palindrom" is appended,
 * if not, " is not a palindrom" is appended.
 * 
 * Synopsis: ispalindrom [-s] [-i] [-o outfile] [file...]
 * 
 **/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <assert.h>

/** @enum bool 
 **/
typedef enum
{
	false = 0,  
	true = 1    
} bool;

#define STR_SIZE_STEPS 1024

/**
 * @brief Prints the information about the proper usage and ends the program.
 * @details The information about the usage is printed to stdout
 * and the program is then terminated with EXIT_FAILURE exit code.
 * @param prog is set by the main function and equals argv[0]
 * @returns EXIT_FAILURE , when the program is not used as it should be
 */
static void usage(char* prog) {
	printf ("%s [-s] [-i] [-o outfile] [file...]\n\n", prog);
	puts ("The program reads files line by line and checks whether the text in the line is a palindom.");
	puts ("Each line is then printed, appended with the information if this is a palindrom or not.");
	puts ("If no input file is given, the program reads from stdin.");
	puts (" -s   ignore whitespaces");
	puts (" -i   case insensitive");
	puts (" -o   redirect output from stdout to outfile");
	exit (EXIT_FAILURE);
}

/**
 * @brief Function that checks a line for a palindrom
 * @details check a line for a palindrom
 * @param prog a string used as program name
 * @param buffer the string buffer that contains the line
 * @param length the length of the string
 * @param out file stream where the result is written to
 * @param case_insensitive boolean, true: process text case insensitive
 * @param ignore_whitespace boolean, true when whitespace characters shall be ignored
 * @return error state: false when the input file was processed, true when a write error occurred.
 **/
bool checkLine (char* prog, char* buffer, size_t length, FILE *out, bool case_insensitive, bool ignore_whitespace)
{
	size_t ix = 0; //°< line processing position
	bool is_palindrom = true;
	int c, d;

	// we allow one single character in the middle
	while (length > ix) {
		c = buffer[ix];
		d = buffer[length];
		// step over whitespace, whichever side
		if (ignore_whitespace) {
			if (c == 0x20 || c == '\t') {
				ix++;
				continue;
			}
			if (d == 0x20 || d == '\t') {
				length--;
				continue;
			}
		}
		if (case_insensitive) {
			c = tolower(c);
			d = tolower(d);
		}
		if (c != d) {
			is_palindrom = false;
			break;
		}
		ix++;
		length--;
	}
	if (fprintf (out, "%s is %s palindrom\n", buffer, is_palindrom ? "a" : "not a") < 0) {
		fprintf (stderr, "%s: Not enough memory.\n", prog);
		return true;
	}
	return false;
}

/**
 * @brief Function that checks the input stream for palindroms
 * @details Read line by line from input file stream,
 * check if the line is a palindrom,
 * and write the result to the output file stream.
 * @param prog a string used as program name
 * @param in file stream used as input
 * @param out file stream where the result is written to
 * @param case_insensitive boolean, true: process text case insensitive
 * @param ignore_whitespace boolean, true when whitespace characters shall be ignored
 * @return error state: false when the input file was processed, true when a write error occurred.
 **/
bool checkPalindrom (char* prog, FILE *in, FILE *out, bool case_insensitive, bool ignore_whitespace)
{
	size_t rpos = 0; //!< line read position
	size_t size = 0; //!< size of the line array
	char* buffer = NULL;
	int c;
	bool err = false;
	
	while ((c = fgetc (in)) != EOF) {
		if (c == '\n') { // now check!
			// set a final string termination
			buffer[rpos] = 0;
			rpos--;
			checkLine(prog, buffer, rpos, out, case_insensitive, ignore_whitespace);
			// prepare for the next line
			rpos = 0;
		}
		else {
			// first check if there is enough space
			if (rpos == size) {
				size += STR_SIZE_STEPS;
				buffer = realloc(buffer, size);
				if (buffer == NULL) {
					fprintf (stderr, "%s: Not enough memory.\n", prog);
					err = true;
				}
			}
			// insert the character into our buffer
			buffer[rpos] = c;
			rpos++;
		}
	}
	// last line
	if (!err && rpos > 0) {
		// set a final string termination
		buffer[rpos] = 0;
		rpos--;
		checkLine(prog, buffer, rpos, out, case_insensitive, ignore_whitespace);
	}
	free (buffer);
	return err;
}

/**
 * @brief Program entry point.
 * @details Decode and check parameters and write appropriate error messages.
 * Open input- and output files and initiate processing.
 * @param argc The number of arguments
 * @param argv The argument values
 * @return EXIT_SUCCESS or EXIT_FAILURE
 **/
int main (int argc, char* argv[])
{
	bool case_insensitive = false;
	bool ignore_whitespace = false;
	bool err = false;

	int c; //!< stores the result of getopt
  char *outfile = NULL;
	FILE *fin = NULL;
	FILE *fout = NULL; 	//!< support for output redirection using a FILE pointer

	// first check command line arguments
  while ((c = getopt(argc, argv, "so:i")) != -1) {
    switch (c) {
      case ':': // argument missing
      case '?': // invalid option
      case 'h':
				usage(argv[0]);
        break;
      case 'o': // set a specific output file
        outfile = optarg;
				break;
			case 'i':  // case insentive
				case_insensitive = true;
				break;
      case 's':  // ignore whitespaces
				ignore_whitespace = true;
        break;
      default: // should never be reached - defensive programming
        assert(0);
    }
  }
  // which is our output target - stdout or a file?
  if (outfile) {
    fout = fopen (outfile, "w");
    if (!fout) {
			fprintf (stderr, "%s: Could not open output file.\n", argv[0]);
			exit(EXIT_FAILURE);
		}
  }
  else fout = stdout;
  // which is our input source?
  if (optind == argc) { // no additional parameter, use stdin
		err = checkPalindrom (argv[0], stdin, fout, case_insensitive, ignore_whitespace);
  }
  // else check the given files
  else while (!err && optind < argc) {
    char *infile = argv[optind];
		fin = fopen (infile, "r");
    if (fin) {
			err = checkPalindrom (argv[0], fin, fout, case_insensitive, ignore_whitespace);
      fclose (fin);
    }
    else {
      fprintf (stderr, "%s: Opening the file %s failed - aborting.\n", argv[0], infile);
			exit(EXIT_FAILURE);
		}
    optind++;
  }
  if (fout != stdout) fclose (fout);
	return err ? EXIT_FAILURE : EXIT_SUCCESS;
}
// --------------------------------------------------------
