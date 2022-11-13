/**
 * @file myexpand.c
 * 
 * @author Laurenz Zehetner 12023972
 * 
 * @brief myexpand
 * This program implements the functionality of the default 'expand' command with the addition of the parameter '-o',
 * where an output file can be specified.
 * 
 * @details
 * Replaces every occurence of '/t' with spaces, so that the following character is at the next multiple of tabstops.
 * Tabstop distance can be specified using the parameter '-t'. Default is 8.
 * If an output file is specified using the parameter '-o', every line is written to that file. Default is stdout
 * Every file specified in the positional arguments is formatted as stated above. If no files are specified, the input stream is stdin.
 * 
 * @date 12.11.2021
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>


char * programName;

volatile sig_atomic_t quit = 0;


/**
 * @brief Prints Information about the correct use of this program.
 * @details global variables: programName
 */
static void usage(void){
	fprintf(stderr, "Usage: %s [-t positiveInteger] [-o file] [file...]\n", programName);
	exit(EXIT_FAILURE);
}

/**
 * @brief Sets global variable quit to 1 to end the program correctly (close open files etc.)
 * @details global variables: quit
 * @param signal: type of signal
 */
static void handleSignal(int signal){
	quit = 1; 
}

/**
 * @brief Converts positive integer in text format to int
 * @details Throws error and terminates program, if any symbol except digits 0-9 is given as input.
 * @param text: string of digits 0-9 no negatives or kommas allowed.
 * @return parsed Number as int
 */
static int parseNat(char* text){
	int n = 0, c;

	while ((c = *(text++)) != '\0') {
		c -= '0';
		if (0 <= c && c <= 9) {	// Digit
			n = 10 * n + c;
		} else {	// Error
			fprintf(stderr, "No valid argument for option -t\n\n");
			usage();
		}
	}
	if (n==0) {
		usage();
	}
	return n;
}

/**
 * @brief Writes input to output and replaces '\t' with spaces until next multible of tabstop in line.
 * @details global varaibles: quit: If quit == 1 the function returns even if there are more lines in the input file.
 * @param input: readable input file stream
 * @param output: writable output file stream
 * @prarm tabstop: number of characters per tabstop
 */
static void expand(FILE *input, FILE *output, int tabstop) {
	char c = fgetc(input);
	int x = 0, p=0;
	while (c != EOF && quit != 1) {
		switch (c) {
			case '\t':
				p = tabstop * (x / tabstop + 1);
				while (x < p) {
					fputc(' ', output);
					x++;
				}
				break;
			case '\n':
				fputc(c, output);
				x = 0;
				break;
			default:
				fputc(c, output);
				x++;
				break;
		}
		c = fgetc(input);
	}
}


/**
 * @brief Main Function
 * Parses arguments, sets input and output streams and then calls expand for every given input or stdin if no such input is given.
 * @param argc: number of arguments
 * @param argv: arguments
 * @return 0 if everything worked, else error code.	
*/
int main(int argc, char *const *argv)
{
	programName = argv[0];
	int tabstop = 8; // Tabstop Distance 8 by default

	FILE *out = stdout;

	// Parse options
	int c;
	while ( (c = getopt(argc, argv, "t:o:")) != -1) {
		switch (c) {
			case 't':
				tabstop = parseNat(optarg);
				break;
			case 'o':
				out = fopen(optarg, "w");
				if (out == NULL) {
					fprintf(stderr, "Failed To open %s", optarg);
					exit(EXIT_FAILURE);
				}
				break;
			default:		// Invalid option character
				usage();
				break;
		}
	}

	// Defines Action for SIGINT
	struct sigaction sa;
	memset(&sa, 0, sizeof sa);
	sa.sa_handler = handleSignal;
	sigaction(SIGINT, &sa, NULL);

	// Checks if there were no positional arguments given
	if (optind >= argc) {
		expand(stdin, out, tabstop);
	}

	FILE *in;
	for (int i=optind; i<argc; i++) {
		in = fopen(argv[i], "r");
		if (in == NULL) {
			fprintf(stderr, "Failed To open %s", argv[i]);
			fclose(out);
			exit(EXIT_FAILURE);
		}
		expand(in, out, tabstop);
		fclose(in);
	}
	fclose(out);
	return 0;
}