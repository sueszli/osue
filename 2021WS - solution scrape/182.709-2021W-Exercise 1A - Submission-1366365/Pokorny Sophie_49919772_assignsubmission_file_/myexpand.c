/**
 * @file myexpand.c
 * @author Sophie Pokorny 01633652 <e01633652@student.tuwien.ac.at>
 * @date 12.11.2021
 * @brief reads input and replaces tabs with a specified number of spaces
 *
 * @details myexpand reads input from either stdin or file(s), replaces tabs with spaces and writes the result to either stdout or a file.
 * The program takes an option -t tabstop with which the number of spaces to replace a tab with can be chosen.
 * Additionally myexpand takes the option -o outfile with which an outputfile can be specified. If no outputfile is set, the modified input will be written to stdout.
 * Lastly, the program takes positional arguments as input files to read from. If no input files are provided it will read from stdin.
 **/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define PROGRAM_NAME "myexpand"

/**
 * @brief prints the usage format and exits with EXIT_FAILURE
 * @details in case of wrong user input the usage format will be printed and the program exits
 **/
void usage(void) {
    fprintf(stderr, "[%s] Usage: %s [-t tabstop] [-o outfile] [file...]\n", PROGRAM_NAME, PROGRAM_NAME);
    exit(EXIT_FAILURE);
}

/**
 * @brief prints an errormessage and exits with EXIT_FAILURE
 * @details in case of an error a matching message will be printed, the output file will be closed if the output is not stdout and the program exits
 * @param msg the specified errormessage
 * @param output which is either stdout or a file
 **/
void error_handling(char msg[], FILE *output) {
	fprintf(stderr, msg, PROGRAM_NAME);
	if (output != stdout) {
		fclose(output);
	}
	exit(EXIT_FAILURE);
}

/**
 * @brief expand takes an input and output file and an integer tabstop to replace tabs with spaces
 * @details takes the input and either replaces tabs with the corresponding number of spaces according to the integer tabstop.
 * After the input is completely read a newline character is added after the last character to the output to guarantee a good layout.
 * @param in which is the input to be read and expanded
 * @param out which is where the output is written to
 * @param tabstop the number of spaces to replace tabs with
 * @return 0 if the expansion was completed successfully and -1 otherwise
 **/
int expand(FILE *in, FILE *out, int tabstop) {
	int x = 0;
	int c; 
	
	while((c = fgetc(in)) != EOF) {
		if (c == '\t') {
			int p = tabstop * ((x / tabstop) + 1 );
			while(x < p) {
				x++;
				fseek(out, 0, SEEK_CUR );  
				fputc(' ', out);
			}
		} else if (c == '\n') {
			x = 0;
			fseek(out, 0, SEEK_CUR );  
			fputc(c, out);
		} else {
			x++;
			fseek(out, 0, SEEK_CUR );  
			fputc(c, out);
		}
	}
	fputc('\n', out);
	if (!feof(in)) {
		return -1;
	}
	return 0;
}

/**
 * @brief entrypoint of the program which handles input and output and processing
 * @details this functions executes the program. It first processes the provided options and arguments and handles the input accordingly.
 * To modify the input the function expand is called and in case of an error the function error_handling is called. 
 * @param argc the argument counter
 * @param argv array containing the programs options and arguments
 * @return EXIT_SUCCESS if the program terminated successfully and EXIT_FAILURE otherwise
 **/
int main(int argc, char *argv[]) {
	int tabstop = 8;
	int opt_t = 0;
	int opt_o = 0;
	int c;

	char *output_name = NULL;
	FILE *output = NULL;
	FILE *input = NULL;
	while ((c = getopt(argc, argv, "t:o:")) != -1) {
		switch(c) {
			case 't':
				tabstop = (int) strtol(optarg, NULL, 10);
				opt_t++;
				break;
			case 'o':
				output_name = optarg;
				opt_o++;
				break;
			default:
				usage();
				break;
		}
	}

	if (opt_t > 1) {
		fprintf(stderr, "[%s] Error: option -t can only be set once.\n", PROGRAM_NAME);
		usage();
	}

	if (tabstop < 0) {
		fprintf(stderr, "[%s] Error: value assigned to tabstop is invalid.\n", PROGRAM_NAME);
		usage();
	}

	if (opt_o > 1) {
		fprintf(stderr, "[%s] Error: option -o can only be set once. Only one output file can be chosen.\n", PROGRAM_NAME);
		usage();
	}

	if (opt_o == 1) {
		if ((output = fopen(output_name, "w")) == NULL) {
			fprintf(stderr,"[%s] Error: output file could not be opened.\n", PROGRAM_NAME);
     	}
	}

	if (opt_o == 0) {
		output = stdout;
	}

	if (argc == optind) {
		input = stdin;
		if (expand(input, output, tabstop) == -1) {
			fclose(input);
			error_handling("[%s] Error: something went wrong while expanding the input.\n", output);
     	}
		exit(EXIT_SUCCESS);
	}

	if ((argc - optind) > 0) {
		for (int i = optind; i < argc; i++) {
			if ((input = fopen(argv[i], "r")) == NULL) {
				error_handling( "[%s] Error: input file(s) could not be opened.\n", output);
     		}
     		if (expand(input, output, tabstop) == -1) {
     			fclose(input);
     			error_handling("[%s] Error: something went wrong while expanding the input.\n", output);
     		}
     		fclose(input);
		}
		if (output != stdout) {
		fclose(output);
		}
		exit(EXIT_SUCCESS);
	}
}

