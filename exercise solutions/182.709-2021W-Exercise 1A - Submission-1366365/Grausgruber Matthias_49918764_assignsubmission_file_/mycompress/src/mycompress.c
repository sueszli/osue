/** Assignment A - mycompress  
 *	@file mycompress.c
 *  @author Matthias Grausgruber <e00525708@student.tuwien.ac.at>
 *  @date 10.11.2021
 *  
 *  @brief Main program module.
 *
 *	This program compresses the input (text) in a very simple way.
 *	It substitutes the subsequent identical characters by only one
 *	occurence of ther character followed by the number of characters.
 *	The input is either a file or stdin (keyboard input).
 *	The output is either a file or stout (shell).
 *  
 **/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

static char *pgm_name; /**< The program name. */

/**
 * Print error and exit.
 * @brief This function prints the error-message to the shell in case of an error and ends the program.
 * @details global variables: pgm_name
 */
void errorExit (int line) {
	fprintf (stderr, "%s: error at line #%d: %s\n", pgm_name, line, strerror(errno));
	exit (EXIT_FAILURE);
}

/**
 * Print error (and no exit).
 * @brief This function writes prints the error-message to the shell in case of an error.
 * 		  In difference to errorExit() it doesn't end the program.
 * @details global variables: pgm_name
 */
void errorPrint (int line) {
	fprintf (stderr, "%s: error at line #%d: %s\n", pgm_name, line, strerror(errno));
}

/**
 * Program entry point.
 * @brief The main program starts here. This function reads all options (and arguments) and
 * positional arguments. It differentiates between file in-/output and stdin-/out.
 * The compression of the input is quite simple. It counts subsequent identical characters and
 * outputs the character and the count. Linefeed will be compressed only by count (no character).
 * 
 * @details In-/Output stream is handled with Stream I/O (bufferd).
 * To extract the usage commands getopt() reads the options and arguments.
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS.
 */
int main (int argc, char **argv) {

    char *o_arg = NULL, // Option Arguments.
		  buffer[1024], out_tmp[1024], temp = ' '; /* Buffer for in-/output. */
	FILE *in = stdin, *out = stdout; /* Stream handler. */
	int c, opt_o = 0, input_ind, /* Variables for option-/argument-handling. */
		num_in = 0, num_out = 0, count = 0; /* Counting read/written characters. */

	pgm_name = argv[0];
	/* Options, arguments and positional arguments are procesed here.*/
	while ( (c = getopt(argc, argv, "o:")) != -1 ) {
		switch (c) {
			case 'o': o_arg = optarg;
				  opt_o++;
				  if (opt_o <= 1) break;
			default: /* invalid option */
				fprintf (stderr, "Usage: %s [-o outfile] [file...]\n", argv[0]);
				exit (EXIT_FAILURE);
		}
	}
	if (o_arg != NULL) {
		if ((out = fopen (o_arg, "w")) == NULL) errorExit(__LINE__);
	}
	if (out == stdout) printf ("Output (printed after every input-line):\n");

	input_ind = optind;
	/** Algorythm starts here. If there are multiple input files, the process will be repeated. */
	do {
		if (input_ind < argc) {
			if ((in = fopen (argv[optind], "r")) == NULL) errorExit(__LINE__);
		} else if (in == stdin) printf ("Start keyboard input (end input with CTRL+D in an empty row):\n");

		/* One loop cicle is equivalent to an input line. */
		while (fgets (buffer, sizeof (buffer), in) != NULL) {		
			/* If "end of input" is reached the loop will be stopped. */	
			if (feof (in)) {
				if (fclose (in) == EOF) errorPrint(__LINE__);
				break;
			}

			/* Subsqeuent linefeeds are processed here. */
			if ((temp == '\n') && (buffer[0] == '\n')) {
				count++;
				temp = buffer[0];
			} else {
				/* The end of subsequent linefeeds are processed here. */
				if (count >= 1) {
					sprintf (out_tmp, "%d", count);
					if (fputs (out_tmp, out) == EOF) errorPrint(__LINE__);
					num_out += strlen (out_tmp);
					num_in += count;
					count = 0;
				}

				/* The for-loop compresses one line (until '\0') */
				temp = buffer[0];
				for (int i=0; buffer[i] != '\0'; i++) {
					if (temp != buffer[i]) {
						sprintf (out_tmp, "%c%d", temp, count);
						if (fputs (out_tmp, out) == EOF) errorPrint(__LINE__);
						num_out += strlen (out_tmp);
						num_in += count;
						count = 1;
						temp = buffer[i];
					} else count++;
				}
				if (num_in > 1) {
					if (fputs ("\n", out) == EOF) errorPrint(__LINE__);
					num_out++;
				}
			}
			if (ferror (out)) errorPrint(__LINE__);
			if (ferror (in)) errorPrint(__LINE__);
		}
		sprintf (out_tmp, "%d\n", count);
		if (fputs (out_tmp, out) == EOF) errorPrint(__LINE__);
		num_out += strlen (out_tmp)-1;
		num_in++;

		if (fclose (in) == EOF) errorPrint(__LINE__);
		input_ind++;
	} while (input_ind < argc);
	
	/* The statistics are written to stderr before exit. */
	fprintf (stderr, "Read: %d characters\nWritten: %d characters\nCompression ratio: %.1f%%\n",\
			num_in, num_out, ((double)num_out/(double)(num_in)*100.0));

	if (fclose (out) == EOF) errorExit (__LINE__);

	return EXIT_SUCCESS;
}
